#ifndef IDT_H
#define IDT_H

#include "lib.h"
#include "i8259.h"

#define NUM_EXCEPTIONS 21
#define SYSCALL_VEC 0x80
#define RTC_VEC 40
#define KEYB_IRQ_NO 0x21
#define RTC_IRQ_NO 0x28
#define PIT_IRQ_NO 0x20


// typedef enum idt_entry_t {
//     DE, //divide by 0
//     DB, //RESERVED fault
//     NMI, //non maskable interrupt
//     BP, //breakpoint exception
//     OF, //overflow trap
//     BR, //BOUND range exceeded
//     UD, //undefinde opcode
//     NM, //No math coprocessor/device unavail
//     DF, //double fault
//     SO, //Segment Overrun
//     TS, //Invalid TSS Fault
//     NP, //Segment not present
//     SS, //Stack segment fault
//     GP, //general protection fault
//     PF, //page fault
//     E15, //reserved by intel
//     MF, //x87 floating point exception
//     AC, //alignment check exception
//     MC, //machine check exception
//     XF, //SIMD floating point exception
//     R //20 onwards reserved
// } idt_entry_t;

void init(void);

#endif /* IDT_H */
