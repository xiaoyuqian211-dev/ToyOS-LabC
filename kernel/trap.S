/* RISC-V trap entry and user return path.
 *
 * User page tables map kernel text/data as supervisor-only pages. That lets the
 * CPU fetch this entry immediately after an ecall, then the handler switches to
 * the real kernel page table before running C code.
 */

#define TF_RA          0
#define TF_GP          8
#define TF_TP          16
#define TF_T0          24
#define TF_T1          32
#define TF_T2          40
#define TF_S0          48
#define TF_S1          56
#define TF_A0          64
#define TF_A1          72
#define TF_A2          80
#define TF_A3          88
#define TF_A4          96
#define TF_A5          104
#define TF_A6          112
#define TF_A7          120
#define TF_S2          128
#define TF_S3          136
#define TF_S4          144
#define TF_S5          152
#define TF_S6          160
#define TF_S7          168
#define TF_S8          176
#define TF_S9          184
#define TF_S10         192
#define TF_S11         200
#define TF_T3          208
#define TF_T4          216
#define TF_T5          224
#define TF_T6          232
#define TF_SP          240
#define TF_KERNEL_SP   248
#define TF_EPC         256
#define TF_STATUS      264
#define TF_KERNEL_SATP 272

.section .text
.globl trap_entry
trap_entry:
  csrrw sp, sscratch, sp

  sd ra, TF_RA(sp)
  sd gp, TF_GP(sp)
  sd tp, TF_TP(sp)
  sd t0, TF_T0(sp)
  sd t1, TF_T1(sp)
  sd t2, TF_T2(sp)
  sd s0, TF_S0(sp)
  sd s1, TF_S1(sp)
  sd a0, TF_A0(sp)
  sd a1, TF_A1(sp)
  sd a2, TF_A2(sp)
  sd a3, TF_A3(sp)
  sd a4, TF_A4(sp)
  sd a5, TF_A5(sp)
  sd a6, TF_A6(sp)
  sd a7, TF_A7(sp)
  sd s2, TF_S2(sp)
  sd s3, TF_S3(sp)
  sd s4, TF_S4(sp)
  sd s5, TF_S5(sp)
  sd s6, TF_S6(sp)
  sd s7, TF_S7(sp)
  sd s8, TF_S8(sp)
  sd s9, TF_S9(sp)
  sd s10, TF_S10(sp)
  sd s11, TF_S11(sp)
  sd t3, TF_T3(sp)
  sd t4, TF_T4(sp)
  sd t5, TF_T5(sp)
  sd t6, TF_T6(sp)

  csrr t0, sscratch
  sd t0, TF_SP(sp)
  csrr t0, sepc
  sd t0, TF_EPC(sp)
  csrr t0, sstatus
  sd t0, TF_STATUS(sp)

  ld t0, TF_KERNEL_SATP(sp)
  csrw satp, t0
  sfence.vma
  ld sp, TF_KERNEL_SP(sp)

  call trap_dispatch
  call trap_return

.globl userret
userret:
  csrw satp, a1
  sfence.vma
  mv t0, a0

  ld t1, TF_EPC(t0)
  csrw sepc, t1
  ld t1, TF_STATUS(t0)
  csrw sstatus, t1
  csrw sscratch, t0

  ld sp, TF_SP(t0)
  ld ra, TF_RA(t0)
  ld gp, TF_GP(t0)
  ld tp, TF_TP(t0)
  ld t1, TF_T1(t0)
  ld t2, TF_T2(t0)
  ld s0, TF_S0(t0)
  ld s1, TF_S1(t0)
  ld a0, TF_A0(t0)
  ld a1, TF_A1(t0)
  ld a2, TF_A2(t0)
  ld a3, TF_A3(t0)
  ld a4, TF_A4(t0)
  ld a5, TF_A5(t0)
  ld a6, TF_A6(t0)
  ld a7, TF_A7(t0)
  ld s2, TF_S2(t0)
  ld s3, TF_S3(t0)
  ld s4, TF_S4(t0)
  ld s5, TF_S5(t0)
  ld s6, TF_S6(t0)
  ld s7, TF_S7(t0)
  ld s8, TF_S8(t0)
  ld s9, TF_S9(t0)
  ld s10, TF_S10(t0)
  ld s11, TF_S11(t0)
  ld t3, TF_T3(t0)
  ld t4, TF_T4(t0)
  ld t5, TF_T5(t0)
  ld t6, TF_T6(t0)
  ld t0, TF_T0(t0)
  sret
