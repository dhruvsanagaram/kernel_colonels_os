#ifndef SYSCALL_H
#define SYSCALL_H

#include "../types.h"
#include "../filesys.h"

/**
* FILE OPERATION TABLE
*/

#define EIGHT_MB  0x8000000
#define FOUR_MB   0x4000000
#define PROGRAM_IMAGE 0x8048000
#define EIGHT_KB 0x2000
#define MAX_FILES_PER_TASK 8

typdef struct {
  int32_t (*open)(const uint8_t* filename);
  int32_t (*close)(int32_t fd);
  int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
  int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} fop_t;





/**
* FILE DESCRIPTOR
* 
*/
typedef struct {
  fop_t* fops; // file operations ptr
  uint32_t inode_num;
  uint32_t fpos; //position within file to read from
  uint32_t flags;
} file_d_t;

/**
* PROCCESS CONTROL BLOCK (PCB)
* 
*/
typedef struct {
  file_d_t fdarray[MAX_FILES_PER_TASK];
  uint32_t pid;
  uint32_t parent_pid;
} pcb_t;

file_d_t stdin;  //index 0 of the fd array
file_d_t stdout; //index 1 of the fd array
file_d_t rtc_file;


/////////////////////////////////////////////////////////////////////
/////////////////////////// SYSTEM CALLS ///////////////////////////
////////////////////////////////////////////////////////////////////

/////////// 
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen start);
int32_t set_handler (int32_t signum, void* handler address);
int32_t sigreturn (void);


#endif /* SYSCALL_H */
