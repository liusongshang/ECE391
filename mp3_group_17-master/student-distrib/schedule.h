#ifndef _SCHEDULE_H
#define _SCHEDULE_H
#include "lib.h"
#include "pcb.h"
#define max_process 6
#define max_terminal 3
#define not_in_use -1
#define max_individual_process 4
/*a two dimension array to store process index in every terminal for each process*/
/*初始化可在kernel里或schedule.c里，暂无,用-1表示not in use,不能用0*/
int32_t terminal_pid[max_terminal][max_individual_process];
int32_t running_terminal[max_terminal];
int32_t terminal_running_pid(int32_t terminal_index);

extern int32_t current_terminal_index;
extern int scheduled_terminal;
#endif


