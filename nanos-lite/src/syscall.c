#include <common.h>
#include "syscall.h"
void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  printf("arrived!\n");
  switch (a[0]) {
	case SYS_yield: yield(); c->GPRx = 1; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
