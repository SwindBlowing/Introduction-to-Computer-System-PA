/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write
#define CSR(imm) BITS(imm, 7, 0)

#ifdef CONFIG_ETRACE
void print_exception(uint32_t pc, uint32_t dnpc, uint32_t errorId);
void exception_return(uint32_t pc, uint32_t dnpc);
#endif

#ifdef CONFIG_FTRACE
void check_funct(uint32_t nowpc, uint32_t jmp_add, uint32_t snpc);
#endif

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_R, TYPE_B,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = ((SEXT(BITS(i, 31, 31), 1) << 19) | (BITS(i, 19, 12) << 11) | (BITS(i, 20, 20) << 10) | BITS(i, 30, 21)) << 1; } while(0)
#define immB() do { *imm = ((SEXT(BITS(i, 31, 31), 1) << 11) | (BITS(i, 7, 7) << 10) | (BITS(i, 30, 25) << 4) | BITS(i, 11, 8)) << 1;} while (0)

static void decode_operand(Decode *s, int *dest, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rd  = BITS(i, 11, 7);
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *dest = rd;
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J:                   immJ(); break;
    case TYPE_R: src1R(); src2R();         break;
    case TYPE_B: src1R(); src2R(); immB(); break;
  }
}

static word_t * get_csr(word_t imm)
{
	if (imm == 0x341) return &cpu.mepc;
	if (imm == 0x305) return &cpu.mtvec;
	if (imm == 0x300) return &cpu.mstatus;
	if (imm == 0x342) return &cpu.mcause;
	panic("Invalid CSR id 0x%x", imm);
}

static int decode_exec(Decode *s) {

  int dest = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &dest, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(dest) = imm);
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(dest) = imm + s->pc);
  
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(dest) = SEXT(Mr(src1 + imm, 1), 8));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(dest) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(dest) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(dest) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(dest) = Mr(src1 + imm, 2));
  
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(dest) = imm + src1);
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(dest) = ((int32_t)src1 < (int32_t)imm) ? 1 : 0);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(dest) = (src1 < imm) ? 1 : 0);
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(dest) = imm ^ src1);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(dest) = imm | src1);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(dest) = imm & src1);
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, R(dest) = src1 << (imm & 0x0000001Fu));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, R(dest) = src1 >> (imm & 0x0000001Fu));
  INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai   , I, R(dest) = (int32_t)src1 >> (imm & 0x0000001Fu));
  
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, s->dnpc = imm + s->pc, R(dest) = s->snpc; IFDEF(CONFIG_FTRACE, check_funct(s->pc, s->dnpc, s->snpc)););
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, s->dnpc = (imm + src1) & 0xFFFFFFFEu, R(dest) = s->snpc; IFDEF(CONFIG_FTRACE, check_funct(s->pc, s->dnpc, s->snpc)););

  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  , I, if (dest) R(dest) = *get_csr(imm); *get_csr(imm) = src1);
  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs  , I, R(dest) = *get_csr(imm); if (BITS(s->isa.inst.val, 19, 15)) *get_csr(imm) |= src1);
  
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, s->dnpc = (src1 == src2) ? imm + s->pc : s->dnpc);
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, s->dnpc = (src1 != src2) ? imm + s->pc : s->dnpc);
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, s->dnpc = ((int32_t)src1 < (int32_t)src2) ? imm + s->pc : s->dnpc); 
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, s->dnpc = ((int32_t)src1 >= (int32_t)src2) ? imm + s->pc : s->dnpc);
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, s->dnpc = (src1 < src2) ? imm + s->pc : s->dnpc);
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, s->dnpc = (src1 >= src2) ? imm + s->pc : s->dnpc);
  
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(dest) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(dest) = src1 - src2);
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(dest) = src1 << (src2 & 0x0000001Fu));
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(dest) = ((int32_t)src1 < (int32_t)src2) ? 1 : 0);
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(dest) = (src1 < src2) ? 1 : 0);
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(dest) = src1 ^ src2);
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(dest) = (int32_t)src1 >> (src2 & 0x0000001Fu));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(dest) = src1 >> (src2 & 0x0000001Fu));
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(dest) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(dest) = src1 & src2);
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(dest) = (uint64_t)src1 * (uint64_t)src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, R(dest) = BITS(SEXT(src1, 32) * SEXT(src2, 32), 63, 32));
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, R(dest) = BITS((uint64_t)src1 * (uint64_t)src2, 63, 32));
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(dest) = (int32_t)src1 / (int32_t)src2);
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(dest) = src1 / src2);
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(dest) = (int32_t)src1 % (int32_t)src2);
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(dest) = src1 % src2);

  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret   , N, s->dnpc = cpu.mepc; IFDEF(CONFIG_ETRACE, exception_return(s->pc, s->dnpc)));
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall  , N, s->dnpc = isa_raise_intr(0x0000000b, s->pc); IFDEF(CONFIG_ETRACE, print_exception(s->pc, s->dnpc, 0xb)));
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();


  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
