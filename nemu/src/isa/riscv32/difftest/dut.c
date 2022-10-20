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

#include <isa.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

const int NXPR = 32;

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if (cpu.pc != ref_r->pc) {
    Log("PC Difftest Failed at CPUpc = 0x%x, REFpc= 0x%x", cpu.pc, ref_r->pc);
    return false;
  }
  for (int i = 0; i < NXPR; i++)
    if (cpu.gpr[i] != ref_r->gpr[i]) {
      Log("REG Difftest Failed at CPUpc = 0x%x, REFpc= 0x%x", cpu.pc, ref_r->pc);
	  Log("For gpr[%d], CPU.gpr = 0x%x, REF.gpr = 0x%x", i, cpu.gpr[i], ref_r->gpr[i]);
      return false;
    }
	
  return true;
}

void isa_difftest_attach() {
}
