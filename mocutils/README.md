# mocutils

mocutils (mo-crystal utils) 是魔晶工作室内部的工具库。

## 安装

Linux

```shell
sudo make install
# 如果需要
make clean
```

Windows

```shell
# 管理员模式下
make install
# 如果需要
make clean
```

## 目前已实现

**log**

log 是 `[I] ` 开头的绿色文字；warn 是 `[W] ` 开头的黄色文字；panic 是 `[E] ` 开头的红色文字，并且会导致程序异常退出。

```cpp
moc::log(std::string);
moc::warn(std::string);
moc::panic(std::string);

moc::logf(const char *fmt, ...);
moc::warnf(const char *fmt, ...);
moc::panicf(const char *fmt, ...);
```

**semaphore**

信号量机制。

semaphore 有 val 和 cap 属性。val 是当前值，cap 是上限。val 的合法值为 [0, cap]。

semaphore 有 acquire 和 release 两种功能。前者相当于 PV 操作的 P，后者相当于 PV 操作的 V。

若 val 在以上两种操作后的值仍然合法，则进程不会阻塞，值会顺利改变；若本次操作会导致 val 值不合法，则该进程会阻塞，并加入等待队列，直到一个相反操作解除此阻塞。

```cpp
moc::semaphore<3> s0;    // 创建一个最大值为 3 的信号量
moc::semaphore<3> s1(2); // 创建一个最大值为 3 的信号量，并且赋初值为 2

s0.release(); // s0.val == 1
s0.release(); // s0.val == 2
s0.acquire(); // s0.val == 1
s0.acquire(); // s0.val == 0
s0.acquire(); // blocked
```

**channel**

功能和 gochannel 完全相同的 channel 工具。

channel 分为无缓冲区（nbchannel）和有缓冲区（bchannel）两种，都是继承自 channel 抽象类。

nbchannel 在管道另一端有等待者的情况下不会阻塞，会直接传输数据；在管道另一端没有等待者的情况下会阻塞，直到对端出现进程，才解除阻塞，传输数据。

bchannel 的功能和用法与 nbchannel 相似。不同之处在于，bchannel 的缓冲区可以容纳本次操作时，不会阻塞。

程序员应当使用 make 宏创建 channel 对象，使用 makeptr 宏创建其指针。这些宏会依据参数，判断程序员需要哪种 channel。当参数仅指定数据 type 时，会返回无缓冲区管道；参数为数据 type & 容量（数字）时，会返回有缓冲区管道。

定义局部变量时，可以使用 auto 关键字来兼容两种 channel；定义参数时，可以使用 channel& 或 channel* 来兼容两种 channel。这么做可以屏蔽两种 channel 的语法差异，代码看起来会比较和谐。

**注意：不要在函数调用中以值的形式传递任何 channel，可能会导致严重的内存安全和逻辑错误问题。可以使用 * 指针或 & 引用。（当然，channel 是一个抽象类，你也没法定义一个 channel 类型的参数... 问题不大）**

关于使用 channel 进行数据传输，可以参考如下代码

```c++
auto c0 = make(channel, int);         // 无缓冲区 channel (nbchannel)
auto c1 = make(channel, int, 3);      // 有缓冲区 channel (bchannel)
auto c2 = makeptr(channel, int);      // 无缓冲区 channel (nbchannel) 的指针，用完需要 delete
auto c3 = makeptr(channel, int, 3);   // 有缓冲区 channel (bchannel) 的指针，用完需要 delete

int recv;
c0 << 1;    // 传入数据
            // 单线程程序会在此卡死，因为此时无缓冲区管道不可能等到另一个进程取走数据
c0 >> recv; // 取出数据。原理同上
c1 << 1;    // 传入数据。缓冲区未满时不会阻塞
c1 >> recv; // 取出数据，原理同上

// 以下为指针操作，功能和原理同上
*c2 << 1;
*c2 >> recv;
*c3 << 1;
*c3 >> recv;

delete c2;
delete c3;
```

**byte**

将 byte 数组进行了一些初步的封装，支持拼接、裁切，字符串转换等。还支持以流的形式读出其中的数据。

**strings**

一些字符串处理函数。
