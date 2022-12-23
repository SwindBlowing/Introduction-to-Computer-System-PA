#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  //printf("%p\n", args);
  int argc = *args;
  //printf("%d\n", argc);
  char **argv; argv = (char **)(args + 1);
  char **envp; envp = (char **)(args + argc + 2);
  //environ = empty;
  environ = envp;
  exit(main(argc, argv, envp));
  assert(0);
}
