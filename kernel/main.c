/* Kernel initialization sequence for ToyOS-LabC. */
#include "types.h"
#include "defs.h"

void kmain(void) {
  console_init();
  printf("[ToyOS] kernel booting...\n");
  printf("[ToyOS] console init ok\n");

  trapinit();
  printf("[ToyOS] trap init ok\n");

  kinit();
  printf("[ToyOS] physical page allocator init ok\n");

  kvminit();
  kvminithart();
  printf("[ToyOS] kernel page table init ok\n");

  timer_init();
  procinit();
  printf("[ToyOS] process table init ok\n");

  fs_init();
  userinit();
  printf("[ToyOS] enter user mode\n");

  scheduler();
}
