#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  //printf("%x %x %x\n", c->mcause, c->mepc, c->mstatus);
  //assert(0);
  printf("%x\n", c->gpr[2]);
  __am_get_cur_as(c);
  if (user_handler) {
	//printf("%x %x\n",c->mcause, c->GPR1);
    Event ev = {0};
    switch (c->mcause) {
	  case 0xb: {
		switch (c->GPR1) {
			case 0x0:
			case 0x1: 
			case 0x2:
			case 0x3:
			case 0x4:
			case 0x5:
			case 0x6: 
			case 0x7:
			case 0x8:
			case 0x9:
			case 0xa:
			case 0xb: 
			case 0xc:
			case 0xd:
			case 0xe:
			case 0xf:
			case 0x10: 
			case 0x11:
			case 0x12:
			case 0x13:
				ev.event = EVENT_SYSCALL; 
				c->mepc += 4;
				break;
			case 0xffffffff:
				ev.event = EVENT_YIELD; c->mepc += 4; break;
			default: ev.event = EVENT_ERROR;  break;
		}
		break;
	  }
	  case 0x80000007: ev.event = EVENT_IRQ_TIMER; break;
      default: ev.event = EVENT_ERROR;  break;
    }
	//printf("%x\n", c->gpr[28]);
    c = user_handler(ev, c);
	//printf("%x\n", c->gpr[28]);
    assert(c != NULL);
  }
//printf("mepc:%x\n", c->mepc);
  //printf("%x\n", c->gpr[28]);
  __am_switch(c);
  printf("%x\n", c->gpr[2]);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {

  Context *cp = (Context *)kstack.end - 1;
  cp->mepc = (uintptr_t)entry;
  cp->mstatus = 0x1800 | 0x80;

  cp->gpr[10] = (uintptr_t)arg;
  cp->pdir = NULL;

  cp->np = 0;
  cp->gpr[2] = (uintptr_t)cp;

  return cp;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
