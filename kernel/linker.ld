/* Kernel linker script: OpenSBI loads the S-mode kernel at 0x80200000. */
OUTPUT_ARCH(riscv)
ENTRY(_entry)

SECTIONS
{
  . = 0x80200000;
  PROVIDE(kernel_start = .);

  . = ALIGN(4096);
  .text : {
    *(.text.entry)
    *(.text .text.*)
  }

  . = ALIGN(4096);
  .rodata : {
    *(.srodata .srodata.*)
    *(.rodata .rodata.*)
  }

  . = ALIGN(4096);
  PROVIDE(__global_pointer$ = . + 0x800);
  .data : {
    *(.sdata .sdata.*)
    *(.data .data.*)
  }

  . = ALIGN(4096);
  .bss : {
    PROVIDE(bss_start = .);
    *(.sbss .sbss.*)
    *(.bss .bss.*)
    *(COMMON)
    PROVIDE(bss_end = .);
  }

  . = ALIGN(4096);
  PROVIDE(end = .);
}
