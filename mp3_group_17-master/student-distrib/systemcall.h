#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H
#include "lib.h"
#include "file_system.h"
#include "x86_desc.h"
#include "rtc.h"
#include "pcb.h"
#include "level_switch.h"
#include "init_paging.h"
#include "schedule.h"

#define MAGIC_0	0x7f
#define MAGIC_1	0x45
#define MAGIC_2	0x4c
#define MAGIC_3 0x46
#define	ENTRY_0	24
#define	ENTRY_1 25
#define	ENTRY_2 26
#define	ENTRY_3 27
#define BYTE_SIZE	8

#define SUCCESS 0
#define MAX_PROCESS 6
#define TERMINATE_EXCEPTION     256
#define EXCEPTION_SIGNAL 0X0F


int32_t halt(uint8_t status);
int32_t execute (const uint8_t* command);
int32_t read (int32_t fd, void* buf, int32_t nbytes);
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
int32_t open(const uint8_t* filename);
int32_t close(int32_t fd);
int32_t getargs(uint8_t* buf, int32_t nbytes);
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler (int32_t signum, void* handler_address);
int32_t sigreturn(void);
void parse(const uint8_t* command,uint8_t* filename);
int32_t executable_check(const uint8_t* filename);
extern int32_t current_terminal_index;
#endif
