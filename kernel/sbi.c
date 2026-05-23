/* Minimal SBI calls used for timer programming and clean QEMU shutdown. */
#include "types.h"
#include "defs.h"

#define SBI_EXT_TIME 0x54494D45UL
#define SBI_EXT_SRST 0x53525354UL

struct sbiret {
  long error;
  long value;
};

static struct sbiret sbi_call(uint64 ext, uint64 fid, uint64 arg0, uint64 arg1,
                              uint64 arg2) {
  register uint64 a0 asm("a0") = arg0;
  register uint64 a1 asm("a1") = arg1;
  register uint64 a2 asm("a2") = arg2;
  register uint64 a6 asm("a6") = fid;
  register uint64 a7 asm("a7") = ext;
  asm volatile("ecall"
               : "+r"(a0), "+r"(a1)
               : "r"(a2), "r"(a6), "r"(a7)
               : "memory");
  struct sbiret ret = {(long)a0, (long)a1};
  return ret;
}

void sbi_set_timer(uint64 stime) {
  (void)sbi_call(SBI_EXT_TIME, 0, stime, 0, 0);
}

void sbi_shutdown(void) {
  (void)sbi_call(SBI_EXT_SRST, 0, 0, 0, 0);
  for (;;) {
    asm volatile("wfi");
  }
}
