#include "schedule.h"
/*
terminal_running_pid()
inputs: the terminal index 0-2 to get the pid
outputs: return thr running pid of specific terminal for success
         return -1 for failure
description: get the newest running pid of a specific terminal
*/
int32_t terminal_running_pid(int32_t terminal_index){
int i;
/*check from the newest process in this specific terminal*/
for(i=max_individual_process-1;i>=0;i--){
    if(terminal_pid[terminal_index][i]!=not_in_use){
        // pcb_t* pcb= creat_pcb(terminal_pid[terminal_index][i]);
        // pcb->current_term=terminal_index;
        return terminal_pid[terminal_index][i];
        }
    }
    return -1;
}


/*initialize the terminal pid array*/
int terminal_pid[max_terminal][max_individual_process]={
{not_in_use,not_in_use,not_in_use,not_in_use},
{not_in_use,not_in_use,not_in_use,not_in_use},
{not_in_use,not_in_use,not_in_use,not_in_use}};

int32_t running_terminal[max_terminal]={1,0,0};


