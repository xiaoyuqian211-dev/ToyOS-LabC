# ToyOS-LabC

## 1. 项目简介

ToyOS-LabC 是一个运行在 QEMU RISC-V64 `virt` 平台上的教学型小操作系统实验项目。项目目标是通过一个轻量级内核展示操作系统从启动、内核初始化到用户程序运行的基本链路，使课程实验中的关键机制能够被编译、运行和测试。

该项目覆盖以下内容：

- 内核启动
- 控制台输出
- Trap / 异常入口
- 系统调用接口
- 物理页分配
- 页表与地址空间
- 进程管理与调度抽象
- 用户程序运行
- ToyFS 只读文件系统
- 自动化 smoke test

该项目并非追求生产级通用操作系统，而是围绕操作系统课程中的核心机制构建一个可编译、可运行、可测试的教学型内核。项目采用轻量化实现方式聚焦课程核心机制，保留了操作系统启动、内核服务、用户程序、进程关系与文件抽象的关键路径。

## 2. 实验目标

本项目对应操作系统实验 C / 从零开始写操作系统方向，主要目标包括：

1. 理解操作系统启动流程；
2. 理解内核态与用户态的边界；
3. 理解系统调用如何作为用户程序访问内核服务的接口；
4. 理解物理内存分配与页表机制；
5. 理解进程抽象、进程创建与等待；
6. 理解文件系统抽象和用户程序读文件的基本流程；
7. 通过 QEMU 完成可重复运行和可验证测试。

## 3. 项目目录结构

```text
ToyOS-LabC/
├── Makefile
├── README.md
├── docs/
├── displays/
├── include/
├── kernel/
├── scripts/
└── user/
```

各目录作用如下：

- `kernel/`：内核源码，包括启动入口、控制台、trap、系统调用、内存管理、进程管理和文件系统等模块。
- `include/`：公共头文件，定义内核与用户程序共享的数据类型、常量和接口声明。
- `user/`：用户态程序源码，包括 `init`、`sh`、`hello`、`forkdemo`、`cat` 等程序。
- `scripts/`：环境检查、QEMU 运行和 smoke test 等辅助脚本。
- `docs/`：实验报告和课程文档。
- `displays/`：运行输出和自动化测试截图。
- `build/`：编译生成目录，包含内核镜像和用户程序产物，通常不纳入 Git 管理。

## 4. 运行环境

项目已在以下环境中验证：

- Windows + WSL Ubuntu
- `qemu-system-riscv64`
- `riscv64-linux-gnu-gcc`
- GNU Make

Ubuntu / WSL 下依赖安装命令：

```sh
sudo apt update
sudo apt install -y build-essential gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu qemu-system-riscv qemu-utils
```

环境检查命令：

```sh
which riscv64-linux-gnu-gcc
which qemu-system-riscv64
make --version
```

## 5. 编译与运行

进入项目目录并编译：

```sh
cd /mnt/e/ToyOS-LabC
make clean
make
```

编译成功后会生成：

- `build/kernel.elf`
- `build/kernel.bin`
- `build/user/*.elf`

运行命令：

```sh
timeout 8s make run | tee build/demo.log
```

内核运行完成后会进入停机等待状态，因此使用 `timeout` 结束 QEMU 是正常的测试方式。运行日志会同时显示在终端并保存到 `build/demo.log`。

## 6. 自动化测试

执行自动化 smoke test：

```sh
make test
```

测试脚本会完成以下步骤：

1. 重新编译项目；
2. 启动 QEMU；
3. 将运行日志保存到 `build/qemu.log`；
4. 检查 ToyOS 启动、初始化、用户程序、ToyFS 和最终完成标记；
5. 输出 `All smoke tests passed`。

测试通过时，终端末尾会出现：

```text
All smoke tests passed. Log: build/qemu.log
```

## 7. 运行结果

QEMU 启动后，OpenSBI 会先输出平台信息，随后 ToyOS 输出内核和用户程序运行链路。项目当前运行结果包含以下关键日志：

```text
[ToyOS] kernel booting...
[ToyOS] console init ok
[ToyOS] trap init ok
[ToyOS] syscall table init ok
[ToyOS] physical page allocator init ok
[ToyOS] kernel page table init ok
[ToyOS] process table init ok
[ToyOS] ToyFS init ok, files=1
[ToyOS] enter user mode
user: hello from user program
user: syscall write works
user: getpid = 1
user: fork test start
parent: child pid = 2
child: hello
parent: wait child done
user: file system test
cat /hello.txt: Hello from ToyFS!
[ToyOS] all basic tests finished
[ToyOS] demo completed, kernel halted.
```

这些日志对应的含义如下：

- `[ToyOS] kernel booting...` 表明内核已从入口地址进入主初始化流程。
- `console init ok` 表明控制台输出链路可用。
- `trap init ok` 和 `syscall table init ok` 表明异常入口和系统调用接口初始化完成。
- `physical page allocator init ok` 和 `kernel page table init ok` 表明物理页分配与页表初始化完成。
- `process table init ok` 表明进程表初始化完成。
- `ToyFS init ok, files=1` 表明 ToyFS 文件系统初始化完成。
- `enter user mode` 之后的用户日志表明用户程序运行链路完成。
- `cat /hello.txt: Hello from ToyFS!` 表明文件读取链路完成。
- `all basic tests finished` 和 `demo completed` 表明 smoke test 覆盖的基本功能已通过。

## 8. 核心模块说明

### 8.1 启动与入口

ToyOS 的启动入口由 `kernel/entry.S` 提供，链接脚本 `kernel/linker.ld` 将内核入口放置在 `0x80200000`。QEMU 的 RISC-V64 `virt` 平台通过 OpenSBI 加载内核，并将控制权转交给内核入口。

`entry.S` 完成早期执行环境准备，包括设置全局指针和内核栈，然后调用 C 语言入口 `kmain`。`kmain` 负责后续内核模块初始化和运行链路组织。

### 8.2 控制台输出

控制台输出由 `kernel/console.c` 和 `kernel/printf.c` 提供。内核运行在裸机环境中，不依赖标准 C 库，因此项目实现了简化的字符输出、字符串输出和格式化输出函数。

该模块用于输出内核启动、初始化状态、用户程序结果和测试完成标记，是观察系统运行状态的基础通道。

### 8.3 Trap 与系统调用

Trap 是用户态进入内核态、处理异常和系统调用的统一入口。用户程序通过系统调用请求内核服务，内核根据系统调用号分发到对应处理函数。

该机制体现了用户程序和内核服务之间的边界：用户态代码不能直接访问内核内部数据结构，而是通过受控接口完成输出、获取进程号、进程创建、等待和文件读取等操作。

### 8.4 内存管理

项目以 4KB 页为单位进行物理页分配，展示页粒度内存管理的基本思想。物理页分配模块负责维护可用页，并向内核其他模块提供页分配能力。

页表用于地址转换和用户/内核空间隔离。ToyOS 的页表实现聚焦教学目标，展示页表建立、地址映射和访问权限控制等核心概念。

### 8.5 进程管理

进程管理模块展示了进程表、进程号、父子进程、进程创建与等待等基本概念。运行日志中的 `fork test` 展示了父进程创建子进程、子进程执行输出、父进程等待子进程完成的关系。

该模块用于理解操作系统如何管理多个执行实体，以及进程间父子关系和退出等待机制的基本流程。

### 8.6 ToyFS 文件系统

ToyFS 是一个轻量级只读文件系统，用于展示文件抽象和读文件接口。项目内置 `/hello.txt` 文件，用户态 `cat` 程序通过系统调用读取该文件。

运行输出：

```text
cat /hello.txt: Hello from ToyFS!
```

该结果表明用户程序、系统调用、内核文件接口和 ToyFS 之间的文件读取链路已经连通。

### 8.7 用户程序

`user/` 目录中包含多个用户态程序：

- `init`：用户程序初始化入口。
- `sh`：用户态程序组织入口。
- `hello`：验证用户态输出和系统调用。
- `forkdemo`：验证进程创建和等待。
- `cat`：验证 ToyFS 文件读取。

这些程序用于验证 syscall、进程管理和文件系统链路，并展示用户态程序如何通过系统调用访问内核服务。

## 9. 测试截图

ToyOS 运行输出：

![ToyOS 运行输出](displays/output.png)

ToyOS 自动测试：

![ToyOS 自动测试](displays/test.png)

## 10. 实验完成度

项目已完成以下内容：

- RISC-V QEMU 运行环境下的内核启动；
- 内核日志输出；
- trap/syscall 初始化链路；
- 物理页分配和页表初始化链路；
- 进程表初始化与 fork/wait 演示；
- 用户程序输出；
- ToyFS 文件读取演示；
- `make run` 运行日志；
- `make test` 自动化 smoke test。

项目采用轻量化实现方式聚焦课程核心机制，保留了操作系统启动、内核服务、用户程序、进程关系与文件抽象的关键路径。

## 11. 常见问题

### 11.1 为什么运行命令使用 timeout？

内核运行完成后会进入等待状态，不会主动退出 QEMU。`timeout` 用于在自动测试中结束模拟器进程，便于保存日志并返回到宿主机终端。

### 11.2 为什么使用 QEMU？

QEMU 能够模拟 RISC-V64 `virt` 硬件平台，使实验无需真实开发板即可运行。OpenSBI 提供早期固件支持，QEMU 提供串口、内存和基本平台设备。

### 11.3 build 目录是否需要提交？

`build/` 是编译产物目录，通常不需要提交。实验检查和课程展示时，本地执行 `make` 即可重新生成 `build/kernel.elf`、`build/kernel.bin` 和 `build/user/*.elf`。

### 11.4 displays 目录的作用是什么？

`displays/` 用于保存运行和测试截图，便于实验报告、课程汇报和项目展示引用。

## 12. 汇报要点

- 项目从 RISC-V 内核入口开始，完成内核启动和基本初始化。
- 用户程序通过系统调用访问内核服务，体现用户态与内核态的边界。
- 物理页分配和页表初始化体现操作系统内存管理思想。
- `fork/wait` 输出体现进程创建、父子进程关系和等待机制。
- ToyFS 与 `cat` 输出体现文件系统抽象和用户态读文件流程。
- `make test` 提供可重复验证方式，能够自动检查启动、初始化、用户程序、文件读取和最终完成标记。
