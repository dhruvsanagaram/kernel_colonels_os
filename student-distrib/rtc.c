
#include "rtc.h"
#include "i8259.h"

#define RTC_PORT_ADDR 0x70
#define RTC_PORT_DATA 0x71
#define REG_A 0x0A
#define REG_B 0x0B
#define REG_C 0x0C
#define SET_BIT_6 0x40
#define RTC_IRQ 8

uint32_t rtc_init(void) {
  // taken from osdev: https://wiki.osdev.org/RTC
  outb(REG_B, RTC_PORT_ADDR); //disable NMI and select regB
  // read the current value of regB
  unsigned char prev = inb(RTC_PORT_DATA);
  outb(REG_B, RTC_PORT_ADDR); //set the index again
  outb(prev | SET_BIT_6, RTC_PORT_DATA); //enable RTC ints

  //reenable IRQ
  enable_irq(RTC_IRQ);
  
  return 0; //SUCCESS
}

void rtc_handle(void){
    outb(0x0C, RTC_PORT_ADDR);
    inb(RTC_PORT_DATA);

    ///
    
    send_eoi(RTC_IRQ);
}