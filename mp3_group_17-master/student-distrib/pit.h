// #ifndef PIT_H
// #define PIT_H

// #include "lib.h"
// #include "types.h"
// #include "i8259.h"
// #include "schedule.h"
// #include "systemcall.h"

// #define CHANNEL_0 0x40
// #define CHANNEL_1 0x41
// #define CHANNEL_2 0x42
// #define MODE_COMMAND 0x43
// #define INPUT_CLOCK  1193180
// #define CMD_BYTE    0x36
// #define LOW_EIGHT_MASK  0xff
// #define EIGHT_OFFSET    8
// #define PIT_IRQ 0

// extern void pit_interrupt();
// void init_pit(int hz);
// void pit_handler();
// #endif

#ifndef PIT_H
#define PIT_H
#include "lib.h"
#include "types.h"
#include "i8259.h"
#include "systemcall.h"
#include "pcb.h"
#define CHANNEL_0 0x40
#define CHANNEL_1 0x41
#define CHANNEL_2 0x42
#define MODE_COMMAND 0x43
#define INPUT_CLOCK  1193180
#define CMD_BYTE    0x36
#define LOW_EIGHT_MASK  0xff
#define EIGHT_OFFSET    8
#define PIT_IRQ 0
#define MAX_TERMINAL_NUM    3
#define ESP_OFFSET  4
#define PIT_FREQUENCY   50
extern void pit_interrupt();
void init_pit();
void pit_handler();
void map_to_terminal();
int scheduled_terminal;
#endif









