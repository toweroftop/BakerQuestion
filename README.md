# BakerQuestion
 A Solution of the BakerQuestion in Linux



## 面包师问题

设计一个使用条件变量的Linux平台下的C程序完成面包师问题



### 问题描述：

面包师有很多面包和蛋糕，由n 个销售人员销售。每个顾客进店后先取一个号，并且等着叫号。当一个销售人员空闲下来，就叫下一个号。请分别编写销售人员和顾客进程的程序。

 

### 运行：

在终端里执行bin内的文件即可。

由consumer2或consumer3发送信号， baker3接收信号。



### 编译：

 主要代码在src里，test是试错的。

bin里是在ubuntu12.04版本里的编译的，版本可能不一样。

执行Complie.sh调用gcc编译



### 说明：

使用了异步信号、队列、条件变量和线程(pthread)