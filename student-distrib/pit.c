#include "pit.h"
#include "lib.h"
#include "i8259.h"
#include "page.h"
#include "x86_desc.h"
#include "terminal.h"

#define SUCCESS 0

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
    //update the video mem paging for the next terminal TID
    //in the round robin schedule cycle
    update_video_memory_paging((schedule_term->tid+1) % 3);
    next_process();
    sti();
    return SUCCESS;
}
