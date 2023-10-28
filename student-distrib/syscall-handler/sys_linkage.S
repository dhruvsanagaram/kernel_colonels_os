#define ASM 1
.globl sys_call_table, handle_syscall, invalid, dispatch_syscall


# Syscall: Move following information to registers
# EAX: system call number IMPORTANT: VERIFY THIS NUMBER IS VALID
# EBX 1st arg
# ECX 2nd arg
# EDX 3rd arg
# Store return value into EAX

# call *sys_call_table(0, %eax, 4)

handle_syscall:
  pushl   %esi
  pushl   %edi
  pushl   %esp

  # arguments
  pushl   %edx         
  pushl   %ecx         
  pushl   %ebx
  pushfl
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
  popfl # restore flags
  popl %ebx
  popl %ecx
  popl %edx
  popl %esp
  popl %edi
  popl %esi

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