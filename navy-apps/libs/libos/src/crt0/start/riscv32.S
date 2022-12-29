.globl  _start
_start:
  move s0, zero
  move sp, a0
  move s0, a0
  jal call_main
