#ifndef TERMINAL_H
#define TERMINAL_H

#define MAX_BUF_SIZE 128


extern static char key_buf[MAX_BUF_SIZE];
extern volatile static int numChars = 0;
extern volatile static int enterKeyPressed = 0;


//term driver syscalls
int32_t terminal_read(int32_t fd, char *buf, int32_t nbytes);
int32_t terminal_write(int32_t fd, const char *buf, int32_t nbytes);
int32_t terminal_open(const uint8_t *filename);
int32_t terminal_close(int32_t fd);

#endif /* TERMINAL_H */
