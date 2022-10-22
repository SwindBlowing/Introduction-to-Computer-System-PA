#include <common.h>
#include "syscall.h"

#define CONFIG_ITRACE

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

#ifdef CONFIG_ITRACE
  printf("System call ID = %u\n", a[0]);
  printf("GPRS value: %x %x %x %x %x\n", c->GPR1, c->GPR2, c->GPR3, c->GPR4, c->GPRx);
#endif

  switch (a[0]) {
	case SYS_exit: halt(c->GPR2); break;
	case SYS_yield: yield(); c->GPRx = 0; break;
	case SYS_write: {
		printf("???%u\n", c->GPRx);
		if (c->GPR2 == 1 || c->GPR2 == 2) {
			for (int i = 0; i < c->GPR4; i++)
				putch(*(char *)(c->GPR3 + i));
			c->GPRx = c->GPR4;
		}
		else c->GPRx = -1;
		break;
	}
	case SYS_brk: c->GPRx = 0; break;
    default: panic("Unhandled syscall ID = %u", a[0]);
  }

}
