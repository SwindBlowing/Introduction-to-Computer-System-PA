#include <common.h>
#include <fs.h>
#include "syscall.h"
#include <proc.h>

//#define CONFIG_STRACE

#ifdef CONFIG_STRACE
static char nowFile[999] = {'\0'};
#endif

void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);
void switch_boot_pcb();
extern PCB *current;
int mm_brk(uintptr_t brk);

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
	case SYS_exit: 
		halt(a[1]);
		if (a[1]) halt(a[1]);
		//naive_uload(NULL, "/bin/nterm"); break;
		context_uload(current, "/bin/nterm", NULL, NULL);
		switch_boot_pcb();
		yield();
		break;
	case SYS_yield: yield(); c->GPRx = 0; break;
	case SYS_open: c->GPRx = fs_open((const char *)a[1], a[2], a[3]); break;
	case SYS_read: c->GPRx = fs_read(a[1], (void *)a[2], a[3]); break;
	case SYS_write: c->GPRx = fs_write(a[1], (void *)a[2], a[3]); break;
	case SYS_close: c->GPRx = fs_close(a[1]); break;
	case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]); break;
	case SYS_brk: c->GPRx = mm_brk(a[1]); break;
	case SYS_execve: {
		//naive_uload(NULL, (const char *)a[1]); break;
		int fd = fs_open((const char *)a[1], 0, 0);
		if (fd == -2) {
			c->GPRx = -2;
			break;
		}
		fs_close(fd);
		context_uload(current, (const char *)a[1], (char *const*)a[2], (char *const*)a[3]);
		switch_boot_pcb();
		yield();
		break;
	}
	case SYS_gettimeofday: 
		if ((void *)a[1] != NULL) {
			*(size_t *)(a[1]) = io_read(AM_TIMER_UPTIME).us / 1000000u;
			*((size_t *)(a[1]) + 1) = io_read(AM_TIMER_UPTIME).us;
			//printf("%u %u\n", *(size_t *)a[1], *((size_t *)(a[1]) + 1));
		}
		c->GPRx = 0;
		break;
		
    default: panic("Unhandled syscall ID = %u", a[0]);
  }

}
