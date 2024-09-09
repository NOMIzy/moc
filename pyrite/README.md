# Pyrite

Pyrite 是一个基于 UDP 的异步通讯协议，支持为数据包分配路由函数，也支持等待回信。

## 安装

Linux

```shell
sudo make install
# 如果需要
make clean
```


使用make工具编译 gcc 项目，形如
`g++ -o {目标执行程序名} src/* -lmocutils -lpyrite`

## 使用

**一个简单的示例**

server.cc

```cc

prt::bytes recv_print(sockaddr_in addr, prt::bytes data) {
  return prt::bytes("server received: " + data.to_string());
}

int main() {
  prt::server s(8080);
  s.set_handler("print", recv_print);
  s.start();
  return 0;
}

```

client.cc

```cc

int main() {
  prt::client c("127.0.0.1", 8080);
  c.async();
  std::string input;
  while (true) {
    std::cin >> input;
    std::cout << c.promise("print", prt::bytes(input)).to_string() << std::endl;
  }
  return 0;
}

```

## 解释
一个基于UDP封装的一个应用层协议
类似于其他的应用层协议一样，设计一个协议头，然后包在UDP协议的外面。

这个协议主要是有两种数据包，一种是promise，一种是tell。
promise会发请求，如果对端回应，会一直阻塞等待对端回应，对端回应了这个promise就会返回一串字节流。然后还有还有一种数据包是tell，没有回信，只负责把数据发出去，不用等待对面回信，发完之后继续执行了。然后这个协议支持像HTTP一样指定一个handler，每一个数据包前面封装一个标识符里面就是一个字符串，对端可以使用它达到类似于各种网络框架里的router的作用。

具体的实现，首先实现比特流类，它是一串比特，可以往里面塞int，可以往里面塞string，即可读可写。然后封装前面四个比特是一个数字，后面是一串字符，再后面就是数据了。前面四个比特是一个标识符，是为promise协议准备的。当收到对端的1个ACK，根据这个标识符就能找到正在等待的那个promise，然后就把这个数据包返回给那个promise里面等待的线程。
tell的话就不用这个标志符，因为tell没有回信，也不需要等待。所以tell的标识符后面是一串string类似于HTTP协议的路径，根据这个string去分配一个处理函数。后面的数据会输入对应处理函数去处理。处理完之后，函数也可以返回，也可以不返回。

简单的封装，主要是为了处理游戏服务器跟客户端的通信问题。选择UDP是因为该场景无连接带来的速度优势更重要。
