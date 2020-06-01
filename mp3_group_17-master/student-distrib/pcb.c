#include "pcb.h"/*this is the file for process control block*/

/* creat_pcb
 * DESCRIPTION:creat pcb for excute part
 * INPUT: int pid (pid is process index send from pcb)
 * OUTPUT: pcb*t pcb
 * RETURN VALUE: pcb*t pcb
 */
// file_table stdin = {terminal_read, terminal_write, terminal_open, terminal_close};
// file_table stdout = {terminal_read, terminal_write, terminal_open, terminal_close};
pcb_t* creat_pcb(int pid){ //pid is process index send from pcb
    
    pcb_t * pcb = (pcb_t *)(address_8MB - (pid+1)*address_8KB);
    pcb->current_p = pid; //store the pid
    
    // store current esp and ebp
    //then we need to set the stdin and  stdout file
    // pcb->file_arr[0].file_op_pointer = &stdin;
    // pcb->file_arr[0].inode = -1;             //for stdin and stout,we don't need inode
    // pcb->file_arr[0].f_pos = 0;
    // pcb->file_arr[0].flag =  1;
    // // stdout file
    // pcb->file_arr[1].file_op_pointer = &stdin;
    // pcb->file_arr[1].inode = -1;             //for stdin and stout,we don't need inode
    // pcb->file_arr[1].f_pos = 0;
    // pcb->file_arr[1].flag =  1;

    // // we have 6 file blank , so we need to initalize 
    // for (i=0;i<MAX_blank_FILES;i++){
    //     pcb->file_arr[i+2].flag =  0;
    // }
    return pcb;
}

/* pcb_t* get_current_pcb();
 * DESCRIPTION:get current pcb for system call
 * INPUT: none 
 * OUTPUT: pcb*t pcb
 * RETURN VALUE: pcb*t pcb
 */
pcb_t* get_current_pcb(){
    pcb_t * curr_pcb;
    unsigned int esp;
    // use bit mask to get the current pcb pointer
    asm volatile(
        "movl %%esp, %0" 
        :"=r"(esp)   
        :                
        :"memory"
    );
    curr_pcb =(pcb_t*) (esp & pcb_mark);
    return curr_pcb;
}

/*
get_pcb_by_pid(int32_t pid)
inputs: process index
output: the pointer to process control block
descrition: return the corresponding pcb of the input pid
*/
pcb_t* get_pcb_by_pid(int32_t pid){
    return (pcb_t*)(address_8MB-(pid+1)*address_8KB);
}
    
    
    




