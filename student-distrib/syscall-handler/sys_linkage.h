#ifndef SYS_LINKAGE_H
#define SYS_LINKAGE_H
#include "./syscall.h"
#ifndef ASM
extern void handle_syscall();
#endif /* ASM */
#endif /* SYS_LINKAGE_H */