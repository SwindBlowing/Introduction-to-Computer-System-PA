#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  //printf("%p\n", args);
  int argc = *args;
  printf("%d\n", argc);
  char **argv; argv = (char **)(*args + 1);
  char *envp[99];
  char *empty[] =  {NULL };
  //environ = empty;
  environ = empty;
  exit(main(argc, argv, empty));
  assert(0);
}
