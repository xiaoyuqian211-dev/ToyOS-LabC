# ToyOS-LabC

ToyOS-LabC 是一个面向《操作系统》实验 C 的教学型小操作系统项目，目标平台为 `qemu-system-riscv64` 的 `virt` 机器。它不是生产级内核，而是用尽量小的代码展示启动、串口输出、trap/系统调用、Sv39 页表、物理页分配、进程调度、fork/wait/exec、只读 ToyFS 和用户程序运行的完整链路。

本仓库代码为原创教学实现，没有复制 xv6 或现成 toyOS 工程。复杂机制采用了适合课堂展示的简化设计，并在下方说明取舍。

## 目录结构

```text
ToyOS-LabC/
  Makefile
  README.md
  docs/
    实验报告.md
  include/
    types.h param.h memlayout.h riscv.h defs.h proc.h syscall.h fs.h
  kernel/
    entry.S linker.ld main.c console.c printf.c sbi.c trap.S trap.c
    syscall.c kalloc.c vm.c proc.c swtch.S timer.c fs.c string.c userbins.S
  user/
    user.h syscall.c usys.S user.ld init.c sh.c hello.c forkdemo.c cat.c
  scripts/
    check_env.sh run_qemu.sh run_tests.sh
```

`kernel/userbins.S` 和 `user/user.ld` 是为内嵌用户 ELF 程序服务的辅助文件，保留在源码树中便于构建。

## 环境依赖

推荐在 Ubuntu 或 WSL2 Ubuntu 中运行：

```sh
sudo apt update
sudo apt install -y build-essential gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu qemu-system-misc
```

也可以使用 `riscv64-unknown-elf-gcc` 工具链。Makefile 会优先选择 `riscv64-unknown-elf-`，找不到时使用 `riscv64-linux-gnu-`。

当前 Windows 环境检测结果：未找到 Windows 侧 `make`、RISC-V GCC 和 QEMU；检测到 `wsl.exe`，但当前没有可用 WSL 发行版。因此本机无法直接完成 QEMU 实机验证，源码和脚本按标准 Ubuntu/WSL2 环境编写。

检查环境：

```sh
sh scripts/check_env.sh
```

## Windows / WSL2 / Ubuntu 运行方式

Windows 推荐使用 WSL2：

```powershell
wsl --install -d Ubuntu
```

安装完成后进入 Ubuntu，将仓库放在 Linux 文件系统或 `/mnt/e/ToyOS-LabC` 下均可，然后执行：

```sh
cd /mnt/e/ToyOS-LabC
sudo apt update
sudo apt install -y build-essential gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu qemu-system-misc
make clean
make
make run
```

Ubuntu 原生环境直接在项目根目录运行同样命令。

## 编译命令

```sh
make clean
make
```

生成物位于 `build/`：

```text
build/kernel.elf
build/kernel.bin
build/user/*.elf
```

## 运行命令

```sh
make run
```

等价脚本：

```sh
sh scripts/run_qemu.sh
```

QEMU 使用：

```text
qemu-system-riscv64 -machine virt -bios default -m 128M -smp 1 -nographic -kernel build/kernel.elf
```

## 功能列表

- RISC-V 64 位 S-mode 内核，OpenSBI 加载，链接地址 `0x80200000`
- QEMU virt UART 串口输出
- 简化 `printf`，支持 `%s`、`%d`、`%x`、`%p`、`%c`
- `panic` 停机输出
- trap 入口汇编、异常分类、系统调用分派
- 系统调用：`write`、`exit`、`getpid`、`yield`、`fork`、`wait`、`exec`、`readfile`、`info`
- 4 KiB 物理页分配器：`kinit`、`kalloc`、`kfree`
- Sv39 页表：`kvminit`、`walk`、`mappages`、`uvmcreate`、`uvmalloc`、`uvmfree`、`copyin`、`copyout`、`copyinstr`
- 用户态和内核态隔离：用户页表只给用户程序页设置 `PTE_U`
- 进程表、内核栈、trapframe、上下文切换、协作式调度
- `fork/wait/exec`，用户 ELF 内嵌到内核并由内核加载
- 用户程序：`init`、`sh`、`hello`、`forkdemo`、`cat`
- ToyFS 只读内存文件系统，内置 `/hello.txt`
- `make test` 冒烟测试脚本

## 预期输出

`make run` 后串口终端应能看到类似输出。OpenSBI 自身 banner 可能会出现在 ToyOS 日志之前。

```text
[ToyOS] kernel booting...
[ToyOS] console init ok
[ToyOS] trap init ok
[ToyOS] physical page allocator init ok
[ToyOS] kernel page table init ok
[ToyOS] process table init ok
[ToyOS] ToyFS init ok, files=1
[ToyOS] enter user mode
sh$ help
builtins: help hello forkdemo cat hello.txt ps info exit
sh$ hello
user: hello from user program
user: syscall write works
user: my pid is 2
sh$ forkdemo
user: fork test start
parent: child pid = 4
child: hello
parent: wait pid = 4 status = 0
sh$ cat hello.txt
user: file system test
cat /hello.txt: Hello from ToyFS!
sh$ ps
[ToyOS] process table:
  pid=1 state=RUNNING name=sh parent=0
sh$ exit
[ToyOS] all basic tests finished
```

## 测试

```sh
make test
```

测试脚本会重新构建内核，运行 QEMU，并检查串口日志中的关键字符串：

- 内核启动和初始化日志
- `printf`/串口输出
- `SYS_write`
- `getpid`
- `fork/wait`
- `yield`
- `cat /hello.txt`
- 最终完成标记

日志保存到：

```text
build/qemu.log
```

## 常见问题

`make: command not found`

安装 `build-essential`，或确认正在 WSL2/Ubuntu 里运行。

`riscv64-linux-gnu-gcc: command not found`

安装：

```sh
sudo apt install -y gcc-riscv64-linux-gnu binutils-riscv64-linux-gnu
```

`qemu-system-riscv64: command not found`

安装：

```sh
sudo apt install -y qemu-system-misc
```

QEMU 只显示 OpenSBI，没有 ToyOS 日志

确认执行的是 `make run`，并且 `build/kernel.elf` 已成功生成。还可以运行 `make clean && make` 查看是否有链接错误。

QEMU 不退出

正常情况下 pid 1 的 shell 执行 `exit` 后会通过 SBI reset 扩展关闭 QEMU。若宿主 OpenSBI/QEMU 版本不支持该扩展，可以按 `Ctrl+A` 再按 `X` 退出。

## 实验完成度说明

已完成实验要求的核心链路：启动、链接、控制台、printf、trap、syscall、页分配、Sv39 页表、用户/内核隔离、进程表、上下文切换、fork/wait/exec、用户程序、脚本式 shell、ToyFS、测试脚本和实验报告。

教学简化点：

- 调度为协作式，主要通过 `SYS_yield` 让出 CPU；timer trap 已分类但默认不启用抢占。
- ToyFS 是只读内存文件表，不实现磁盘块、inode 位图、目录项写入或缓存。
- Shell 为自动脚本式演示 shell，不从 UART 读取交互输入。
- `exec` 从内核内嵌的用户 ELF 表加载程序，不实现真实磁盘上的可执行文件查找。
- 没有实现多核、锁、中断嵌套、文件描述符表或写文件。

这些简化保留了操作系统课程最关键的机制边界，便于阅读、调试和课堂展示。
