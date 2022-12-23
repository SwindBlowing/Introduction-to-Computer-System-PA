#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
Context *kcontext(Area kstack, void (*entry)(void *), void *arg);

void context_kload(PCB *pcb, void (*entry)(void *), void * arg)
{
	Area kstack;
	kstack.start = pcb->stack;
	kstack.end = (pcb->stack) + sizeof(pcb->stack);
	pcb->cp = kcontext(kstack, entry, arg);
}

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {

  context_kload(&pcb[0], hello_fun, NULL);

  switch_boot_pcb();

  Log("Initializing processes...");

  // load program here
  naive_uload(NULL, "/bin/nterm");

}

Context* schedule(Context *prev) {
  // save the context pointer
  current->cp = prev;

   // always select pcb[0] as the new process
  current = &pcb[0];

  // then return the new context
  return current->cp;
}
