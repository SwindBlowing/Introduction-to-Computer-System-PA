#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
void context_uload(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
	j++;
	yield();
	if (j % 10000) continue;
	if (arg != NULL)
    	Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (char *)arg, j);
	else 
		Log("Hello World from Nanos-lite with no arg for the %dth time!", j);
  }
}

void init_proc() {

  Log("Initializing processes...");
  
  static char temp[99] = "The second Hello-World";
  context_kload(&pcb[0], hello_fun, (void *)temp);
  //context_uload(&pcb[0], "/bin/dummy");

  context_uload(&pcb[1], "/bin/pal");

  switch_boot_pcb();

  // load program here
  //naive_uload(NULL, "/bin/nterm");

}

Context* schedule(Context *prev) {
  // save the context pointer
  current->cp = prev;

   // (before) always select pcb[0] as the new process
  //current = &pcb[0];
  // (current) transform between pcb[0] and pcb[1]
  current = ((current == &pcb[0]) ? &pcb[1] : &pcb[0]);

  // then return the new context
  return current->cp;
}
