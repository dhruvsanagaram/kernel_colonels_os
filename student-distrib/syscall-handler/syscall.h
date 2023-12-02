#ifndef SYSCALL_H
#define SYSCALL_H

#include "../types.h"


/**
* FILE OPERATION TABLE
*/
#define FOUR_MB   0x400000
#define PROGRAM_IMAGE 0x8048000 //this address is in virtual memory
#define EIGHT_MB 0x800000
#define EIGHT_KB 0x2000
#define MAX_FILES_PER_TASK 8

typedef struct {
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

//////// FILE OPS FOR THE DIFFERENT DEVICES //////////
fop_t nul_fops; //index 0
fop_t rtc_fops; // index 1
fop_t dir_fops; //index 2
fop_t file_fops; //index 3
fop_t stdin_fops; //index 4
fop_t stdout_fops; //index 5

/**
* PROCCESS CONTROL BLOCK (PCB)
* 
*/
typedef struct {
  file_d_t fd_arr[MAX_FILES_PER_TASK];
  uint32_t pid;
  uint32_t parent_pid;
  uint32_t tss_kernel_stack_ptr; 
  uint32_t process_eip;
  uint32_t process_esp;
  uint32_t process_ebp;
  uint8_t* arg1;
  int vidmap_present;
  int32_t tid;
} pcb_t;


/////////////////////////////////////////////////////////////////////
/////////////////////////// SYSTEM CALLS ///////////////////////////
////////////////////////////////////////////////////////////////////
int32_t halt (uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write (int32_t fd, const void* buf, int32_t nbytes);
int32_t open (const uint8_t* filename);
int32_t close (int32_t fd);
int32_t getargs (uint8_t* buf, int32_t nbytes);
int32_t vidmap (uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn (void);


/////////////// HELPERS //////////////
int32_t populate_fops(); //populate file ops
//null fileops to pass
int32_t nul_read (int32_t fd, void* buf, int32_t nbytes);
int32_t nul_write (int32_t fd, const void* buf, int32_t nbytes);
int32_t nul_open (const uint8_t* filename);
int32_t nul_close (int32_t fd);


#endif /* SYSCALL_H */
