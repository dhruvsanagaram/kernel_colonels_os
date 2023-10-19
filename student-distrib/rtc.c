
#include "rtc.h"
#include "i8259.h"
#include "lib.h"

#define RTC_PORT_ADDR 0x70
#define RTC_PORT_DATA 0x71
#define REG_A 0x0A
#define REG_B 0x0B
#define REG_C 0x0C
#define SET_BIT_6 0x40
#define RTC_IRQ 8


#define SUCCESS 0
#define FAILURE 1

#define MAX_FQ 1024
#define MIN_FQ 2

volatile uint32_t rtcInterrupt = 0;

/* void rtc_init();
 * Inputs: N/A
 * Return Value: uint32_t
 *  Function: Disables NMI & reads in the state of RegB (see osDev) to enable RTC-backed interrupts/syscalls */
int32_t rtc_init(void) {
  // taken from osdev: https://wiki.osdev.org/RTC
  outb(REG_B, RTC_PORT_ADDR); //disable NMI and select regB
  // read the current value of regB
  unsigned char old = inb(RTC_PORT_DATA);
  outb(REG_B, RTC_PORT_ADDR); //set the index again
  outb(old | SET_BIT_6, RTC_PORT_DATA); //enable RTC ints

  //set the frequency to 2Hz

  

  //reenable IRQ
  enable_irq(RTC_IRQ);

  
  
  return SUCCESS; //SUCCESS
}

/* void rtc_handle();
 * Inputs: N/A
 * Return Value: void
 *  Function: Discards old RTC value and sends EOI */
void rtc_handle(void){
    outb(0x0C, RTC_PORT_ADDR);
    inb(RTC_PORT_DATA);
    /// vv triggers whenever RTC sends interrupt
    //printf("ADFASGJAJSAJGJ\n");
    /// call test_interrupts for handling
    send_eoi(RTC_IRQ);
}

/* void rtc_open();
 * Inputs: N/A
 * Return Value: void
 *  Function: Open RTC */

 int32_t rtc_open(const uint8_t *filename) {
  
   return SUCCESS;
 }


 /* void rtc_close();
 * Inputs: fd -- the file descriptor
 * Return Value: void
 *  Function: Close RTC */

 int32_t rtc_close(int32_t fd) {

   return SUCCESS;
 }

 /* void rtc_read();
 * Inputs: N/A
 * Return Value: void
 *  Function: Read RTC */

int32_t rtc_read(int32_t fd, void* buf, uint32_t nbytes){
  rtcInterrupt = 0;
  while(rtcInterrupt == 0);
  return SUCCESS;
}

 /* void rtc_write();
 * Inputs: N/A
 * Return Value: void
 *  Function: Write RTC */

 /*The write system call writes data to the terminal or to a device (RTC). In the case of the terminal, all data should
be displayed to the screen immediately. In the case of the RTC, the system call should always accept only a 4-byte
integer specifying the interrupt rate in Hz, and should set the rate of periodic interrupts accordingly. Writes to regular
files should always return -1 to indicate failure since the file system is read-only. The call returns the number of bytes
written, or -1 on failure.
*/

int32_t rtc_write(int32_t fd, const void* buf, uint32_t nbytes){
  uint32_t fq;
  int PowerOfTwo;
  fq = (int) *buf; //SOMEONE FIX THIS PLZ

  //Checks:
  //Frequency OOB - fail
  //Check if the frequency is a power of 2
  

  //Start checks: check the bounds of the frequency 
  if (fq > MAX_FQ || fq < MIN_FQ) {
    return -FAILURE;
  }
  PowerOfTwo = !(fq & (fq - 1));
  if (!PowerOfTwo){
    return -FAILURE;
  }
  
  
  
  return SUCCESS;
}
