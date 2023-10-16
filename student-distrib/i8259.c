/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = IRQ_MASK; /* IRQs 0-7  */
uint8_t slave_mask = IRQ_MASK;  /* IRQs 8-15 */

/* void i8259_init();
 * Inputs: N/A
 * Return Value: void
 *  Function: INIT PICs, enable secondary PIC */
void i8259_init(void) {

    //INIT PICS
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW1, SLAVE_8259_PORT);

    //SET OFFSETS for each PIC's vector ranges 
    outb(ICW2_MASTER, MASTER_8259_PORT+1);
    outb(ICW2_SLAVE, SLAVE_8259_PORT+1);
  
    //INDICATES SLAVE PIC AT IRQ2 - tells SLAVE PIC cascade id
    outb(ICW3_MASTER, MASTER_8259_PORT+1);
    outb(ICW3_SLAVE, SLAVE_8259_PORT+1);

    //SELECT 8086 mode
    outb(ICW4, MASTER_8259_PORT+1);
    outb(ICW4, SLAVE_8259_PORT+1);

    //enable IRqs on the slave pic -- 2 corresponds to the IRQ No for slave PIC
    enable_irq(2);
    
}

/* void enable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num -- represents the IRQ line
 * Return Value: void
 *  Function: Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) { //Adapted from OSDEV

  uint8_t mask;
  uint16_t port;

  //Check if irq_num is out of range: anything >= 8*2 (number of lines) is bad
  if (irq_num > 15){
    return;
  }

  //if irq_num < 8, this falls into the range for master PIC: services IRQs from 0 to 7
  if (irq_num < 8){
    master_mask &= ~(1 << irq_num);     //unmask the corresponding irq line to prepare servicing this interrupt
                                        //This will then be able to service the IRQ on the master PIC
                                        //Set the according line to active low (service begins on active high on all ints)
    mask = master_mask;
    port = MASTER_8259_PORT+1;          //write this info into the data port to enable the correct IRQ line
  }
  //Other IRQs in this range get serviced by the slave PIC
  else {
    slave_mask &= ~(1 << (irq_num - 8)); //gets the IRq on the slave mask
    mask = slave_mask;
    port = SLAVE_8259_PORT+1;
  }
  outb(mask, port);
  return;
}


/* void disable_irq(uint32_t irq_num);
 * Inputs: uint32_t irq_num -- represents the IRQ line
 * Return Value: void
 *  Function: Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
  uint8_t mask;
  uint16_t port;
  //Check if irq_num is out of range: anything >= 8*2 (number of lines) is bad
  if(irq_num > 15){
    return;
  }
  //if irq_num < 8, this falls into the range for master PIC: services IRQs from 0 to 7
  if(irq_num < 8){
    master_mask |= (1 << irq_num); //The active line is set back to active high. This essentially means the line is turned off
    mask = master_mask;
    port = MASTER_8259_PORT+1;    //Update the data on the port
  }
  //Other IRQs in this range get serviced by the slave PIC
  else {
    slave_mask |= (1 << (irq_num - 8));   //The corresponding interrupt on the slave PIC is set to active high
    mask = slave_mask;
    port = SLAVE_8259_PORT+1;            //Update port data
  }
  outb(mask, port);
  return;
}


/* void send_eoi(uint32_t irq_num);
 * Inputs: uint32_t irq_num -- represents the IRQ line
 * Return Value: void
 *  Function: Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) { //Adapted from OSDEV for 8259 PIC https://wiki.osdev.org/PIC

    //irq number OOB (2*NUM_OF_PORTS)
    if (irq_num >= 16) {
      return; 
    }

    //irq number is serviced on slave PIC
    if (irq_num >= 8) {
      outb(EOI | 2, MASTER_8259_PORT); //This handles data on the slave port. The slave pic is identified with number 2, and address is set
		  outb(EOI | (irq_num - 8), SLAVE_8259_PORT);  //Handles data of the IRQ end for the address port. 
      return;
    }
    //master PIC IRQs
	  outb(EOI | irq_num, MASTER_8259_PORT); //Take care of master interrupts
}
