#pragma once
#include "types.h"
#include "paging.h"
#include "param.h"
#include "idt.h"
#include ""

typedef struct {
    u64 rdi;
    u64 rsi;
    u64 rbx;
    u64 rbp;
    u64 rip;
} context;

// Per-CPU state
// At any point in time, a processor will be running one of
// - its own initialization routine (only once the kernel is setting up)
// - a user process (or any interrupts or system calls that come up)
// - a scheduler routine to run the next process
// Thus see 'started', 'proc' properties
// The scheduler isn't itself a process - it uses the 'kpgdir' page directory and has its own context - we
// store the context in 'scheduler' property
struct cpu {
    u8 apicid;                // Local APIC ID (local interrupt controller)
    context *scheduler;   // kernel context at the top of scheduler stack
    //struct taskstate ts;         // Used by x86 to find stack for interrupt (TSS)
    //struct segdesc gdt[NSEGS];   // x86 global descriptor table
    volatile boolean started;       // Has the CPU started?
    int ncli;                    // Depth of pushcli nesting.
    int saved_IF;                  // Were interrupts enabled before pushcli?
    struct proc *proc;           // The process running on this cpu or null
};

// UNUSED - process doesn't exist
// EMBRYO - undergoing setup before becoming RUNNABLE
// SLEEPING - blocked waiting for something (e.g. I/O)
// RUNNABLE - ready to be scheduled
// RUNNING - currently running on a CPU
// ZOMBIE - killing a process requires cleanup before it goes back to UNUSED
enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Per-process state
typedef struct proc {
    size_t sz;                   // size of process memory
    pt_t* pml4;                  // page table
    u8 *kstack;                  // bottom of kernel stack for this process
    enum procstate state;
    pid_t pid;
    struct proc *parent;
    trapframe *tf;               // state of the CPU when interrupted, needed to resume process
    context *context;     // process context at the top of its stack
    void *chan;                  // if non-zero, sleeping on chan
    int killed;                  // have been killed/should be killed soon
    //struct file *ofile[PROCNFD]; // open files
    File *cwd;
    char name[16];               // debugging
} proc_t;

struct proc* myproc(void);
struct cpu* mycpu(void);
void yield(void);
