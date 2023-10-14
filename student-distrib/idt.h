#ifndef IDT_H
#define IDT_H

#ifndef ASM

#include <lib.h>

#define NUM_EXCEPTIONS 21
#define SYSCALL_VEC 0x80
#define RTC_VEC 40
#define KEYB_VEC 33
typedef enum idt_entry_t {
  DE; //divide by 0
  DB; //RESERVED fault
  NMI; //non maskable interrupt
  BP; //breakpoint exception
  OF; //overflow trap
  BR; //BOUND range exceeded
  UD; //undefinde opcode
  NM; //No math coprocessor/device unavail
  DF; //double fault
  SO; //Segment Overrun
  TS; //Invalid TSS Fault
  NP; //Segment not present
  SS; //Stack segment fault
  GP; //general protection fault
  PF; //page fault
  E15; //reserved by intel
  MF; //x87 floating point exception
  AC; //alignment check exception
  MC; //machine check exception
  XF; //SIMD floating point exception
  R; //20 onwards reserved
} idt_entry_t;

// struct reg_t {
//   uint32_t edi;
//   uint32_t esi;
//   uint32_t eax;  
//   uint32_t ebx;
//   uint32_t ecx;
//   uint32_t edx;
//   uint32_t ebp;
//   uint32_t esp;
// } __attribute__(( packed ));

// typedef struct reg_t reg_t;


void init(void);
void handle_exception(uint32_t id, reg_t regs, uint32_t eflags, uint32_t err);
void general_interrupt(void);


#endif /* ASM */

#endif /* IDT_H */