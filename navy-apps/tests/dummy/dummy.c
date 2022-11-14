#include <stdint.h>
#include <fixedptc.h>
#include <stdio.h>

#ifdef __ISA_NATIVE__
#error can not support ISA=native
#endif

#define SYS_yield 1
extern int _syscall_(int, uintptr_t, uintptr_t, uintptr_t);

int main() {
  //return _syscall_(SYS_yield, 0, 0, 0);
  fixedpt a = fixedpt_rconst(0);
  printf("%x", fixedpt_ceil(a));
  return 0;
}
