#ifndef TERMINAL_H
#define TERMINAL_H

#define MAX_BUF_SIZE 128

#define _128MB 0x8000000
#define FOUR_MB 0x400000
// #define FOUR_KB 0x1000

#define VIDMEM_TERM1_START _128MB + FOUR_MB
#define VIDMEM_TERM2_START _128MB + FOUR_MB + FOUR_KB
#define VIDMEM_TERM3_START _128MB + FOUR_MB + FOUR_KB + FOUR_KB
#define VIDMEM_FISH _128MB + FOUR_MB + 3 * FOUR_KB
#define MAX_TERMS 3

#include "types.h"

extern char key_buf[MAX_BUF_SIZE];
extern int numChars;
extern int enterKeyPressed;


//helper to change vidmem mapping based on terminal id
void update_video_memory_paging(int term_id);

//term driver syscalls
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t terminal_open(const uint8_t *filename);
int32_t terminal_close(int32_t fd);
int32_t init_terms();
void terminal_switch(int32_t target_tid);

typedef struct {
  int32_t tid;
  int32_t pid;
  //cursor locations within video
  int cursor_x;
  int cursor_y;
  
  //paging offsets for this terminal 
  
  uint32_t vidmem_data; //20 bits
  // uint32_t vidmap_present;

  //Keyboard vars

  int enterKeyPressed;
  int keyb_char_count;
  char key_buf[MAX_BUF_SIZE];
  
} terminal_t;

extern terminal_t terminals[3];
extern terminal_t *schedule_term;
extern terminal_t *view_term;



#endif /* TERMINAL_H */
