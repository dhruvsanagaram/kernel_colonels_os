#include "pit.h"

/* http://www.osdever.net/bkerndev/Docs/pit.htm */
int32_t pit_init(int hz){ //Taken from OSDever for PIT
    int divisor = 1193180 / hz;       /* Calculate our divisor */
    outb(0x43, 0x36);             /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
    outb(0x40, divisor >> 8);     /* Set high byte of divisor */

    enable_irq(0);
    return SUCCESS;
}


/*
* 
*/

int32_t pit_handler() {
    cli();
    //send eoi to the irq line for the PIT (0)
    //Step 2: Scheduler
    update_video_memory_paging(target_terminals->tid);
    next_process();
    sti();
    return SUCCESS;
}
