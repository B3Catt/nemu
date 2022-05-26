#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  /*if (id_dest->type == OP_TYPE_MEM) {
    rtl_lm(&t1, &(id_dest->addr), id_dest->width);
    rtl_push(&t1);
  }
  else if (id_dest->type == OP_TYPE_REG) {
    rtl_push(&(id_dest->val));
  }
  else if (id_dest->type == OP_TYPE_IMM) {
    rtl_push(&id_dest->val);
	}*/
	rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t1);
  operand_write(id_dest, &t1);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  if (decoding.is_operand_size_16) {
    t2 = reg_w(R_SP);
    t0 = reg_w(R_CX);
    rtl_push(&t0);
    t0 = reg_w(R_DX);
    rtl_push(&t0);
    t0 = reg_w(R_BX);
    rtl_push(&t0);
    rtl_push(&t2);
    t0 = reg_w(R_BP);
    rtl_push(&t0);
    t0 = reg_w(R_SI);
    rtl_push(&t0);
    t0 = reg_w(R_DI);
    rtl_push(&t0);
  }
  else {
    t2 = reg_l(R_ESP);
    rtl_push(&reg_l(R_EAX));
    rtl_push(&reg_l(R_ECX));
    rtl_push(&reg_l(R_EDX));
    rtl_push(&reg_l(R_EBX));
    rtl_push(&t2);
    rtl_push(&reg_l(R_EBP));
    rtl_push(&reg_l(R_ESI));
    rtl_push(&reg_l(R_EDI));
  }

  print_asm("pusha");
}

make_EHelper(popa) {
	printf("g\n");
  if (decoding.is_operand_size_16) {
    rtl_pop(&t0);
    rtl_sr(R_DI, 2, &t0);
    rtl_pop(&t0);
    rtl_sr(R_SI, 2, &t0);
    rtl_pop(&t0);
    rtl_sr(R_BP, 2, &t0);
    rtl_pop(&t0);
    rtl_pop(&t0);
    rtl_sr(R_BX, 2, &t0);
    rtl_pop(&t0);
    rtl_sr(R_DX, 2, &t0);
    rtl_pop(&t0);
    rtl_sr(R_CX, 2, &t0);
    rtl_pop(&t0);
    rtl_sr(R_AX, 2, &t0);
  }
  else {
    rtl_pop(&t0);
    rtl_sr(R_EDI, 4, &t0);
    rtl_pop(&t0);
    rtl_sr(R_ESI, 4, &t0);
    rtl_pop(&t0);
    rtl_sr(R_EBP, 4, &t0);
    rtl_pop(&t0);
    rtl_pop(&t0);
    rtl_sr(R_EBX, 4, &t0);
    rtl_pop(&t0);
    rtl_sr(R_EDX, 4, &t0);
    rtl_pop(&t0);
    rtl_sr(R_ECX, 4, &t0);
    rtl_pop(&t0);
    rtl_sr(R_EAX, 4, &t0);
  }
	printf("g\n");
  print_asm("popa");
}

make_EHelper(leave) {
  rtl_lr(&t2, R_EBP, 4);
  rtl_sr(R_ESP, 4, &t2);
  rtl_pop(&t2);
  rtl_sr(R_EBP, 4, &t2);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    rtl_lr_w(&t2, R_AX);
    rtl_msb(&t3, &t2, 2);
    if (t3)
      t0 = 0xffff;
    else t0 = 0;
    rtl_sr_w(R_DX, &t0);
  }
  else {
    rtl_lr_l(&t2, R_AX);
    rtl_msb(&t3, &t2, 4);
    if (t3)
      t0 = 0xffffffff;
    else t0 = 0;
    rtl_sr_l(R_EDX, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr_b(&t2, R_AL);
    rtl_sext(&t0, &t2, 1);
    rtl_sr_b(R_AH, &t0);
  }
  else {
    rtl_lr_w(&t2, R_AX);
    rtl_sext(&t0, &t2, 2);
    rtl_sr_l(R_EAX, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
