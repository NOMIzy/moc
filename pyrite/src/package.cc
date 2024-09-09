#include "package.h"

#include <mocutils/log.h>
//头文件的具体实现
//构造函数
prt::package::package() {
	this->sequence = 0;
	this->identifier = "";
	this->body = prt::bytes(0);
}

prt::package::package(prt::bytes raw) {
	this->sequence = raw.next_int32();
	this->identifier = raw.next_string();
	this->body = raw.range(raw.ptr, raw.size());
}

prt::package::package(package *old) {
	assert(old);
	this->sequence = old->sequence;
	this->identifier = old->identifier;
	this->body = old->body;
}

prt::package::package(i32 sequence, std::string identifier, bytes body) {
	this->sequence = sequence;
	this->identifier = identifier;
	this->body = body;
}

//将sequence信息转换成字节和body信息拼接
prt::bytes prt::package::to_bytes() {
	bytes ret(4);
	for (int i = 0; i < 4; i++)
		ret[i] = (moc::byte)(this->sequence >> (i * 8));
	ret = ret + bytes(this->identifier);
	return ret + this->body;
}

bool prt::package::operator==(const prt::package &other) {
	if (other.sequence != this->sequence) return false;
	if (other.identifier != this->identifier) return false;
	if (other.body != this->body) return false;
	return true;
}

bool prt::package::operator!=(const prt::package &other) {
	return !(this->operator==(other));
}

void prt::package::set_body(std::string text) {
	this->body = bytes(text);
}

//调用bytes对象的转换字符串方法
std::string prt::package::body_as_string() {
	return this->body.to_string();
}

//发包函数
void prt::package::send_to(int socket_fd, sockaddr_in socket_addr) {
	//将当前的 package 对象转换为字节序列
	bytes pkg_bytes = this->to_bytes();
	//检查生成的字节序列是否超出了最大传输大小，
	//如果 pkg_bytes 的大小超出限制，
	//则调用 moc::panic("content overflowed")，引发一个异常或终止程序，防止数据溢出错误。
	if (pkg_bytes.size() > prt::max_transmit_size)
		moc::panic("content overflowed");

	char msg[pkg_bytes.size()];
	for (int i = 0; i < pkg_bytes.size(); i++)
		msg[i] = pkg_bytes[i];

	//基于 POSIX 标准的 Berkeley sockets API，
	//通常用于网络编程，特别是使用UDP协议
	//用于通过套接字发送数据包到指定的网络地址。
	//这个函数在几乎所有的 Unix-like 操作系统（如Linux、macOS）和Windows的网络编程中都可以使用。
	sendto(socket_fd, msg, pkg_bytes.size(), 0, (struct sockaddr *)&socket_addr, sizeof(socket_addr));
	//参数与返回值说明：
	//sockfd：一个套接字的文件描述符，指定要通过哪个套接字发送数据。
	//buf：一个指向要发送的数据的指针。
	//len：要发送的数据的长度（以字节为单位）。
	//flags：发送操作的标志位，通常设置为0以使用默认行为。
	//dest_addr：一个指向目的地址的 sockaddr 结构体的指针，这里指定了数据包的目标网络地址。
	//addrlen：dest_addr 结构体的长度（通常使用 sizeof(struct sockaddr) 来获取）。
	//sendto 返回发送的字节数，如果发生错误，返回 -1，并设置 errno 以指示错误类型。
}

bool prt::package::operator<(prt::package &other) {
	auto b1 = this->to_bytes();
	auto b2 = other.to_bytes();
	int l = b1.size();
	if (b2.size() < l)
		l = b2.size();
	for (int i = 0; i < l; i++)
		if (b1[i] != b2[i])
			return b1[i] < b2[i];
	if (b2.size() > b1.size())
		return true;
	return false;
}
