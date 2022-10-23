#include <common.h>
#include <fs.h>
#include "syscall.h"

//#define CONFIG_STRACE

#ifdef CONFIG_STRACE
static char nowFile[999] = {'\0'};
#endif

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

#ifdef CONFIG_STRACE
  if (a[0] == SYS_open) strcpy(nowFile, (const char *)a[1]);
  if (nowFile != NULL) printf("Visited from file %s\n", nowFile);
  printf("System call ID = %u\n", a[0]);
  printf("GPRS value: %x %x %x %x %x\n", c->GPR1, c->GPR2, c->GPR3, c->GPR4, c->GPRx);
#endif

  switch (a[0]) {
	case SYS_exit: halt(a[1]); break;
	case SYS_yield: yield(); c->GPRx = 0; break;
	case SYS_open: c->GPRx = fs_open((const char *)a[1], a[2], a[3]); break;
	case SYS_read: c->GPRx = fs_read(a[1], (void *)a[2], a[3]); break;
	case SYS_write: c->GPRx = fs_write(a[1], (void *)a[2], a[3]); break;
	case SYS_close: c->GPRx = fs_close(a[1]); break;
	case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
	case SYS_brk: c->GPRx = 0; break;
	case SYS_gettimeofday: {
		if ((void *)a[1] != NULL) {
			//size_t now = io_read(AM_TIMER_UPTIME).us;
			//printf("%u\n", now);
			*(size_t *)(a[1]) = io_read(AM_TIMER_UPTIME).us / 1000000u;
			*((size_t *)(a[1]) + 1) = io_read(AM_TIMER_UPTIME).us;
			printf("%u %u\n", *(size_t *)a[1], *((size_t *)(a[1]) + 1));
		}
		c->GPRx = 0;
		break;
	}
		
    default: panic("Unhandled syscall ID = %u", a[0]);
  }

}
