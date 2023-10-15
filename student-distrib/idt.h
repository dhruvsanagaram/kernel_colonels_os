#ifndef IDT_H
#define IDT_H

#ifndef ASM

#include <lib.h>
#include "i8259.h"

#define NUM_EXCEPTIONS 21
#define SYSCALL_VEC 0x80
#define RTC_VEC 40
#define KEYB_IRQ_NO 0x21
#define RTC_IRQ_NO 0x28

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
// void handle_exception(uint32_t id, reg_t regs, uint32_t eflags, uint32_t err);
// void general_interrupt(void);

///// exception handlers /////
void divide_by_zero();
void reserved_fault();
void non_maskable_interrupt();
void breakpoint();
void overflow();
void bounds_range();
void undefined_opcode();
void device_unavailable();
void double_fault();
void segment_overrun();
void invalid_tss();
void segment_not_present();
void stack_segfault();
void gen_protection();
void intel_reserved();
void floating_point_x87();
void machine_check();
void alignment_check();
void floating_point_SIMD();
void onwards_20();


#endif /* ASM */

#endif /* IDT_H */