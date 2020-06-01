#include "systemcall.h"
#include "schedule.h"

file_table rtc_operation={rtc_read,rtc_write,rtc_open,rtc_close};
file_table dir_operation={dir_read,dir_write,dir_open,dir_close};
file_table file_operation={file_read,file_write,file_open,file_close};
file_table stdin = {terminal_read, terminal_write, terminal_open, terminal_close};
file_table stdout = {terminal_read, terminal_write, terminal_open, terminal_close};

static uint32_t task_active[MAX_blank_FILES]={0,0,0,0,0,0};



/*  halt
 *  Description: halt the process
 * Input:uint8_t status
 * Output:0 for fail and 1 for success 
 * Side effect: restore the parent data and pages
 */
int32_t halt(uint8_t status){//exit(15)
    int i;
    int pid;
    pcb_t* cur;
    uint32_t esp,ebp;
    // cur = get_current_pcb();                    //get current pcb
/////////////////////////////////////////////////////////////////////////////////////////
    /*get pid and pcb for current terminal*/
    pid=terminal_running_pid(scheduled_terminal);
    cur=get_pcb_by_pid(pid);
    /*set pid in terminal array to be not in use*/
    for(i=max_individual_process-1;i>=0;i--){
    if(terminal_pid[scheduled_terminal][i]!=not_in_use){
        terminal_pid[scheduled_terminal][i]=not_in_use;
        break;
        }
    }
///////////////////////////////////////////////////////////////////////////////////////////
    int ret;
    ret = (int32_t)status;  //change status 32 bits
    cli();
     //clear current data
    file_t* farray = cur->file_arr;
    if(farray == NULL){
         return -1;
    }
    
    //close relevant FDs
    for(i=0;i<MAX_FILES;i++){
        close(i);                           // use close to close the relevant FDs
    }
    esp = cur->parent_esp;
    ebp = cur->parent_ebp;                  // restore the esp and ebp
    task_active[cur->current_p]=0;

    //if the parent process is shell
   if(cur->parent_p == -1){                 // -1 is the pid of parent of shell
        execute((const uint8_t*)"shell");   // if current is shell
   }
   else{
       tss.ss0 = KERNEL_DS;
       tss.esp0=address_8MB-(cur->parent_p *address_8KB)-4;     // 4kB is the esp's space
       //restore parent paging
        map_process_to_phys(cur->parent_p);
        flush_TLB();

    }
    // printf("esp: %d\n",esp);
    // printf("ebp: %d\n",ebp);
    sti();
            asm volatile ("         \n\
            movl %0, %%esi   \n\
            movl %1, %%esp   \n\
            movl %2, %%ebp   \n\
            jmp return_execute      \n\
            "
            :
            : "g" (ret), "g" (esp), "g" (ebp)
            : "edi", "ebp", "esp"
        );
    return SUCCESS;
}
/*  read
 *  Description: read the data in file 
 * Input:int32_t fd, void* buf, int32_t nbytes
 * Output:0 for fail and 1 for success 
 * Side effect: read data in buffer
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    //printf("call read");
    int count;
    int pid;
    if(fd<0 || fd >MAX_FILES || nbytes<0 || buf==NULL){
        return -1;
    }
//////////////////////////////////////////////////////////////
    pid=terminal_running_pid(scheduled_terminal);
    pcb_t* pcb =get_pcb_by_pid(pid);
////////////////////////////////////////////////////////////////
    if(pcb->file_arr[fd].flag !=0){
    
        count =pcb->file_arr[fd].file_op_pointer->read(fd,buf,nbytes);
        //printf("%d",count);
         
        return count;
    }
    return -1; 
}
/*  write
 *  Description: write the data to buf
 * Input:int32_t fd, void* buf, int32_t nbytes
 * Output:0 for fail and 1 for success 
 * Side effect: write data to buffer
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes){
    int count;
    int pid;
     if(fd<0 || fd >MAX_FILES || nbytes<0 || buf==NULL){
        return -1;
    }
//////////////////////////////////////////////////////////////
    pid=terminal_running_pid(scheduled_terminal);
    pcb_t* pcb =get_pcb_by_pid(pid);
////////////////////////////////////////////////////////////////
    if(pcb->file_arr[fd].flag!=0){

          count = pcb->file_arr[fd].file_op_pointer->write(fd,buf,nbytes);
           //printf("%d",count);
           return count;
    }
    return -1;
}

/*
 *   open
 *   DESCRIPTION: open a file in current pcb
 *   INPUTS: filename -- the filename of the file to be open
 *   OUTPUTS: none
 *   RETURN VALUE: the file descriptor number fd in current pcb->file_array for success 0-7
 *				   -1 for failure
 *   SIDE EFFECTS:open the file of the filename and store it in a free file descriptor, or return -1 for no free file descriptor or invaild filename
 */
int32_t open(const uint8_t* filename){
    pcb_t * pcb;
    dentry_t dentry;
    int pid;
    /* temp for loop*/
    int i;

    /*get the pointer of current pcb of current terminal*/
//////////////////////////////////////////////////////////////
    pid=terminal_running_pid(scheduled_terminal);
    pcb =get_pcb_by_pid(pid);
////////////////////////////////////////////////////////////////
    /*if filename is NULL or no such name in dentry, return -1 for failure*/
    if((filename==NULL)||(read_dentry_by_name(filename,&dentry)==-1||filename[0]=='\0')){
        return -1;
    }
    /*find the first fd that can be used, or return -1 if no descriptor is -1*/
    for(i=MIN_FILES;i<MAX_FILES;i++){
        if(pcb->file_arr[i].flag==0){
            /*keep track of where the user is currently reading from file, 0 when open*/
            pcb->file_arr[i].f_pos=0;
            /*set because this file descriptor is using*/
            pcb->file_arr[i].flag=1;
            break;
        }
        /*all file descriptor is full, return -1 for failure*/
        if(i==MAX_FILES-1){
            return -1;
        }
    }
    /*link operation to specific operation*/
    if(dentry.file_type==RTC_TYPE){
        pcb->file_arr[i].file_op_pointer=&rtc_operation;
        /*for rtc,the inode number should be zero*/
         pcb->file_arr[i].inode=0;
    }
    else if(dentry.file_type==DIR_TYPE){
        pcb->file_arr[i].file_op_pointer=&dir_operation;
        /*for directory,the inode number should be zero*/
         pcb->file_arr[i].inode=0;
    }
    else if(dentry.file_type==FILE_TYPE){
        pcb->file_arr[i].file_op_pointer=&file_operation;
        /*for data file,the inode is valid*/
        pcb->file_arr[i].inode=dentry.inode_num;
    }
    /*invalid file type*/
    else{
        return -1; 
    }
    /*if open a specific file fail, return -1 for failure*/
    if(pcb->file_arr[i].file_op_pointer->open(filename)==-1){
        return -1;
    }
    /*return file descriptor number in current pcb file array*/
    //printf("open!\n");
    return i;
}





/*
 *   close
 *   DESCRIPTION: close the file of give fd in current pcb
 *   INPUTS: fd -- the corresponding file descriptor of the file to be closed
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *				   -1 for failure
 *   SIDE EFFECTS:close the file or return -1 for file not in use or invalid fd
 */
int32_t close(int32_t fd){
    pcb_t* pcb;
    int pid;
    /*try to close a invalid file, return value of -1*/
    if((fd<MIN_FILES)||(fd>=MAX_FILES)){
        return -1;
    }
//////////////////////////////////////////////////////////////
    pid=terminal_running_pid(scheduled_terminal);
    if(pid==-1){
        return -1;
    }
    pcb =get_pcb_by_pid(pid);
////////////////////////////////////////////////////////////////
    /*1 is flag in use,0 is not in use */
    if(pcb->file_arr[fd].flag==1){
        pcb->file_arr[fd].flag=0;
    }
    /*if the fd isn't currently in use, return -1*/
    else{
        return -1;
    }
    /*return 0 for success close, -1 for failure*/
    if(pcb->file_arr[fd].file_op_pointer->close(fd)==0){
        return 0;
    }
    else{
        return -1;
    }
}


//===============================TO DO in cp3.4====================================================
/*
 *   getargs
 *   DESCRIPTION:get the arg to the buf
 *   INPUTS: uint8_t* buf, int32_t nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *				   -1 for failure
 *   SIDE EFFECTS:close the file or return -1 for file not in use or invalid fd
 */
int32_t getargs(uint8_t* buf, int32_t nbytes){
    // int j;
    int pid;
//////////////////////////////////////////////////////////////
    pid=terminal_running_pid(scheduled_terminal);
    pcb_t* pcb =get_pcb_by_pid(pid);
////////////////////////////////////////////////////////////////
     // get the current pcb()
    memset(buf,'\0',nbytes);            
    if(pcb->arg==NULL){                     //
        return -1;              
    }
    if(pcb->arg[0]=='\0'){              //remember to consider the situation that arg
        return -1;
    }
    int32_t length =0;
    length=strlen(pcb->arg);            //strlen remmember
    if(nbytes<length){
        length=nbytes;
    }
    strncpy((int8_t*)buf,pcb->arg,length);
    // int i;
    // while (i<length)
    // {
    //     /* code */putc(buf[i]);
    // }
    // for ( j=0;j<strlen((int8_t*)buf);j++)
    //     putc(buf[j]);
    
    return SUCCESS;
}

/*  vidmap
 *  Description: maps the text-mode video memory into user space at a pre-set virtual address. Check whether the address falss within
 *  the address range covered by the single user-level address. the video memory will require you to add 
 *  another page mapping for the program, in this case a 4 kB page.
 * Input: uint8_t** screen_start: pointer of pointer to the start address of vid memory
 * Output:none
 * Return value: if the location is invalid, return -1. the address returned is always the same
 * Side effect: 
 */


/*要使用pcb的话可用其他函数中的方式*/
int32_t vidmap(uint8_t** screen_start){
    uint32_t curr_pde = 0;
    uint32_t curr_pte = 0;
    if (screen_start==NULL){
        return -1;
    }
    if ((uint32_t)screen_start<START_ADDR_VIRTUAL || (uint32_t)screen_start>=START_ADDR_VIRTUAL+FOUR_MB){
        return -1;
    }
    curr_pde |= BITMASK_P | BITMASK_RW | BITMASK_US;
    curr_pde |= (uint32_t)(arr_paging_table_vidmap);    // a new table for vidmap
    arr_paging_directory[(START_ADDR_VIRTUAL+FOUR_MB)/FOUR_MB] = curr_pde;
    curr_pte |= BITMASK_P | BITMASK_RW | BITMASK_US;
    curr_pte |= START_ADDR_VIDEO;
    arr_paging_table_vidmap[0]=curr_pte;
    flush_TLB();
    *screen_start = (uint8_t*)(START_ADDR_VIRTUAL+FOUR_MB);
    return (uint32_t)(START_ADDR_VIRTUAL+FOUR_MB);
}

int32_t set_handler (int32_t signum, void* handler_address){
    return -1;
}
int32_t sigreturn(void){
    return -1;
}
//===================================================================================================
/*  parse
 *  Description: The command is a space-separated sequene of words. The first word
 *  is the filename.The rest word is the file name of the program to be executed
 * Input:const uint8_t* command,uint8_t* filename
 * Output:none
 * Side effect: copy the filename in the command to the file we want.
 */
void parse(const uint8_t* command,uint8_t* filename){
    int i,length;
    int space_length=0;
    while (command[space_length]==' ')
    {
       space_length++;
    }
    
    length = strlen((char*)command);
    //printf("%d",length);
    for(i=space_length;i<length;i++){
        if(command[i] == (uint8_t)' ' || command[i]==(uint8_t)'\n')   //check for the space
            break;
        else{
            filename[i-space_length] = command[i];
        }
    }
    return;
}

/*  parse_arg
 *  Description: helper function for parse the argument, handle space condition
 * Input:const uint8_t* command,  int8_t* arg--argument of commands
 * Side effect: change the arg
 */
void parse_arg(uint8_t* command,int8_t* arg){
    int i,length;
    int space_length=0;
    int count=0;
    length = strlen((char*)command);
    // printf("%d",length);
    while (command[space_length]==' ')
    {
       space_length++;
    }
    while (command[space_length]!=' ' && command[space_length]!='\0')
    {
       space_length++;
    }
    if(space_length>=length){
        return;
    }
    if(command[space_length]=='\0'){
        return;
    }
    while (command[space_length]==' ')
    {
       space_length++;
    }
    // printf("%d",space_length);
    for(i=space_length;i<length;i++){
       if(command[i]==' '){
            count++;
        }
        else{
            arg[i-space_length-count]=command[i];
        }

    }
    // than we get 
    // for ( j=0;j<strlen(arg);j++)
    //     putc(arg[j]);
    return;
}
/*  executable_check
 *  Description: check whether the file is executa
 * Input:const uint8_t* command,uint8_t* filename
 * Output:none
 * Side effect: copy the filename in the command to the file we want.
 */
int32_t executable_check(const uint8_t* filename){

    dentry_t tdentry;
    uint8_t buf[4]; // we need to check first 4 bytes of file.
    int32_t ret;
    ret = read_dentry_by_name(filename,&tdentry);
    //printf("check ret");
    //printf("%d",ret);
    if(ret!=0 || tdentry.file_type!=FILE_TYPE){ 
        return -1;
    }
    ret = read_data(tdentry.inode_num,0,buf,4); //read 4 magic bytes
    if(ret==-1){
        return -1;
    }
    // firstly, check elf magic number
    if(buf[0]!=MAGIC_0 || buf[1]!=MAGIC_1 || buf[2]!=MAGIC_2||buf[3]!=MAGIC_3){// check first 4 bytes
        printf("Wrong magic number.\n");
        return -1;
    }
    return 0;  
}
/*  execute
 *  Description: check whether the file is executa
 * Input:const uint8_t* command,uint8_t* filename
 * Output:none
 * Side effect: copy the filename in the command to the file we want.
 */
int32_t execute (const uint8_t* command){
    int status=0;
    int j,c_len;
    int old_pid;
    int i,k;
    int count=0;
    //int arg_blank=0;
   // printf("0");
   //printf("start execute, command: %s\n",command);
   c_len= strlen((char*)command);
    uint8_t command_store[arg_size];
    memset(command_store,0,arg_size);
    uint32_t entry_point;
    uint8_t filename[arg_size];
    memset(filename,0,arg_size);
    
    /* parse*/
    parse(command,filename);
    /*check for excutable*/
   // printf("1.5");
   for (j=0;j<c_len;j++){
       command_store[j]=command[j];
   }
    // printf("%d1",strlen((char*)command));
    if(-1 == executable_check(filename)){
        return -1;
    }
    // printf("%d2",strlen((char*)command));
    //paging
    int pid=0;
    int parent_id =0;   
    for(k=0;k<=2;k++){
        if(running_terminal[k]==0){
            count+=1;
        }
    }
    for (pid = 0; pid < MAX_PROCESS-count; pid++) {
        if (task_active[pid] == 0) {
            task_active[pid] = 1;
            break;
        }
    }
 
   //printf("%d",pid);
    if (pid >= MAX_PROCESS-count){
          return -1;
    }
    else{
//////////////////////////////////////////////////////////////
        old_pid=terminal_running_pid(current_terminal_index);
        if(old_pid==-1){
            parent_id=-1;
        }
        else{
        pcb_t* parent =get_pcb_by_pid(old_pid);
////////////////////////////////////////////////////////////////
        parent_id=parent->current_p;
    }
}
    for(i=0;i<max_individual_process;i++){
        if(terminal_pid[current_terminal_index][i]==not_in_use){
            terminal_pid[current_terminal_index][i]=pid;
            break;
        }
    }
    // printf("%d3",strlen((char*)command));
    //printf("c");
    map_process_to_phys(pid);
    flush_TLB();
    /* program loader*/
    // printf("%d4",strlen((char*)command));
    entry_point = get_entry_point(filename);
    if(-1==program_loader(filename)){
        return -1;
    }
    // create pcb
    pcb_t* pcb =creat_pcb(pid);
    // printf("%d66",strlen((char*)command_store));
    memset(pcb->arg,0,arg_size);
    parse_arg(command_store,pcb->arg);
    pcb->file_arr[0].file_op_pointer = &stdin;
    pcb->file_arr[0].inode = -1;             //for stdin and stout,we don't need inode
    pcb->file_arr[0].f_pos = 0;
    pcb->file_arr[0].flag =  1;
    // stdout file
    pcb->file_arr[1].file_op_pointer = &stdout;
    pcb->file_arr[1].inode = -1;             //for stdin and stout,we don't need inode
    pcb->file_arr[1].f_pos = 0;
    pcb->file_arr[1].flag =  1;
    pcb->parent_p= parent_id;
    // printf("%d",pcb->parent_p);
    asm volatile("MOVL %%ebp, %%eax;" : "=a" (pcb->parent_ebp));
    asm volatile("MOVL %%esp, %%eax;" : "=a" (pcb->parent_esp));
    i=0;
    for (i=0;i<MAX_blank_FILES;i++){
        pcb->file_arr[i+2].flag =  0;
    }
    
    //context switch
    tss.ss0=KERNEL_DS;
    tss.esp0=address_8MB-pid*address_8KB-4;
    //printf("aaaaa");
    context_switch(entry_point);
    asm volatile(
        "return_execute:;"
        "movl %%esi, %0;"
        :"=r" (status)
        :
        :"esi"
    );
      //printf("%d",status);
    if(status==EXCEPTION_SIGNAL){//exit(15)
      
        return TERMINATE_EXCEPTION;
    }
    //printf("lallala");
   //printf("after execute, command: %s\n",command);
    return 0;
}


