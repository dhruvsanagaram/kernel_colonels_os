#define ASM 1
.globl sys_call_table, handle_syscall, invalid, dispatch_syscall


# Syscall: Move following information to registers
# EAX: system call number IMPORTANT: VERIFY THIS NUMBER IS VALID
# EBX 1st arg
# ECX 2nd arg
# EDX 3rd arg
# Store return value into EAX


handle_syscall:
  pushl %ebp
  pushl   %esi
  pushl   %edi

  # arguments
  pushl   %edx         
  pushl   %ecx         
  pushl   %ebx
  
  # Check if syscall number valid
  cmpl $10, %eax
  jg invalid
  cmpl $1, %eax
  jl invalid

  call *sys_call_table(, %eax, 4) # taken from lecture
  jmp dispatch_syscall
  
invalid:
  movl $-1, %eax

dispatch_syscall:
  #Restore registers
  popl %ebx
  popl %ecx
  popl %edx
  popl %edi
  popl %esi
  popl %ebp
  
  iret

sys_call_table:
  .long 0x0       # return null since syscall #0 does not exist
  .long halt
  .long execute
  .long read
  .long write
  .long open
  .long close
  .long getargs
  .long vidmap
  .long set_handler
  .long sigreturn
