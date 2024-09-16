#include "spinlock.h"
#include "proc.h"
#include "pmm.h"
#include "console.h"
#include "vmm.h"

// global process table
struct {
  spinlock lock;
  proc_t procs[MAX_PROC];
} ptable;

void procstart(void);
extern void trapret(void);

int nextpid = 1;

proc_t* create_proc(void) {
    //acquire(&ptable.lock);

    // find UNUSED slot in process table
    proc_t* proc;
    for (proc = ptable.procs; proc < &ptable.procs[MAX_PROC]; ++proc)
        if(proc->state == UNUSED)
            break;

    // no slot found
    if (proc->state != UNUSED) {
        //release(&ptable.lock);
        return 0;
    }

    proc->state = EMBRYO;
    proc->pid = nextpid++;

    //release(&ptable.lock);

    // allocate a page for the process's kernel thread to use as a stack
    if((proc->kstack = pmm_alloc()) == 0){
        proc->state = UNUSED;
        return 0;
    }

    // We won't set up pgdir yet - that'll happen in fork()
    // When the scheduler first runs the new process, it'll check its context via p->context to get its
    // register contents, including %eip
    // stack top
    u8* kstack = proc->kstack + KSTACKSIZE;

    // allocate space for the trapframe
    kstack -= sizeof *proc->tf;
    proc->tf = (trapframe*)kstack;

    // push return address trapret()
    kstack -= sizeof(void*);
    *kstack = (uintptr_t)trapret;

    // allocate context
    kstack -= sizeof *proc->context;
    proc->context = (context*)kstack;
    __builtin_memset(proc->context, 0, sizeof *proc->context);
    proc->context->rip = (uintptr_t)procstart;

    return proc;
}

// A process swtch()es here on first schedule
void procstart(void) {
    // still holding ptable.lock from scheduler.
    // any other kernel code that switches into the scheduler (e.g. sleep(), yield()) will have a similar lock
    // release right after returning from the scheduler
    //release(&ptable.lock);

    // return to trapret then iret to user space
}

void panic(char *s) {
    bochs_debug_output(s);
    for(;;);
}

// Load the initcode into address 0 of pgdir.
// sz must be less than a page.
// Similar to loaduvm(), but instead of loading program code from disk, it copies it from memory
// Put 'sz' bytes from 'init' in address 0 of process's 'pgdir'
// Simple because we only call it for programs less than 1 page in size, so there's no looping over pages
void setup_initproc_uvm(pt_t *pml4, char *init, size_t sz) {
    if (sz >= PGSIZE)
        panic("inituvm: more than a page");
    void* mem = pmm_alloc();
    __builtin_memset(mem, 0, PGSIZE); // clear garbage from kfree()
    mappages(pml4, 0, PGSIZE, KV2P(mem), 1, 1); // put in pgdir at address 0
    __builtin_memmove(mem, init, sz); // copy code from init into new page
}

// set up first user process 'init'
void userinit(void) {
  proc_t *initproc = create_proc();
  // see https://pdos.csail.mit.edu/6.828/2008/
  extern char _binary_initcode_start[], _binary_initcode_size[];
  
  if ((initproc->pml4 = setupkvm()) == 0)
      panic("userinit: out of memory?");
  setup_initproc_uvm(p->pml4, _binary_initcode_start, (int)_binary_initcode_size);
  initproc->sz = PGSIZE;
  __builtin_memset(initproc->tf, 0, sizeof(*initproc->tf));
  initproc->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  initproc->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  initproc->tf->es = initproc->tf->ds;
  initproc->tf->ss = initproc->tf->ds;
  initproc->tf->eflags = FL_IF;
  initproc->tf->esp = PGSIZE;
  initproc->tf->eip = 0;  // beginning of initcode.S

  __builtin_strncpy(initproc->name, "init", sizeof(initproc->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  //acquire(&ptable.lock);

  p->state = RUNNABLE;

  //release(&ptable.lock);
}
