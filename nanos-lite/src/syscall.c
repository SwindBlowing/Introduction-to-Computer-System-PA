#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  assert(0);
#ifdef CONFIG_STRACE
  printf("System call ID = %u\n", a[0]);
  printf("GPRS value: %x %x %x %x %x\n", c->GPR1, c->GPR2, c->GPR3, c->GPR4, c->GPRx);
#endif
  switch (a[0]) {
	case SYS_exit: halt(c->GPR2); break;
	case SYS_yield: yield(); c->GPRx = 0; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

}
