<div align=center><img src="logo_01(2).jpg"></div>

# OS #
## 1. 进程控制 ##
1.定义管理每个进程的数据结构 PCB.  
2.模拟触发进程状态转换的事件.  
3.根据当前发生的事件对进程的状态进行切换，并显示出当前系统中的执 
行队列、就绪队列和阻塞队列.  
4.完成可变分区的分配与回收，创建进程的同时申请一块连续的 
内存空间，在 PCB 中设置好基址和长度.  
5.回收所有进程的空间，对空间分区的合并。  
6.可以查看进程所占的空间和系统空闲空间。

## 2. 分页式存储器管理 ##
1.在实验 1 基础上实现分页式存储管理内存分配和地址转换过程。  
2.进一步实现请求分页式存储管理过程，包括内存和置换空间管理、地址转换以及缺页处理，能够体现 FIFO 和 LRU 算法思想.