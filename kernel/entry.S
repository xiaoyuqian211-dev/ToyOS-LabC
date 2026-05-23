/* First kernel instructions after OpenSBI transfers control to ToyOS. */
.section .text.entry
.globl _entry
_entry:
  .option push
  .option norelax
  la gp, __global_pointer$
  .option pop

  la sp, boot_stack_top
  call kmain

1:
  wfi
  j 1b

.section .bss
.align 12
boot_stack:
  .space 4096
boot_stack_top:
