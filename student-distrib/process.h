#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "syscall-handler/syscall.h"
#include "filesys.h"

#define CMD_SIZE 10
#define MAX_BUF 128
#define ARG_SIZE 128
#define USER_INDEX 32
#define NEAREST_8KB_BOUND 0x007fe000 //8MB - 8KB
#define USER_ESP 0x8400000 - sizeof(int32_t)


void next_process();


int32_t system_execute(const uint8_t* command);
int32_t system_halt(uint16_t status);
pcb_t* getRunningPCB(); //get the most recently running pcb from the stack
pcb_t* getPCBByPid(int pid);

//////HELPERS FOR EXECUTE (easier testing suite)///////
void parse_command(const uint8_t* command, uint8_t* cmd, uint8_t *arg1);
int32_t check_exec(dentry_t *dentry, uint8_t *buffer, uint8_t *cmd);


#endif
