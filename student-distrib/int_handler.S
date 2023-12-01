#define ASM
.global keyboard_handler_wrapper
.global rtc_handler_wrapper
.global pit_handler_wrapper

keyboard_handler_wrapper:
    pushal
    call keyb_main
    popal 
    iret

rtc_handler_wrapper:
    pushal
    call rtc_handle
    popal
    iret

pit_handler_wrapper:
    pushal
    call pit_handler
    popal
    iret
