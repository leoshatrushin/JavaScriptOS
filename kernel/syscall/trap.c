#include "idt.h"
#include "proc.h"
#include "pit.h"
#include "lapic.h"

trapframe *interrupt_dispatch(trapframe *tf) {
    switch(tf->interrupt_num){
        case PAGE_FAULT:
            //page_fault_handler(status->error_code);
            break;
        case GENERAL_PROTECTION:
            asm("hlt");
            break;
        case APIC_TIMER_INTERRUPT_NUM:
            //timer_handler();
            //tf = schedule(tf);
            //kernel_settings.kernel_uptime++;
            lapic_eoi();
            proc_t *p = myproc();
            if (p && p->state == RUNNING)
                yield();
            break;
        case APIC_SPURIOUS_INTERRUPT_NUM:
            //pretty_log(Verbose, "Spurious interrupt received");
            //should i send an eoi on a spurious interrupt?
            lapic_eoi();
            break;
        case KEYBOARD_INTERRUPT_NUM:
            //keyboard_interrupt_handler();
            lapic_eoi();
            break;
        case PIT_INTERRUPT_NUM:
            pit_irq_handler();
            lapic_eoi();
            break;
        case SYSCALL_INTERRUPT_NUM:
            //pretty_log(Verbose, "Serving syscall.");
            //syscall_dispatch(status);
            break;
        default:
            asm("hlt");
            break;
    }

    // return to alltraps and iret to user space process
    return tf;
}
