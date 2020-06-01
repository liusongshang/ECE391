#include "pit.h"
/* 
 *   init_pit()
 *   DESCRIPTION: initilize the pit interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 *  Based on pit tutorial -- http://www.osdever.net/bkerndev/Docs/pit.htm
 */
void init_pit(){
    int divisor = INPUT_CLOCK/PIT_FREQUENCY;
    outb(CMD_BYTE,MODE_COMMAND);
    outb(divisor&LOW_EIGHT_MASK,CHANNEL_0); /* Set low byte of divisor */
    outb(divisor>>EIGHT_OFFSET,CHANNEL_0);  /* Set high byte of divisor */
    enable_irq(PIT_IRQ);
    scheduled_terminal=current_terminal_index;
}
/* 
 *   pit_handler
 *   DESCRIPTION: generate pit interrupt periodically, do the RR scheduling
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: schedule the processes
 */
void pit_handler(){
    send_eoi(PIT_IRQ);
    //clear interrupt flag
    cli();
    //  then loop all the terminal
    //int i;
    int curent_process;
    int next_process;
    // int running_term;
    pcb_t* new;
    pcb_t* old;
    curent_process=terminal_running_pid(scheduled_terminal);
    old = get_current_pcb();
    
    if((terminal_running_pid((scheduled_terminal+1)%MAX_TERMINAL_NUM)==-1)){
        if(terminal_running_pid((scheduled_terminal+2)%MAX_TERMINAL_NUM)==-1){
            return; // which means threre is only one  terminal running
        }
        next_process=terminal_running_pid((scheduled_terminal+2)%MAX_TERMINAL_NUM);         // next process using the pid
        scheduled_terminal=(scheduled_terminal+2)%MAX_TERMINAL_NUM;
    }
    else
    {
    next_process=terminal_running_pid((scheduled_terminal+1)%MAX_TERMINAL_NUM);
    scheduled_terminal=(scheduled_terminal+1)%MAX_TERMINAL_NUM;                     // schechdule terminal not current terminal
    }
    //printf("%d",next_process);
     new = creat_pcb(next_process);
    // running_term=new->current_term;
    // handler the tss
    // which is context switch part 
    map_process_to_phys(next_process);  // start the next process
    tss.ss0=KERNEL_DS;
    tss.esp0=address_8MB-next_process*address_8KB-ESP_OFFSET;
    
    map_to_terminal();
    flush_TLB();
    asm volatile("MOVL %%ebp, %%eax;" : "=a" (old->ebp));
    asm volatile("MOVL %%esp, %%eax;" : "=a" (old->esp));

    asm volatile("MOVL %%eax, %%ebp;" :: "a" (new->ebp));
    asm volatile("MOVL %%eax, %%esp;" :: "a" (new->esp));
    sti();
    return;
    
}
/* 
 *   map_to_terminal
 *   DESCRIPTION: Used for fish. if the running terminal is not displayed, map to the vidmap page
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: change the paging
 */
void map_to_terminal(){
    uint32_t curr_pde = 0;
    uint32_t curr_pte = 0;
    curr_pde |= BITMASK_P | BITMASK_RW | BITMASK_US;
    curr_pde |= (uint32_t)(arr_paging_table_vidmap);    // a new table for vidmap
    arr_paging_directory[(START_ADDR_VIRTUAL+FOUR_MB)/FOUR_MB] = curr_pde;
    curr_pte |= BITMASK_P | BITMASK_RW | BITMASK_US;
    if(scheduled_terminal!=current_terminal_index){// if ot displayed, map to the buffer
        curr_pte |= START_ADDR_VIDEO + (scheduled_terminal+1)*FOUR_KB_SIZE; 
    }
    else{
        curr_pte |= START_ADDR_VIDEO;   
    }
    arr_paging_table_vidmap[0]=curr_pte;
    return;
}







