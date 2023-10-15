#include "idt.h"
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

#define NUM_EXCEPTIONS 21
#define INTEL_RESERVED 15

void init(){
    //iterate through array of idt descriptors and init to 0
    uint32_t i;
    for(i = 0; i < NUM_VEC; i++){
        if(i < NUM_EXCEPTIONS && i != INTEL_RESERVED){
            idt[i].present = 1; //make valid IDT entry
        }
        else{
            idt[i].present = 0;
        }
        idt[i].seg_selector = KERNEL_CS;
        idt[i].dpl = 0x0;
        idt[i].reserved0 = 0x0;
        idt[i].reserved1 = 0x1; //sets gate type to 16-bit int gate
        idt[i].reserved2 = 0x1; //sets gate type to 16-bit int gate
        idt[i].reserved3 = 0x0;
        idt[i].reserved4 = 0x0;
        idt[i].size = 0x1;
    }

    //set syscall entry to userspace callable (dpl set to 0x3)
    idt[SYSCALL_VEC].reserved3 = 1; 
    idt[SYSCALL_VEC].dpl = 0x3;
    idt[SYSCALL_VEC].present = 1;

    //populate idt with exceptions 
    /*
    * we need to also make the second arg
    * to the SET_IDT_ENTRY macro the handler ptr
    */
    //Finish this once Sagnik is done writing enum
    SET_IDT_ENTRY(idt[DE], divide_error);
    SET_IDT_ENTRY(idt[DB], reserved_fault);
    SET_IDT_ENTRY(idt[NMI], non_maskable_interrupt);
    SET_IDT_ENTRY(idt[BP], breakpoint);
    SET_IDT_ENTRY(idt[OF], overflow);
    SET_IDT_ENTRY(idt[BR], bounds_range);
    SET_IDT_ENTRY(idt[UD], undefined_opcode);
    SET_IDT_ENTRY(idt[NM], device_unavailable);
    SET_IDT_ENTRY(idt[DF], double_fault);
    SET_IDT_ENTRY(idt[SO], segment_overrun);
    SET_IDT_ENTRY(idt[TS], invalid_tss);
    SET_IDT_ENTRY(idt[NP], segment_not_present);
    SET_IDT_ENTRY(idt[SS], stack_segfault);
    SET_IDT_ENTRY(idt[GP], gen_protection);
    SET_IDT_ENTRY(idt[PF], page_fault);
    SET_IDT_ENTRY(idt[E15], intel_reserved);
    SET_IDT_ENTRY(idt[MF], floating_point_87);
    SET_IDT_ENTRY(idt[AC], alignment_check);
    SET_IDT_ENTRY(idt[MC], machine_check);
    SET_IDT_ENTRY(idt[XF], floating_point_SIMD);
    SET_IDT_ENTRY(idt[R], onwards_20);

    //populate idt with system call handlers
    SET_IDT_ENTRY(idt[SYSCALL_VEC], "system call");
    
    //populate idt with device interrupts -- keyboard, rtc, pic
    idt[KEYB_IRQ_NO].present = 1;
    idt[KEYB_IRQ_NO].reserved3 = 1; //set third LSB to 1 which sets gate type to 32-bit INT gate
    idt[RTC_IRQ_NO].present = 1;
    idt[RTC_IRQ_NO].reserved3 = 1;

    //Do we register the PIC with the idt and port-address each device or connect each device?
    lidt(idt_desc_ptr);
}


///create the appropriate funcs here for each exception
void divide_error(){
    printf("Divide by Zero");
    halt(255);
}

void reserved_fault(){
    printf("Debug Error");
    halt(255);
}

void non_maskable_interrupt(){
    printf("Non-Maskable Interrupt");
    halt(255);
}

void breakpoint(){
    printf("Breakpoint");
    halt(255);
}

void overflow(){
    printf("Overflow Trap");
    halt(255);
}

void bounds_range(){
    printf("Bound Range Exceeded");
    halt(255);
}

void undefined_opcode(){
    printf("Undefined opcode encountered");
    halt(255);
}

void device_unavailable(){
    printf("Device is not available");
    halt(255);
}

void double_fault(){
    printf("double fault encountered");
    halt(255);
}

void segment_overrun(){
    printf("segment overrun fault");
    halt(255);
}

void invalid_tss(){
    printf("Invalid TSS Fault");
    halt(255);
}

void segment_not_present(){
    printf("Segment not present");
    halt(255);
}

void stack_segfault(){
    printf("segmentation fault");
    halt(255);
}

void gen_protection(){
    printf("general protection compromised");
    halt(255);
}

void intel_reserved(){
    printf("reserved by intel");
    halt(255);
}

void floating_point_87(){
    printf("x87 floating point exception");
    halt(255);
}

void alignment_check(){
    printf("Alignment Check encountered");
    halt(255);
}

void machine_check(){
    printf("Machine check encountered");
    halt(255);
}

void floating_point_SIMD(){
    printf("SIMD floating point exception");
    halt(255);
}

void onwards_20(){
    printf("reserved by intel");
    halt(255);
}