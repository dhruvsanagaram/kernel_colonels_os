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



void init(){
    //iterate through array of idt descriptors and init to 0
    uint32_t i;
    int numExceptions = 21;
    int intelReserved = 15;
    for(i = 0; i < NUM_VEC; i++){
        if(i < 21 && i != intelReserved){
            idt[i].present = 1;
        }
        else{
            idt[i].present = 0;
        }
    }

    //populate idt with exceptions 
    //Finish this once Sagnik is done writing enum
    SET_IDT_ENTRY(idt[DE], "divide error");
    SET_IDT_ENTRY(idt[DB], "divide error");
    SET_IDT_ENTRY(idt[NMI], "divide error");
    SET_IDT_ENTRY(idt[BP], "divide error");
    SET_IDT_ENTRY(idt[OF], "divide error");
    SET_IDT_ENTRY(idt[BR], "divide error");
    SET_IDT_ENTRY(idt[UD], "divide error");
    SET_IDT_ENTRY(idt[NM], "divide error");
    SET_IDT_ENTRY(idt[DF], "divide error");
    SET_IDT_ENTRY(idt[SO], "divide error");
    SET_IDT_ENTRY(idt[TS], "divide error");
    SET_IDT_ENTRY(idt[NP], "divide error");
    SET_IDT_ENTRY(idt[SS], "divide error");
    SET_IDT_ENTRY(idt[GP], "divide error");
    SET_IDT_ENTRY(idt[PF], "divide error");
    SET_IDT_ENTRY(idt[E15], "divide error");
    SET_IDT_ENTRY(idt[MF], "divide error");
    SET_IDT_ENTRY(idt[AC], "divide error");
    SET_IDT_ENTRY(idt[MC], "divide error");
    SET_IDT_ENTRY(idt[XF], "divide error");
    SET_IDT_ENTRY(idt[R], "divide error");




    //populate idt with system call handlers
    SET_IDT_ENTRY(idt[SYSCALL_VEC], "system call");
    
    //populate idt with device interrupts -- keyboard, rtc, pic
    //Do we register the PIC with the idt and port-address each device or connect each device?
}