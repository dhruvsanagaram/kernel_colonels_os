#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "syscall-handler/syscall.h"

#define CMD_SIZE 10
#define MAX_BUF 128
#define ARG_SIZE 128
#define USER_INDEX 32
#define NEAREST_8KB_BOUND 0x007fe000 //8MB - 8KB





// pcb_t* get_pcb(); //return the current operating pcb based on curr_process global watcher
// pcb_t* get_pcb_from_pid(uint32_t pid); //return the pcb based on the given pid
int32_t system_execute(const uint8_t* command);
int32_t system_halt(uint8_t status);
pcb_t* getRunningPCB(); //get the most recently running pcb from the stack
void setupIRET();

#endif
