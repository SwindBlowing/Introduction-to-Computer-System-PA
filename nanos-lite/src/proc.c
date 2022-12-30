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
  
  static char proc_temp[99] = "The second Hello-World";
  context_kload(&pcb[0], hello_fun, (void *)proc_temp);
  //context_uload(&pcb[0], "/bin/dummy");
  static char *proc_empty[] = {NULL};
  /*static char *pal_argv[3] = {"/bin/pal", "--skip", NULL};
  context_uload(&pcb[1], "/bin/pal", pal_argv, proc_empty);*/
  static char *proc_argvs[3] = {"/bin/hello", NULL};
  context_uload(&pcb[1], "/bin/hello", proc_argvs, proc_empty);

  switch_boot_pcb();

  // load program here
  //naive_uload(NULL, "/bin/nterm");

}

//const int rates = 10;
//int nowTimes = 0;

Context* schedule(Context *prev) {
  
  /*if (current == NULL || current == &pcb[0]) {
	// save the context pointer
  	current->cp = prev;
	current = ((current == &pcb[0]) ? &pcb[1] : &pcb[0]);
  }
  else {
	nowTimes++;
	if (nowTimes == rates) {
		// save the context pointer
  		current->cp = prev;
		current = ((current == &pcb[0]) ? &pcb[1] : &pcb[0]);
		nowTimes = 0;
	}
	else {
		current->cp = prev;
		current = &pcb[1];
	}
  }
  // then return the new context
  return current->cp;*/
  printf("pcb0 & 1: %p %p\n", &pcb[0], &pcb[1]);
  printf("current cp & current: %p %p\n", current->cp, current);
  current->cp = prev;
  current = ((current == &pcb[0]) ? &pcb[1] : &pcb[0]);
  return current->cp;
}
