#include "cpu/exec.h"
#include "monitor/watchpoint.h"
#include "monitor/monitor.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();
extern void raise_intr(uint8_t NO, vaddr_t ret_addr);
make_EHelper(lidt) {
  cpu.idtr.limit = vaddr_read(id_dest->addr, 2);
  if (decoding.is_operand_size_16) {
    cpu.idtr.base = vaddr_read(id_dest->addr + 2, 3);
  }
  else {
    cpu.idtr.base = vaddr_read(id_dest->addr + 2, 4);
  }

  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  raise_intr(id_dest->val, decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  rtl_pop(&decoding.jmp_eip);
  decoding.is_jmp = true;
  rtl_pop(&t0);
  cpu.cs = t0;
  rtl_pop(&cpu.eflags.val);

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  t2 = pio_read(id_src->val, id_src->width);
  operand_write(id_dest, &t2);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val, id_src->width, id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int_3) {
  (*eip)--;
  WP* p = get_breakpoint(*eip);
	if (p) {
    printf("Hit breakpoint %d at address 0x%08x\n", p->NO, *eip);
    uint8_t op = p->old_op;
    vaddr_write(*eip, 1, op);
    printf("program paused\n");
  }
  nemu_state = NEMU_STOP;
}
