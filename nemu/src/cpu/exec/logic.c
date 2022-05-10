#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t2, &id_dest->val, &id_src->val);
  rtl_update_ZFSF(&t2, id_dest->width);
  t3 = 0;
  rtl_set_CF(&t3);
  rtl_set_OF(&t3);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  t3 = 0;
  rtl_set_CF(&t3);
  rtl_set_OF(&t3);

  print_asm_template2(and);
}

make_EHelper(xor) {
	rtl_xor(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  t3 = 0;
  rtl_set_CF(&t3);
  rtl_set_OF(&t3);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  rtl_update_ZFSF(&t2, id_dest->width);
  t3 = 0;
  rtl_set_CF(&t3);
  rtl_set_OF(&t3);

	print_asm_template2(or);
}

make_EHelper(sar) {
  rtl_sext(&t2, &id_dest->val, id_dest->width);
  rtl_sar(&t3, &t2, &id_src->val);
  operand_write(id_dest, &t3);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t3, id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  rtl_shl(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  rtl_shr(&t2, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t2);
  // unnecessary to update CF and OF in NEMU
  rtl_update_ZFSF(&t2, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  rtl_not(&id_dest->val);
  operand_write(id_dest, &id_dest->val);

  print_asm_template1(not);
}

make_EHelper(rol) {
  rtl_mv(&t2, &id_src->val);
  while (t2 != 0) {
    rtl_msb(&t3, &id_dest->val, id_dest->width);
    rtl_shli(&id_dest->val, &id_dest->val, 1);
    rtl_add(&id_dest->val, &id_dest->val, &t3);
    rtl_subi(&t2, &t2, 1);
  }
  if (id_src->val == 1) {
    rtl_msb(&t3, &id_dest->val, id_dest->width);
    if (t3 != cpu.eflags.CF)
      t0 = 1;
    else t0 = 0;
    rtl_set_OF(&t0);
  }
  
  print_asm_template2(rol);
}
