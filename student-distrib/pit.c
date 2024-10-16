#include "pit.h"
#include "lib.h"
#include "i8259.h"
#include "page.h"
#include "x86_desc.h"
#include "terminal.h"
#include "process.h"

#define SUCCESS 0

/* http://www.osdever.net/bkerndev/Docs/pit.htm */


/**
* pit_init
* inputs: int hz
* output: int32_t
* side effects: pit initializer
*/
int32_t pit_init(int hz){ //Taken from OSDever for PIT
    int divisor = 1193181/hz;
    outb(0x37,0x43);             /* Set our command byte 0x36 */
    outb(divisor & 0xFF,0x40);   /* Set low byte of divisor */
    outb(divisor >> 8,0x40);     /* Set high byte of divisor */

    // enable_irq(0);
    return SUCCESS;
}


/*
* 
*/


/**
* pit_handler
* inputs: none
* output: int32_t
* side effects: interrupt handler for pit schedule
*/
int32_t pit_handler() {
    cli();
    send_eoi(0);
    //send eoi to the irq line for the PIT (0)
    //Step 2: Scheduler
    //update the video mem paging for the next terminal TID
    //in the round robin schedule cycle
    update_video_memory_paging((view_term->tid+1) % 3);
    next_process();
    sti();
    return SUCCESS;
}
