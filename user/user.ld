/* User program linker script: each program starts at virtual address 0. */
OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS
{
  . = 0;
  .text : {
    *(.text.entry)
    *(.text .text.*)
  }
  . = ALIGN(16);
  .rodata : {
    *(.srodata .srodata.*)
    *(.rodata .rodata.*)
  }
  . = ALIGN(16);
  PROVIDE(__global_pointer$ = . + 0x800);
  .data : {
    *(.sdata .sdata.*)
    *(.data .data.*)
  }
  . = ALIGN(16);
  .bss : {
    *(.sbss .sbss.*)
    *(.bss .bss.*)
    *(COMMON)
  }
  PROVIDE(end = .);
}
