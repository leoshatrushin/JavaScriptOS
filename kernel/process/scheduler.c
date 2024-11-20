#include "proc.h"
#include "spinlock.h"

extern void context_switch(context **, context *);

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU's setup ends with calling scheduler(), which never returns and loops, doing:
//  - choose a RUNNABLE process from the process table to run
//  - swtch to that process to resume it
//  - eventually the process swtch-es back to the scheduler.
// Interrupts are enabled for the first time
void scheduler(void) {
    struct proc *p;
    struct cpu *c = mycpu(); // ok to call because interrupts are disabled
    c->proc = 0; // a CPU running the scheduler isn't running a process

    for(;;){
        // Enable interrupts on this processor. Chance to handle outstanding interrupts (e.g. disk interrupt to
        // unblock SLEEPING processes) while lock is released (to prevent deadlocks if an interrupt handler
        // needs to acquire the lock)
        sti();

        // Loop over process table looking for a RUNNABLE process to run.
        acquire(&ptable.lock); // acquiring a lock disables interrupts

        // scheduling algorithm
        for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
            if(p->state != RUNNABLE)
                continue;

            // Switch to chosen process.  It is the process's job
            // to release ptable.lock and then reacquire it
            // before jumping back to us.
            // switch to the process pgdir
            // kernel code continues to be safe to execute because it uses addresses in the higher half, which are
            // the same for every page directory (setupkvm())
            c->proc = p;
            switchuvm(p);
            p->state = RUNNING;

            // pick up where process left off - in kernel mode, which handled a syscall, interrupt or exception
            // before calling the scheduler
            // process will still be holding the ptable.lock, this is the main reason for the existence of forkret()
            // DANGER - if you want to add a new syscall that will let go of the CPU, it must release the process
            // table lock at the point at which it starts executing after switching to it from the scheduler
            // Can't release before calling swtch() and reacquire after - think of locks as protecting some
            // invariant, which may be violated temporarily while you hold the lock - the process table protects
            // invariants related to the process's p->state and p->context fields
            // - CPU registers must hold process's register values
            // - RUNNABLE process must be able to run by any idle CPU's scheduler
            // - etc.
            // These don't hold true while executing in swtch() - problem if another CPU decides to run the
            // process before swtch() is done executing
            //
            // At some point, the process will be done running and will give up the CPU again
            // Before it switches back into the scheduler, it has to acquire the process table lock again
            // DANGER - make sure to acquire the process table lock if you add your own scheduling-related syscall
            swtch(&(c->scheduler), p->context);

            // Eventually process will swtch back
            switchkvm(); // switch back to kpgdir

            // Process is done running for now.
            // It should have changed its p->state before coming back.
            c->proc = 0;
        }
        release(&ptable.lock);

    }
}

// We say code that runs after switching away from the scheduler, this runs after switching to the scheduler
// Functions can't just call scheduler(), it probably left off last time halfway through the loop and
// should resume in the same place
// Should be called after acquiring the process table lock and without holding any other locks (lest we cause
// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void) {
    struct proc *p = myproc();

    /*if(!holding(&ptable.lock)) // should be holding process table lock*/
    /*    panic("sched ptable.lock");*/
    /*if(mycpu()->ncli != 1) // should not be holding any other locks (lest we cause a deadlock)*/
    /*    panic("sched locks");*/
    /*if(p->state == RUNNING) // should not be RUNNING since we're about to stop running it*/
    /*    panic("sched running");*/
    /*if(readeflags()&FL_IF)*/
    /*    panic("sched interruptible");*/

    // pushcli() and popcli() check whether interrupts were enabled before turning them off while holding
    // a lock, but this is really a property of this kernel thread, not of this CPU, so we need to save that
    boolean intena = mycpu()->saved_IF;
    // pick up where scheduler left off
    context_switch(&p->context, mycpu()->scheduler);
    // this process will resume executing eventually, at which point we'll restore the data about whether
    // interrupts were enabled and let it run again
    mycpu()->saved_IF = intena;
}

void yield(void) {
    //acquire(&ptable.lock);  //DOC: yieldlock
    myproc()->state = RUNNABLE; // can be picked up in next scheduling round
    sched(); // switch into scheduler
    //release(&ptable.lock); // release lock when we eventually return here
}
