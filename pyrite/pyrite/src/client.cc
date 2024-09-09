#include "client.h"

#include <mocutils/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstring>

#include "pthread.h"

prt::client::client(const char *ip, int port) {
#ifdef Windows
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data))
		moc::panic("WSAStartup failed.");
#endif
	//初始状态关闭
	this->state = prt::closed;
	//尝试建立一个套接字
	if ((this->server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		moc::warnf("Pyrite connect creation failed. Server addr: %s:%d.", ip, port);
		return;
	}

	//地址初始化空间
	memset(&this->server_addr, 0, sizeof(this->server_addr));

	//指定地址家族为 AF_INET，表示使用 IPv4
	this->server_addr.sin_family = AF_INET;

	//将端口号转换为网络字节序，并存储在 sin_port 中。htons 是 "host to network short" 的缩写，
	//将主机字节序（通常是小端序）转换为网络字节序（大端序）
	this->server_addr.sin_port = htons(port);

	//将 IP 地址从点分十进制字符串转换为一个 32 位的二进制数
	this->server_addr.sin_addr.s_addr = inet_addr(ip);

	//初始化sequence长度
	this->sequence = 0;

	//现在client准备好进行通信
	this->state = prt::established;
}

prt::client::~client() {
	close(this->server_fd);
#ifdef Windows
	WSACleanup();
#endif
}

void prt::client::start() {
	int recv_len;
	socklen_t l = sizeof(this->server_addr);
	pthread_t tid;
	char buf[prt::max_transmit_size];

	while (true) {
		recv_len = recvfrom(this->server_fd, buf, prt::max_transmit_size, 0, (struct sockaddr *)&this->server_addr, &l);
		if (recv_len < 0) {
			//使用 static int counter 来跟踪连续失败的次数，counter 每次加 1，循环到 32 时重新开始。
			static int counter;
			counter %= 32;
			if (++counter) continue;
			moc::warnf("invalid recv_len: %d.", recv_len);
			continue;
		}
		process_args *args = new process_args;
		args->ptr = this;
		//数据拷贝，每次新接收数据buf也会装入新数据，所以这里是安全的
		args->pkg = prt::package(prt::bytes(buf, recv_len));

		//创建线程处理接受数据，避免主线程阻塞
		pthread_create(&tid, NULL, this->process, (void *)args);
	}
}

namespace prt {
void *client_async_runner(void *args) {
	client *c = (client *)args;
	c->start();
	return nullptr;
}
};	// namespace prt

void prt::client::async() {
	pthread_t tid;
	pthread_create(&tid, NULL, client_async_runner, (void *)this);
}

//用于设置一个处理程序（handler），该处理程序会根据数据包的标识符（identifier）来处理接收到的数据包。
//处理程序是一个函数对象（std::function），它接受一个 bytes 对象作为输入，并返回一个 bytes 对象作为输出。
bool prt::client::set_handler(std::string identifier, std::function<bytes(bytes)> handler) {
	//检查标识符是否以 "prt-" 开头
	if (identifier.find("prt-") == 0)
		return false;
	this->router[identifier] = handler;
	return true;
}

//process 函数用于处理接收到的数据包。
//这个函数会在一个独立的线程中执行，负责解析数据包并调用适当的处理程序，同时发送回应（如果需要）
void *prt::client::process(void *_args) {
	//检查参数是否不为空，提取参数，释放空间
	assert(_args);
	process_args *args = (process_args *)_args;
	prt::client *client_ptr = (prt::client *)args->ptr;
	prt::package recv_pkg = args->pkg;
	delete args;

	// process prt ack
	//如果 client_ptr->promise_buf 中存在对应的序列号缓冲区，将数据包 recv_pkg 写入该缓冲区，处理完毕
	if (recv_pkg.identifier == "prt-ack") {
		if (client_ptr->promise_buf[recv_pkg.sequence])
			*client_ptr->promise_buf[recv_pkg.sequence] << recv_pkg;
		return nullptr;
	}

	//检查 recv_pkg.identifier 是否在 client_ptr->router 中存在。
	//如果不存在，将标识符设置为 "*"，表示通配符匹配。
	if (!client_ptr->router.count(recv_pkg.identifier))
		recv_pkg.identifier = "*";
	//如果 client_ptr->router 中仍然没有对应的标识符，返回 nullptr，表示没有适当的处理程序。
	if (!client_ptr->router.count(recv_pkg.identifier))
		return nullptr;
	//调用与标识符关联的处理程序，将数据包的 body 作为参数传入，获取处理结果 reply。
	prt::bytes reply = client_ptr->router[recv_pkg.identifier](recv_pkg.body);

	//如果处理结果不为空，创建一个回应数据包 reply_pkg，将 reply 作为回应体，并设置标识符为 "prt-ack"
	if (!reply.size())
		return nullptr;
	prt::package reply_pkg(recv_pkg.sequence, "prt-ack", recv_pkg.body);
	reply_pkg.send_to(client_ptr->server_fd, client_ptr->server_addr);
	return nullptr;
}

//tell 函数用于发送一种无需响应的数据包到服务器，不需要等待服务器的回应。
void prt::client::tell(std::string identifier, bytes body) {
	prt::package pkg(-1, identifier, body);
	pkg.send_to(this->server_fd, this->server_addr);
}

//promise发送请求后会等待服务器的回应。该函数会阻塞直到接收到回应，并返回回应的数据体。
prt::bytes prt::client::promise(std::string identifer, bytes body) {
	int seq = this->sequence++;
	prt::package pkg(seq, identifer, body);
	pkg.send_to(this->server_fd, this->server_addr);

	//channel 管道，用于等待和接收来自服务器的回应
	//在mocutils中实现
	this->promise_buf[seq] = makeptr(channel, prt::package);
	prt::package reply;
	*this->promise_buf[seq] >> reply;
	delete this->promise_buf[seq];
	return reply.body;
}
