#ifndef TERMINAL_H
#define TERMINAL_H

#define MAX_BUF_SIZE 128

#include "types.h"

extern char key_buf[MAX_BUF_SIZE];
extern int numChars;
extern int enterKeyPressed;


//term driver syscalls
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t terminal_open(const uint8_t *filename);
int32_t terminal_close(int32_t fd);

typedef struct terminal_t {
  int32_t cur_term;
  int32_t pid;
  terminal_t* next_term;
} terminal_t;

terminal_t* terminals[3];


#endif /* TERMINAL_H */
