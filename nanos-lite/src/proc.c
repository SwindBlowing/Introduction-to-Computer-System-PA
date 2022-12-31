#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);

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
  
  //static char proc_temp[99] = "The second Hello-World";
  static char *proc_empty[] = {NULL};

  static char *proc_argvs_0[2] = {"/bin/hello", NULL};
  context_uload(&pcb[0], "/bin/hello", proc_argvs_0, proc_empty);
  static char *proc_argvs_1[2] = {"/bin/nterm", NULL};
  context_uload(&pcb[1], "/bin/nterm", proc_argvs_1, proc_empty);
  static char *proc_argvs_2[2] = {"/bin/bird", NULL};
  context_uload(&pcb[2], "/bin/bird", proc_argvs_2, proc_empty);
  static char *proc_argvs_3[3] = {"/bin/pal", "--skip", NULL};
  context_uload(&pcb[3], "/bin/pal", proc_argvs_3, proc_empty);

  switch_boot_pcb();

  // load program here
  //naive_uload(NULL, "/bin/nterm");

}

static const int rates = 1000;
static int nowTimes = 0;
int fg_pcb = 1;

Context* schedule(Context *prev) {
  
  if (current == NULL || current == &pcb[0]) {
	// save the context pointer
  	current->cp = prev;
	current = ((current == &pcb[0]) ? &pcb[fg_pcb] : &pcb[0]);
  }
  else {
	nowTimes++;
	if (nowTimes == rates) {
		// save the context pointer
  		current->cp = prev;
		current = ((current == &pcb[0]) ? &pcb[fg_pcb] : &pcb[0]);
		nowTimes = 0;
	}
	else {
		current->cp = prev;
		current = &pcb[fg_pcb];
	}
  }
  // then return the new context
  return current->cp;
  //printf("pcb0 & 1: %p %p\n", &pcb[0], &pcb[1]);
  /*current->cp = prev;
  current = ((current == &pcb[0]) ? &pcb[1] : &pcb[0]);
  //printf("current cp & current: %p %p\n", current->cp, current);
  return current->cp;*/
}
