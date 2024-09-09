#ifndef _PRT_CLIENT_H
#define _PRT_CLIENT_H

#include <mocutils/channel.h>

#include "define.h"
#include "functional"
#include "map"
#include "package.h"

namespace prt {
class client {
	//server的socket文件描述符，地址
	int server_fd;
	sockaddr_in server_addr;
	//路由
	std::map<std::string, std::function<bytes(bytes)>> router;
	int sequence;

	std::map<int, moc::channel<prt::package> *> promise_buf;

 public:
 //连接状态
	connection_state state;
	client(const char *ip, int port);
	~client();
//流程函数
	void start();
	void async();
	//设置处理函数
	bool set_handler(std::string identifier, std::function<bytes(bytes)> handler);
	static void *process(void *_args);
	void tell(std::string identifier, bytes body);
	bytes promise(std::string identifer, bytes body);
};
}	 // namespace prt

#endif