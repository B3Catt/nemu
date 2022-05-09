#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  rtl_push(eip);
  rtl_add(&decoding.jmp_eip, eip, &id_dest->val);
  decoding.is_jmp = 1;
	printf("call : 0x%08x\n", decoding.jmp_eip);
	print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {  
  rtl_pop(&decoding.jmp_eip);
  decoding.is_jmp = 1;
  if (decoding.opcode == 0xc2) {
    rtl_lr(&t0, R_ESP, 4);
    rtl_addi(&t0, &t0, id_dest->val);
    rtl_sr(R_ESP, 4, &t0);
  }
	printf("ret : 0x%x\n", decoding.jmp_eip);
	print_asm("ret");
}

make_EHelper(call_rm) {
  rtl_push(eip);
  rtl_mv(&decoding.jmp_eip, &id_dest->val);
  decoding.is_jmp = 1;
	printf("call_rm : 0x%08x\n", decoding.jmp_eip);
  print_asm("call *%s", id_dest->str);
}
