#ifndef RTC_H
#define RTC_H

#include "lib.h"
#include "types.h"
#include "i8259.h"

#define IRQ_SLAVE       2
#define IRQ_8           8
#define RTC_PORT        0x70
#define CMOS_PORT       0x71
#define RTC_VALUE       0x71
#define NMI_REG_A       0x8A
#define NMI_REG_B       0x8B
#define NMI_REG_C       0x0C
#define MIN_FREQ        2
#define DEFAULT_FREQ    1024
#define MAX_FREQ        8192
#define DEFAULT_RATE    6
#define MAX_RATE        15
#define FIVE_TO_EIGHT   0xF0
#define TWO_POWER_16    65536




int change_rate(int rate);
extern void rtc_init();
extern void rtc_interrupt();
extern void rtc_handler();
extern int rtc_read(int32_t fd,  void* buf, int32_t nbytes);
extern int rtc_write(int32_t fd, const void* buf, int32_t nbytes);
extern int rtc_open(const uint8_t* filename);
extern int rtc_close(int32_t fd);
volatile int rtc_flag[3];
int rtc_count;
#endif

