# Digression on interrupts and exceptions
# interrupt - asynchronous, exception - synchronous
# Each interrupt and exception in the IDT is identified with a number called a vector
# some vectors from 0-31 are predefined
# vectors 32-255 are for software-defined interrupts, which are for either software interrupts or
# maskable hardware interrupts
# the processor also defines several additional interrupts that do not point to entries in the IDT
# e.g. SMI interrupt
# when the processor detects an interrupt or exception, it either executes an implicit call to a handler
# procedure or handler task

#include "segmentation.h"

# vectors.S sends all traps here
# part of the trap frame (iret frame) is already pushed on the stack by the hardware
.globl alltraps
alltraps:
  # complete trap frame
  save_context
  
  # Set up data segments, i.e. set up %ds and %es for the kernel (%cs and %ss already done by processor)
  movw $SEG_KDATA, %ax
  movw %ax, %ds
  movw %ax, %es

  # call trap() with trap frame on the stack as argument
  movq %rsp, %rdi
  cld
  call trap

  # Return falls through to trapret...
  # new process executes forkret() and any call to trap() return here
  # restores process state
.global trapret
trapret:
  movq %rax, %rsp # use the returned context
  restore_context
  addl $16, %esp  # trapno and errcode
  iretq # interrupt return - restore 5 hardware-pushed items from stack, including user privilege CS
        # important for atomicity - so interrupts don't occur while changing privilege levels

.macro save_context
    pushw %ds
    pushw %es
    pushw %fs
    pushw %gs
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %rsi
    pushq %rdi
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
.endm

.macro restore_context
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rdi
    popq %rsi
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popw %gs
    popw %fs
    popw %es
    popw %ds
.endm
