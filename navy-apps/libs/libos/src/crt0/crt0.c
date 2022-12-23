#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  char *empty[] = {NULL};
  //printf("%p\n", args);
  int argc = *args;
  printf("%d\n", argc);

  /*char *argv[argc];
  for (int i = 0; i < argc; i++) 
	strcpy(argv[i], (char *)(args + i + 1));*/

  /*int sz_envp = 0;
  while ((char *)(args + argc + 2 + sz_envp) != NULL) sz_envp++;
  char *envp[sz_envp];
  for (int i = 0; i < sz_envp; i++)
  	strcpy(envp[i], (char *)(args + argc + 2 + i));*/
  
  environ = empty;
  exit(main(0, empty, empty));
  assert(0);
}
