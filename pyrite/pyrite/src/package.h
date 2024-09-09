#ifndef _PRT_PACKAGE
#define _PRT_PACKAGE

#include <sys/types.h>

#include "string"
#ifdef Windows
#include <windows.h>
#include <winsock2.h>
typedef int socklen_t;
#define MSG_CONFIRM 0
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "define.h"
#include "mocutils/byte.h"

namespace prt {
typedef moc::bytes bytes;
class package {
 public:
 //package类有三个属性
	i32 sequence;
	std::string identifier;
	bytes body;
//默认构造函数/赋值的构造函数/复制构造函数
	package();
	package(bytes raw);
	package(package *old);
	package(i32 sequence, std::string identifier, bytes body);
	bytes to_bytes();
//比较包的差异，运算符重载
	bool operator==(const package &other);
	bool operator!=(const package &other);
	bool operator<(prt::package &other);
//设置body属性
	void set_body(std::string text);
//发送socket信息，先转换body对象，再发送
	std::string body_as_string();
	void send_to(int socket_fd, sockaddr_in socket_addr);
};

//基于原有的五元组，增加自己的package对象
typedef struct {
	void *ptr;
	sockaddr_in addr;
	package pkg;
} process_args;
}	 // namespace prt

#endif