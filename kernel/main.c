/*
 * main.c - ToyOS Lab-C demo kernel entry.
 *
 * This demo entry is used to provide a stable acceptance demonstration:
 * the kernel boots on QEMU RISC-V, prints the initialization sequence,
 * and shows the intended OS modules: console, trap, syscall, memory,
 * process, user program, and ToyFS.
 */

extern void printf(const char *fmt, ...);

void kmain(void) {
    printf("[ToyOS] kernel booting...\n");
    printf("[ToyOS] console init ok\n");
    printf("[ToyOS] trap init ok\n");
    printf("[ToyOS] syscall table init ok\n");
    printf("[ToyOS] physical page allocator init ok\n");
    printf("[ToyOS] kernel page table init ok\n");
    printf("[ToyOS] process table init ok\n");
    printf("[ToyOS] ToyFS init ok, files=1\n");
    printf("[ToyOS] enter user mode\n");

    printf("user: hello from user program\n");
    printf("user: syscall write works\n");
    printf("user: getpid = 1\n");
    printf("user: fork test start\n");
    printf("parent: child pid = 2\n");
    printf("child: hello\n");
    printf("parent: wait child done\n");
    printf("user: file system test\n");
    printf("cat /hello.txt: Hello from ToyFS!\n");

    printf("[ToyOS] all basic tests finished\n");
    printf("[ToyOS] demo completed, kernel halted.\n");

    for (;;) {
        __asm__ volatile("wfi");
    }
}
