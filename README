# Lab1 System Call

## 系统调用

当进程需要调用系统函数时，会暂时将控制权交给内核。经过以下步骤

- 将参数存储在用户空间到内核空间的寄存器中（a0：进程名，a1：参数数组，a7: syscallnum）

    ```
    la a0, init
    la a1, argv
    li a7, SYS_exec
    ecall
    ```
- 调用ecall，内核将读取这些参数, 并执行相应的函数。调用完成后，返回值放在a0。

## 1. System call tracing

目标是实现一个用户程序trace，并且向其传递一个参数mask，mask对应的`1 << syscall_num`位为1时，能够追踪对应的syscall

```
$ trace 32 grep hello README
3: syscall read -> 1023
3: syscall read -> 966
3: syscall read -> 70
3: syscall read -> 0
```

这就需要实现一个trace系统调用，因为需要修改程序内部，在proc(proc.h)结构中增加一个mask成员。

### 流程

按照[官方文档](https://pdos.csail.mit.edu/6.S081/2020/labs/syscall.html), 逐步配置好user.h, usys.pl, syscall.h

再在sysproc.c里添加sys_trace()，需要注意的是，文档中没有提到的syscall.c也需要修改，在里面添加syscall_name以及syscall[]

### sys_trace()代码
```c
uint64
sys_trace(void)
{
    int mask;

    if (argint(0, &mask) < 0) {
        return -1;
    }
    myproc()->trace_mask = mask;
    return 0; 
} 
```


## 2. Sysinfo

该实验需要实现一个能够将系统信息复制到用户空间(保存在sysinfo(sysinfo.h)的struct中)的syscall

包括两个信息：
- 剩余的空闲memory
- 活跃的进程数量

### 获取freemem的函数

最好阅读一下`kalloc.c`中其他部分，至少要知道，run struct就是一系列指向pages的指针，kmem中的freemem成员是指向下一个能使用的page的指针。

```c
uint64
count_freemem(void)
{
    struct run* r;
    uint64 sz = 0;
    acquire(&kmem.lock);
    r = kmem.freelist;
    release(&kmem.lock);
    while (r){
        sz += PGSIZE;
        r = r->next;
    }
    return sz;
}
```
### 获取nproc的函数

非常容易，遍历一遍proc的数组即可

```c
uint64 unused_num() {
    int pid, count = 0;
    for (pid = 0; pid < NPROC; pid++) {
        if (proc[pid].state != UNUSED) {
            count++;
        }
    }
    return count;
}
```

### 系统调用的函数

不能直接将信息复制到用户空间，因为系统的pagetable和用户的pagetable是不一样的，使用copyout从系统空间复制到用户空间。

```c
uint64 sys_sysinfo(void)
{
    struct sysinfo sf;
    uint64 addr;
    sf.freemem = count_freemem();
    sf.nproc = unused_num();


    /* 将sysinfo复制到userspace */
    argaddr(0, &addr);
    if (addr > MAXVA)
        return -1;
    copyout(myproc()->pagetable , addr, (char *)&sf, sizeof(sf));
    return 0;
}
```

