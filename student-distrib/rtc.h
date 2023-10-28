#ifndef RTC_H
#define RTC_H

#include "types.h"


int32_t rtc_init(void);

void rtc_handle(void);
int32_t rtc_close(int32_t fd);
int32_t rtc_open(const uint8_t *filename);
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
void freq_change(uint32_t valHz);
#endif
