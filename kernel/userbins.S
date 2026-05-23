/* Embedded user ELF images. Build rules generate these ELF files first. */
.section .rodata
.balign 8
.globl user_init_start
user_init_start:
  .incbin "build/user/init.elf"
.globl user_init_end
user_init_end:

.balign 8
.globl user_sh_start
user_sh_start:
  .incbin "build/user/sh.elf"
.globl user_sh_end
user_sh_end:

.balign 8
.globl user_hello_start
user_hello_start:
  .incbin "build/user/hello.elf"
.globl user_hello_end
user_hello_end:

.balign 8
.globl user_forkdemo_start
user_forkdemo_start:
  .incbin "build/user/forkdemo.elf"
.globl user_forkdemo_end
user_forkdemo_end:

.balign 8
.globl user_cat_start
user_cat_start:
  .incbin "build/user/cat.elf"
.globl user_cat_end
user_cat_end:
