/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    //unsigned char pic1_data, pic2_data //Save masks
    //pic1_data = inb(MASTER_8259_PORT+1);
    //pic2_data = inb(SLAVE_8259_PORT+1);

    //INIT PICS
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    //SET OFFSETS for each PIC's vectore ranges 
    outb(ICW2_MASTER, MASTER_8259_PORT+1);
    outb(ICW2_SLAVE, SLAVE_8259_PORT+1);
  
    //INDICATES SLAVE PIC AT IRQ2 - tells SLAVE PIC cascade id
    outb(ICW3_MASTER, MASTER_8259_PORT+1);
    outb(ICW3_SLAVE, SLAVE_8259_PORT+1);

    /*SELECT 8086 mode*/
    outb(ICW4, MASTER_8259_PORT+1);
    outb(ICW4, SLAVE_8259_PORT+1);
    
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) { //Adapted from OSDev
  uint16_t port;
  uint8_t value;
  if (irq_num < 8) {
    port = MASTER_8259_PORT+1; //Get data for primary
  }
  else {
    port = SLAVE_8259_PORT+1; //Get data for secondary
    irq_num -= 8;
  }
  value = inb(port) | (1<<irq_num);
  outb(value,port);
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) { //Adapted from OSDEV
  uint16_t port;
  uint8_t value;
  if (irq_num < 8) {
    port = MASTER_8259_PORT + 1; //Access data for primary
  }
  else {
    port = SLAVE_8259_PORT + 1; //Access data for secondary
    irq_num -= 8;
  }
  value = inb(port) & ~(1 << irq_num); 
  outb(value,port);
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) { //Adapted from OSDEV for 8259 PIC https://wiki.osdev.org/PIC
    if (irq_num >= 16) {
      return;
    }
    if (irq_num >= 8) {
		  outb(EOI | irq_num - 8,SLAVE_8259_PORT);
      outb(EOI | 2, MASTER_8259_PORT);
      return;
    }
	outb(EOI | irq_num, MASTER_8259_PORT);
}
