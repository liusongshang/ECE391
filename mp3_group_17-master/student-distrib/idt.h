#ifndef IDT_H
#define IDT_H


#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "exception.h"
#include "keyboard.h"
#include "rtc.h"
#include "schedule.h"
#include "pit.h"
/* the several index in IDT*/
#define PIT_ENTRY		0x20
#define KEYBOARD_ENTRY	0x21
#define RTC_ENTRY		0x28
#define SYSTEMCALL_ENTRY	0x80
/*initialize interrupt descriptor table*/
void init_idt();

#endif




