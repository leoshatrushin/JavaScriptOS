# Initial process execs /init.
# This code runs in user space.

#include "syscall.h"
#include "traps.h"

# exec(init, argv)
.global start
start:
  movq $init, %rdi
  movq $argv, %rsi
  //pushl $0  // where caller pc would be
  movl $SYS_exec, %eax
  int $SYSCALL_INTERRUPT_NUM

# for(;;) exit();
exit:
  movl $SYS_exit, %eax
  int $SYSCALL_INTERRUPT_NUM
  jmp exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
.balign 4
argv:
  .quad init
  .quad 0
