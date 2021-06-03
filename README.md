# 1. 根据提示输入分配内存范围 #

Input two integer: upper and lower of memory
**100 2000**

> 指示系统将逻辑地址100 - 2000B 划分为可寻址的空间
>
> 页框默认大小50B, 进程最大占用3个页框.

# 2. 根据提示输入指令 #

-------------INSTRUCTIONS------------
ONE: OperationCode + PID/SPACE
  1-->Create a new process
  2-->Suspend a process
  3-->Block a process
  4-->End a process
  5-->[FIFO]Access a address
  6-->[LRU]Access a address
  7-->Add random number

_>**1 500**
<process>
pid: 1 uid: 0 status: RUN length: 500
 page_table:-1 -1 -1 -1 -1 -1 -1 -1 -1 -1

<memory>
00000000
00000000
00000000
00000000
11000000
_>**6 1 0**
<process>
pid: 1 uid: 0 status: RUN length: 500
 page_table:0 -1 -1 -1 -1 -1 -1 -1 -1 -1
0
<memory>
00000001
00000000
00000000
00000000
11000000
_>**6 1 50**
<process>
pid: 1 uid: 0 status: RUN length: 500
 page_table:0 1 -1 -1 -1 -1 -1 -1 -1 -1
0 1
<memory>
00000011
00000000
00000000
00000000
11000000
_>**6 1 100**
<process>
pid: 1 uid: 0 status: RUN length: 500
 page_table:0 1 2 -1 -1 -1 -1 -1 -1 -1
0 1 2
<memory>
00000111
00000000
00000000
00000000
11000000
_>**6 1 50**
<process>
pid: 1 uid: 0 status: RUN length: 500
 page_table:0 1 2 -1 -1 -1 -1 -1 -1 -1
0 2 1

> LRU: 将最近访问页码调到栈顶.

<memory>
00000111
00000000
00000000
00000000
11000000
_>**6 1 150**
<process>
pid: 1 uid: 0 status: RUN length: 500
 page_table:-1 1 2 0 -1 -1 -1 -1 -1 -1
2 1 3

> LRU于FIFO相同, 弹出栈底页码, 将请求页码入栈.

<memory>
00000111
00000000
00000000
00000000
11000000
