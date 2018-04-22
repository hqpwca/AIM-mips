// by ZBY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <arch-trap.h>
.text

.globl _start
_start:
    li sp, KERN_START
    j master_early_init

.globl high_address_entry
high_address_entry:
    li sp, KERN_BASE
    j master_init

.globl slave_upper_entry
slave_upper_entry:
    j slave_upper_entry // FIXME

.globl trap_entry
.align 4
trap_entry:
    csrrw sp, sscratch, sp // swap sscratch and sp
    beqz sp, trap_from_kernel
  // trap from user
  // now_sp(old_sscratch) is kernel stack
  // old_sp(now_sscratch) is user stack
  STORE sp,32*REGBYTES(sp)//save old_sscatch(i.e. kernel stack) to tf->sscratch
save_registers:
  # Preserve the registers.  Compute the address of the trap handler.
  STORE x0, (sp) # Zero x0's save slot.
  STORE ra, 1*REGBYTES(sp)
  STORE gp, 3*REGBYTES(sp)
  STORE tp, 4*REGBYTES(sp)
  STORE t0, 5*REGBYTES(sp)
  STORE t1, 6*REGBYTES(sp)
  STORE t2, 7*REGBYTES(sp)
  STORE s0, 8*REGBYTES(sp)
  STORE s1, 9*REGBYTES(sp)
  STORE a0,10*REGBYTES(sp)
  STORE a1,11*REGBYTES(sp)
  STORE a2,12*REGBYTES(sp)
  STORE a3,13*REGBYTES(sp)
  STORE a4,14*REGBYTES(sp)
  STORE a5,15*REGBYTES(sp)
  STORE a6,16*REGBYTES(sp)
  STORE a7,17*REGBYTES(sp)
  STORE s2,18*REGBYTES(sp)
  STORE s3,19*REGBYTES(sp)
  STORE s4,20*REGBYTES(sp)
  STORE s5,21*REGBYTES(sp)
  STORE s6,22*REGBYTES(sp)
  STORE s7,23*REGBYTES(sp)
  STORE s8,24*REGBYTES(sp)
  STORE s9,25*REGBYTES(sp)
  STORE s10,26*REGBYTES(sp)
  STORE s11,27*REGBYTES(sp)
  STORE t3,28*REGBYTES(sp)
  STORE t4,29*REGBYTES(sp)
  STORE t5,30*REGBYTES(sp)
  STORE t6,31*REGBYTES(sp)
  // make sscratch always zero in kernel mode
  // save now_sscratch(user stack||kernel stack) to tf->sp
  csrrw t0, sscratch, x0           # t0 <- user/kernel sp
  STORE t0, 2*REGBYTES(sp)         # sp
  
  csrr t0, sepc
  STORE t0, 33*REGBYTES(sp)
  csrr t0, scause
  STORE t0, 34*REGBYTES(sp)
  csrr t0, 0x143//stval
  STORE t0, 35*REGBYTES(sp)
  csrr t0, sstatus
  STORE t0, 36*REGBYTES(sp)
  
  mv a0, sp                        # a0 <- regs
    j trap_handler
trap_from_kernel:
  // trap from kernel mode
  // old_sscratch is zero, restore sp to kernel stack
  // now_sscratch is kernel stack
    csrr sp, sscratch
    addi sp, sp, -SIZEOF_TRAPFRAME
    STORE x0,32*REGBYTES(sp) // save zero to tf->sscratch
    j save_registers




.globl trap_exit
trap_exit:
  mv sp,a0
  # Restore sscratch
  LOAD t0,32*REGBYTES(sp)
  csrw sscratch, t0
  
  #restore other csr
  LOAD t0,33*REGBYTES(sp)
  csrw sepc, t0
  LOAD t0,34*REGBYTES(sp)
  csrw scause, t0
  LOAD t0,35*REGBYTES(sp)
  csrw 0x143/*stval*/, t0
  LOAD t0,36*REGBYTES(sp)
  csrw sstatus, t0
  
  # Restore all of the registers.
  LOAD ra, 1*REGBYTES(sp)
  LOAD gp, 3*REGBYTES(sp)
  LOAD tp, 4*REGBYTES(sp)
  LOAD t0, 5*REGBYTES(sp)
  LOAD t1, 6*REGBYTES(sp)
  LOAD t2, 7*REGBYTES(sp)
  LOAD s0, 8*REGBYTES(sp)
  LOAD s1, 9*REGBYTES(sp)
  LOAD a0,10*REGBYTES(sp)
  LOAD a1,11*REGBYTES(sp)
  LOAD a2,12*REGBYTES(sp)
  LOAD a3,13*REGBYTES(sp)
  LOAD a4,14*REGBYTES(sp)
  LOAD a5,15*REGBYTES(sp)
  LOAD a6,16*REGBYTES(sp)
  LOAD a7,17*REGBYTES(sp)
  LOAD s2,18*REGBYTES(sp)
  LOAD s3,19*REGBYTES(sp)
  LOAD s4,20*REGBYTES(sp)
  LOAD s5,21*REGBYTES(sp)
  LOAD s6,22*REGBYTES(sp)
  LOAD s7,23*REGBYTES(sp)
  LOAD s8,24*REGBYTES(sp)
  LOAD s9,25*REGBYTES(sp)
  LOAD s10,26*REGBYTES(sp)
  LOAD s11,27*REGBYTES(sp)
  LOAD t3,28*REGBYTES(sp)
  LOAD t4,29*REGBYTES(sp)
  LOAD t5,30*REGBYTES(sp)
  LOAD t6,31*REGBYTES(sp)
  LOAD sp, 2*REGBYTES(sp)
  sret
  
.globl switch_regs
switch_regs:
  //save callee-saved registers (sp, s0, s1, s2-s11)
  //   althrough ra is caller-saved
  //   but we MUST save it as well
  //   because we are SWITCHING CONTEXT and pc(ra) is part of it
  
  // FIXME: should we save gp, tp and CSR???
  
  // a0 is struct context for current thread
  // a1 is struct context for target thread

  // save current thread
  STORE ra, 1*REGBYTES(a0)
  STORE sp, 2*REGBYTES(a0)
  STORE s0, 8*REGBYTES(a0)
  STORE s1, 9*REGBYTES(a0)
  STORE s2,18*REGBYTES(a0)
  STORE s3,19*REGBYTES(a0)
  STORE s4,20*REGBYTES(a0)
  STORE s5,21*REGBYTES(a0)
  STORE s6,22*REGBYTES(a0)
  STORE s7,23*REGBYTES(a0)
  STORE s8,24*REGBYTES(a0)
  STORE s9,25*REGBYTES(a0)
  STORE s10,26*REGBYTES(a0)
  STORE s11,27*REGBYTES(a0)
  
  // load target thread
  LOAD ra, 1*REGBYTES(a1)
  LOAD sp, 2*REGBYTES(a1)
  LOAD s0, 8*REGBYTES(a1)
  LOAD s1, 9*REGBYTES(a1)
  LOAD s2,18*REGBYTES(a1)
  LOAD s3,19*REGBYTES(a1)
  LOAD s4,20*REGBYTES(a1)
  LOAD s5,21*REGBYTES(a1)
  LOAD s6,22*REGBYTES(a1)
  LOAD s7,23*REGBYTES(a1)
  LOAD s8,24*REGBYTES(a1)
  LOAD s9,25*REGBYTES(a1)
  LOAD s10,26*REGBYTES(a1)
  LOAD s11,27*REGBYTES(a1)
  ret

