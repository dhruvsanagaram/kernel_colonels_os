#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "keyboard.h"
#include "int_handler.h"
#include "syscall-handler/sys_linkage.h"
#include "process.h"
/*
 0 #DE Divide Error Fault No DIV and IDIV instructions.
 1 #DB RESERVED Fault/
Trap No For Intel use only.
 2 — NMI Interrupt Interrupt No Nonmaskable external interrupt.
 3 #BP Breakpoint Trap No INT 3 instruction.
 4 #OF Overflow Trap No INTO instruction.
 5 #BR BOUND Range Exceeded Fault No BOUND instruction.
 6 #UD Invalid Opcode (Undefined Opcode)
Fault No UD2 instruction or reserved
opcode.1

 7 #NM Device Not Available (No Math Coprocessor) Fault No Floating-point or WAIT/FWAIT instruction.
 8 #DF Double Fault Abort Yes
(Zero) Any instruction that can generate
an exception, an NMI, or an INTR.
 9 Coprocessor Segment Overrun (reserved) Fault No Floating-point instruction.2
10 #TS Invalid TSS Fault Yes Task switch or TSS access.
11 #NP Segment Not Present Fault Yes Loading segment registers or accessing system segments.
12 #SS Stack-Segment Fault Fault Yes Stack operations and SS register loads.
13 #GP General Protection Fault Yes Any memory reference and other protection checks.
14 #PF Page Fault Fault Yes Any memory reference.
15 — (Intel reserved. Do not use.) No
16 #MF x87 FPU Floating-Point Error (Math Fault) Fault No x87 FPU floating-point or WAIT/FWAIT instruction.
17 #AC Alignment Check Fault Yes (Zero) Any data reference in memory.3
18 #MC Machine Check Abort No Error codes (if any) and source
are model dependent.4
19 #XF SIMD Floating-Point Exception Fault No SSE/SSE2/SSE3 floating-point
instructions5
20-31 — Intel reserved. Do not use.
32-
255 — User Defined (Non- reserved) Interrupts Interrupt External interrup

*/

/*
PIC Interrupts
0x20: IRQ0 Timer Chip
0x21: IRQ1 Keyboard
0x22: IRQ2 Cascade to secondary
0x23: IRQ3 
0x24: Serial Port 

*/

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
void page_fault();
void intel_reserved();
void floating_point_x87();
void machine_check();
void alignment_check();
void floating_point_SIMD();
void onwards_20();
void rtc_handler();
//void keyb_handler();




#define NUM_EXCEPTIONS 21
#define INTEL_RESERVED 15

/* void init();
 * Inputs: N/A
 * Return Value: void
 *  Function: Init idt descriptors and set idt entries for exceptions/sys calls/devices */
void init(){

    //iterate through array of idt descriptors and init to 0
    uint32_t i;
    for(i = 0; i < NUM_VEC; i++){
        if(i < NUM_EXCEPTIONS && i != INTEL_RESERVED){
            idt[i].present = 1;                 //make valid IDT entry
            idt[i].seg_selector = KERNEL_CS;
            idt[i].dpl = 0x0;
            idt[i].reserved0 = 0x0;
            idt[i].reserved1 = 0x1;             //sets gate type to 16-bit int gate
            idt[i].reserved2 = 0x1;             //sets gate type to 16-bit int gate
            idt[i].reserved3 = 0x0;
            idt[i].reserved4 = 0x0;
            idt[i].size = 0x1;
        }
        else{
            idt[i].present = 0;
            idt[i].seg_selector = KERNEL_CS;
            idt[i].dpl = 0x0;
            idt[i].reserved0 = 0x0;
            idt[i].reserved1 = 0x1;             //sets gate type to 16-bit int gate
            idt[i].reserved2 = 0x1;             //sets gate type to 16-bit int gate
            idt[i].reserved3 = 0x0;
            idt[i].reserved4 = 0x0;
            idt[i].size = 0x1;
        }
    }

    //set syscall entry to userspace callable (dpl set to 0x3)
    idt[SYSCALL_VEC].dpl = 0x3;
    idt[SYSCALL_VEC].present = 1;
    idt[SYSCALL_VEC].reserved3 = 1;

    //populate idt with exceptions 
    SET_IDT_ENTRY(idt[0], divide_by_zero);
    SET_IDT_ENTRY(idt[1], reserved_fault);
    SET_IDT_ENTRY(idt[2], non_maskable_interrupt);
    SET_IDT_ENTRY(idt[3], breakpoint);
    SET_IDT_ENTRY(idt[4], overflow);
    SET_IDT_ENTRY(idt[5], bounds_range);
    SET_IDT_ENTRY(idt[6], undefined_opcode);
    SET_IDT_ENTRY(idt[7], device_unavailable);
    SET_IDT_ENTRY(idt[8], double_fault);
    SET_IDT_ENTRY(idt[9], segment_overrun);
    SET_IDT_ENTRY(idt[10], invalid_tss);
    SET_IDT_ENTRY(idt[11], segment_not_present);
    SET_IDT_ENTRY(idt[12], stack_segfault);
    SET_IDT_ENTRY(idt[13], gen_protection);
    SET_IDT_ENTRY(idt[14], page_fault);
    SET_IDT_ENTRY(idt[15], intel_reserved);
    SET_IDT_ENTRY(idt[16], floating_point_x87);
    SET_IDT_ENTRY(idt[17], alignment_check);
    SET_IDT_ENTRY(idt[18], machine_check);
    SET_IDT_ENTRY(idt[19], floating_point_SIMD);
    SET_IDT_ENTRY(idt[20], onwards_20);

    //populate idt with system call handlers
    SET_IDT_ENTRY(idt[SYSCALL_VEC], handle_syscall);  
    
    //populate idt with device interrupts -- keyboard, rtc
    idt[KEYB_IRQ_NO].seg_selector = KERNEL_CS;
    idt[KEYB_IRQ_NO].present = 1;
    idt[KEYB_IRQ_NO].reserved3 = 0; 
    idt[KEYB_IRQ_NO].reserved4 = 0;
    idt[KEYB_IRQ_NO].reserved2 = 1;
    idt[KEYB_IRQ_NO].reserved1 = 1;
    idt[KEYB_IRQ_NO].size = 1;
    idt[KEYB_IRQ_NO].reserved0 = 0;
    idt[KEYB_IRQ_NO].dpl = 0;
    idt[KEYB_IRQ_NO].present = 1;
    /////// EDIT ( 10 /20 /23): ADDED IDT LOGIC SET IN KEYB.C ////////
    idt[RTC_IRQ_NO].seg_selector = KERNEL_CS;
    idt[RTC_IRQ_NO].present = 1;
    idt[RTC_IRQ_NO].reserved3 = 0;
    idt[RTC_IRQ_NO].reserved4 = 0;
    idt[RTC_IRQ_NO].reserved2 = 1;
    idt[RTC_IRQ_NO].reserved1 = 1;
    idt[RTC_IRQ_NO].size = 1;
    idt[RTC_IRQ_NO].reserved0 = 0;
    idt[RTC_IRQ_NO].dpl = 0;
    idt[RTC_IRQ_NO].present = 1;

    //SET_IDT_ENTRY(idt[RTC_IRQ_NO], rtc_handler); 
    SET_IDT_ENTRY(idt[RTC_IRQ_NO], rtc_handler_wrapper);
    //SET_IDT_ENTRY(idt[KEYB_IRQ_NO], keyb_handler);
    SET_IDT_ENTRY(idt[KEYB_IRQ_NO], keyboard_handler_wrapper);

    lidt(idt_desc_ptr);
}

/* void divide_by_zero();
 * Inputs: 
 * Return Value: void
 *  Function: Divide by zero exception */
void divide_by_zero(){
    printf("Divide by Zero");
    system_halt(256);
}

/* void reserved_fault();
 * Inputs: N/A
 * Return Value: void
 *  Function: Debug error*/
void reserved_fault(){
    printf("Debug Error");
    system_halt(256);
}

/* void non_maskable_interrupt();
 * Inputs: N/A
 * Return Value: void
 *  Function: Non-maskable-interrupt */
void non_maskable_interrupt(){
    printf("Non-Maskable Interrupt");
    system_halt(256);
}

/* void breakpoint();
 * Inputs: N/A
 * Return Value: void
 *  Function: Breakpoint */
void breakpoint(){
    printf("Breakpoint");
    system_halt(256);
}

/* void overflow();
 * Inputs: N/A
 * Return Value: void
 *  Function: Overflow Trap */
void overflow(){
    printf("Overflow Trap");
    system_halt(256);
}

/* void bounds_range();
 * Inputs: N/A
 * Return Value: void
 *  Function: Bound Range Exceeded */
void bounds_range(){
    printf("Bound Range Exceeded");
    system_halt(256);
}

/* void undefined_opcode();
 * Inputs: N/A
 * Return Value: void
 *  Function: Undefined opcode */
void undefined_opcode(){
    printf("Undefined opcode encountered");
    system_halt(256);
}

/* void device_unavailable();
 * Inputs: N/A
 * Return Value: void
 *  Function: Device is not available */
void device_unavailable(){
    printf("Device is not available");
    system_halt(256);
}

/* void double_fault();
 * Inputs: N/A
 * Return Value: void
 *  Function: double fault encountered */
void double_fault(){
    printf("double fault encountered");
    system_halt(256);
}

/* void segment_overrun();
 * Inputs: N/A
 * Return Value: void
 *  Function: segment overrun fault */
void segment_overrun(){
    printf("segment overrun fault");
    system_halt(256);
}

/* void invalid_tss();
 * Inputs: N/A
 * Return Value: void
 *  Function: Invalid TSS Fault */
void invalid_tss(){
    printf("Invalid TSS Fault");
    system_halt(256);
}

/* void segment_not_present();
 * Inputs: N/A
 * Return Value: void
 *  Function: segment not present */
void segment_not_present(){
    printf("Segment not present");
    system_halt(256);
}

/* void stack_segfault();
 * Inputs: N/A
 * Return Value: void
 *  Function: segmentation fault */
void stack_segfault(){
    printf("segmentation fault");
    system_halt(256);
}

/* void gen_protection();
 * Inputs: N/A
 * Return Value: void
 *  Function: general protection compromised */
void gen_protection(){
    printf("general protection compromised");
    system_halt(256);
}

/* void page_fault();
 * Inputs: N/A
 * Return Value: void
 *  Function: page_fault(); */
void page_fault(){
    int err_code;
    asm volatile (
            "movl %%cr2, %0"
        : "=r" (err_code)
    );
    printf("%x", err_code);
    printf("page fault");
    system_halt(256);
}

/* void intel_reserved();
 * Inputs: N/A
 * Return Value: void
 *  Function: reserved by intel */
void intel_reserved(){
    printf("reserved by intel");
    system_halt(256);
}

/* void floating_point_x87
 * Inputs: N/A
 * Return Value: void
 *  Function: x87 floating point exception */
void floating_point_x87(){
    printf("x87 floating point exception");
    system_halt(256);
}

/* void alignment_check();
 * Inputs: N/A
 * Return Value: void
 *  Function: Alignment Check encountered */
void alignment_check(){
    printf("Alignment Check encountered");
    system_halt(256);
}

/* void machine_check();
 * Inputs: N/A
 * Return Value: void
 *  Function: Machine check encountered */
void machine_check(){
    printf("Machine check encountered");
    system_halt(256);
}

/* void floating_point_SIMD();
 * Inputs: N/A
 * Return Value: void
 *  Function: SIMD floating point exception */
void floating_point_SIMD(){
    printf("SIMD floating point exception");
    system_halt(256);
}

/* void onwards_20();
 * Inputs: N/A
 * Return Value: void
 *  Function: reserved by intel */
void onwards_20(){
    printf("reserved by intel");
    system_halt(256);
}

/* void syscall_handler();
 * Inputs: N/A
 * Return Value: void
 *  Function: Syscall detected */
// void syscall_handler(){
//     printf("Syscall detected");
//     while(1);
// }
////////// USE handle_syscall() INSTEAD ////////////

/* void rtc_handler();
 * Inputs: N/A
 * Return Value: void
 *  Function: Delegates control to rtc interrupt handler*/
void rtc_handler(){
    rtc_handle();
    // asm volatile(
    //     "pushal\n\t"
    //     "call rtc_handle\n\t"
    //     "popal\n\t"
    //     "iret\n\t"
    // );
}

/* void keyb_handler();
 * Inputs: N/A
 * Return Value: void
 *  Function: Delegated control to keyboard handler */
// void keyb_handler(){
//     keyb_main();
//     // asm volatile(
//     //     "pushal\n\t"
//     //     "call keyb_main\n\t"
//     //     "popal\n\t"
//     //     "iret\n\t"
//     // );
// }

