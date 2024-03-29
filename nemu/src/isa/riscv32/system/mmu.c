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
#include <memory/paddr.h>
#include <memory/vaddr.h>
#include <cpu/cpu.h>

#define VPN0(x) (((vaddr_t)(x) & 0x003ff000) >> 12)
#define VPN1(x) (((vaddr_t)(x) & 0xffc00000) >> 22)
#define PTE_PPN(x) ((vaddr_t)(x) >> 10)
#define satp_PPN (cpu.satp & 0x3fffff)
#define offset(x) ((vaddr_t)(x) & 0xfff)

typedef uint32_t PTE;

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  //return MEM_RET_FAIL;
  //printf("\nvaddr:%x\n", vaddr);
  paddr_t PTE_loc = satp_PPN * 4096 + VPN1(vaddr) * 4;
  //printf("PTE_loc:%x\n", PTE_loc);
  PTE firstPTE = paddr_read(PTE_loc, sizeof(PTE));
  //printf("firstPTE:%x\n", firstPTE);
  Assert(firstPTE & 0x1, "firstPTE %x with vaddr %x is invalid", firstPTE, vaddr);

  paddr_t leaf_PTE_loc = PTE_PPN(firstPTE) * 4096 + VPN0(vaddr) * 4;
  //printf("leaf_PTE_loc:%x\n", leaf_PTE_loc);
  PTE leafPTE = paddr_read(leaf_PTE_loc, sizeof(PTE));
  Assert(leafPTE & 0x1, "leafPTE %x with vaddr %x is invalid", leafPTE, vaddr);

  paddr_t paddr = PTE_PPN(leafPTE) * 4096 + offset(vaddr);
  //printf("%x %x\n", vaddr, paddr);
  //Assert(paddr == vaddr, "Incorrect translation with va = %x and pa = %x", vaddr, paddr);
	//assert(0);
  return paddr;
}
