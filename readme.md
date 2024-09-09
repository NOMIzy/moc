```
cd mocutils
sudo make install
```
如果出现
`cp: 无法获取'cp' 的文件状态(stat): 没有那个文件或目录`
`cp: 警告：指定来源文件'libmocutils.a' 多于一次`

则检查g++版本，手动执行
```
sudo cp libmocutils.a /usr/lib/gcc/{shell uname -m,检查自己的环境}-linux-gnu/*
```
Pyrite库安装同理

测试：
编译服务端并启动
```
cd server
make build
./server
```

编译客户端并启动
```
cd client
make build
./client
```
客户端向服务端发送字符串，服务端原封不动返回字符串的示例