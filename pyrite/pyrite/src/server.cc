#include "server.h"

#include <mocutils/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

prt::server::server(int port) {
#ifdef Windows
	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data))
		moc::panic("WSAStartup failed.");
#endif
	this->state = closed;
	if ((this->server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		moc::panic("Pyrite bind port failed.");

	memset(&this->server_addr, 0, sizeof(struct sockaddr_in));
	//指定地址家族为 AF_INET，表示使用 IPv4
	this->server_addr.sin_family = AF_INET;

	//将端口号转换为网络字节序，并存储在 sin_port 中。htons 是 "host to network short" 的缩写，
	//将主机字节序（通常是小端序）转换为网络字节序（大端序）
	this->server_addr.sin_port = htons(port);

	//将 IP 地址从点分十进制字符串转换为一个 32 位的二进制数
	this->server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//INADDR_ANY 是一个宏，表示“接受所有网络接口上的所有地址”

	//将服务器套接字 this->server_fd 绑定到 this->server_addr 所指定的地址和端口
	if (bind(this->server_fd, (struct sockaddr *)&this->server_addr, sizeof(this->server_addr)) < 0)
		moc::panic("Pyrite server binding failed.");

	this->sequence = 0;
	this->state = prt::established;
}

prt::server::~server() {
#ifdef Windows
	WSACleanup();
#endif
}

void prt::server::start() {
	int recv_len;
	sockaddr_in client_addr;
	socklen_t l = sizeof(client_addr);
	pthread_t tid;
	char buf[prt::max_transmit_size];

	while (true) {
		recv_len = recvfrom(this->server_fd, buf, prt::max_transmit_size, 0, (struct sockaddr *)&client_addr, &l);
		if (recv_len < 0) {
			static int counter;
			counter %= 32;
			if (++counter) continue;
			moc::warnf("invalid recv_len: %d.", recv_len);
			continue;
		}
		process_args *args = new process_args{this, client_addr, prt::package(prt::bytes(buf, recv_len))};
		pthread_create(&tid, NULL, this->process, (void *)args);
	}
}

namespace prt {
void *server_async_runner(void *args) {
	server *s = (server *)args;
	s->start();
	return nullptr;
}
};	// namespace prt

void prt::server::async() {
	pthread_t tid;
	pthread_create(&tid, NULL, server_async_runner, (void *)this);
}

bool prt::server::set_handler(std::string identifier, std::function<bytes(sockaddr_in, bytes)> handler) {
	if (identifier.find("prt-") == 0)
		return false;
	this->router[identifier] = handler;
	return true;
}

void *prt::server::process(void *_args) {
	assert(_args);
	process_args *args = (process_args *)_args;
	prt::server *server_ptr = (prt::server *)args->ptr;
	prt::package recv_pkg = args->pkg;
	sockaddr_in _client_addr = args->addr;
	delete args;

	// process prt ack
	if (recv_pkg.identifier == "prt-ack") {
		//将 _client_addr 转换为 prt::bytes 类型，存储客户端地址。
		prt::bytes client_addr(&_client_addr, sizeof(_client_addr));
		//检查 server_ptr->client_data 中是否包含这个客户端的记录。如果没有记录，返回 nullptr。
		if (!server_ptr->client_data.count(client_addr))
			return nullptr;
		//检查 promise_buf 中是否存在对应的序列号。如果不存在，返回 nullptr。
		if (!server_ptr->client_data[client_addr].promise_buf[recv_pkg.sequence])
			return nullptr;
		*server_ptr->client_data[client_addr].promise_buf[recv_pkg.sequence] << recv_pkg;
		return nullptr;
	}

	if (!server_ptr->router.count(recv_pkg.identifier))
		recv_pkg.identifier = "*";

	if (!server_ptr->router.count(recv_pkg.identifier))
		return nullptr;

	prt::bytes reply = server_ptr->router[recv_pkg.identifier](_client_addr, recv_pkg.body);
	if (!reply.size())
		return nullptr;
	prt::package reply_pkg(recv_pkg.sequence, "prt-ack", reply);
	reply_pkg.send_to(server_ptr->server_fd, _client_addr);
	return nullptr;
}

void prt::server::tell(sockaddr_in client_addr, std::string identifier, bytes body) {
	prt::package pkg(-1, identifier, body);
	pkg.send_to(this->server_fd, client_addr);
}

prt::bytes prt::server::promise(sockaddr_in _client_addr, std::string identifer, bytes body) {
	//每个client有一个seq，发包会记录服务器这边，这个client目前的seq

	
	prt::bytes client_addr(&_client_addr, sizeof(_client_addr));
	//新来的client，在client_data注册，设置seq
	if (!this->client_data.count(client_addr))
		this->client_data[client_addr] = prt::_client_data{.sequence = 0};
	int seq = this->client_data[client_addr].sequence++;
	prt::package pkg(seq, identifer, body);
	pkg.send_to(this->server_fd, _client_addr);
	//自己实现的阻塞channel
	this->client_data[client_addr].promise_buf[seq] = makeptr(channel, prt::package);
	prt::package reply;
	*this->client_data[client_addr].promise_buf[seq] >> reply;
	delete this->client_data[client_addr].promise_buf[seq];
	return reply.body;
}
