#define ASM     1

.globl enable
.align 4
enable: # Pass in page address
    pushl %ebp
    movl %esp, %ebp
    movl 8(%ebp), %eax

    # pushl %ebx
    # pushl %esi
    # pushl %edi
    
    movl %eax, %cr3

    movl %cr4, %eax # read from cr4 for writing
    orl $0x10, %eax # enable the bit for setting 4MB pages by oring the PS bit
    # this is the PSE bit (bit 5)
    movl %eax, %cr4 # write to cr4

    # See: https://wiki.osdev.org/CPU_Registers_x86

    # lmswl %eax # read from cr0 for writing


    movl %cr0, %eax
    # Paging and protection bits set
    orl $0x80000001, %eax #set the MSB and LSB to 1
    # this sets Paging:31 (PG) and protection:0 (PE) bits
    movl %eax, %cr0
    
    # flush the TLB
    movl %cr3, %eax
    movl %eax, %cr3


    leave
    ret