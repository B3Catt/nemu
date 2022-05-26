#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  rtl_push(&cpu.eflags.val);
  t2  = cpu.cs;
  rtl_push(&t2);
  rtl_push(&ret_addr);

  t0 = cpu.idtr.base + 8 * NO;
  rtl_lm(&t2, &t0, 4);
  rtl_andi(&t2, &t2, 0xffff);

  rtl_addi(&t0, &t0, 4);
  rtl_lm(&t3, &t0, 4);
  rtl_andi(&t3, &t3, 0xffff0000);
  rtl_or(&t1, &t2, &t3);

  decoding.jmp_eip = t1;
  decoding.is_jmp = true;
}

void dev_raise_intr() {
}
