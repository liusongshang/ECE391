#include "keyboard.h"
#include "lib.h"
#include "x86_desc.h"
#include "i8259.h"
#include "schedule.h"
#include "pcb.h"
#include "pit.h"
 terminal_type terminal[3];
int current_terminal_index;
const unsigned char scantable[scannum] ={      // 64 is the value for scancode store
     '\0',27,     // esc is 27
    '1','2','3','4','5','6','7','8','9','0','-','=','\b',// number and backspace
    '\0','q','w','e','r','t','y','u','i','o','p','[',']',  //tab and the fist line
    '\n' ,'\0',// the enter is here after the first line and left control
    'a','s','d','f','g','h','j','k','l',';','\'','`', //as ps2 said , put '`' here
    '\0',//left shift here
    '\\', // represent a '\'
    'z','x','c','v','b','n','m',',','.','/',    //the third line of character 
    //we do not need to care keypad
    '\0', '\0','\0', ' ', '\0'      // so the final line only " " is meanful
};
const unsigned char SHIFTscantable[scannum] ={      // 64 is the value for scancode store
     '\0',27,     // esc is 27
    '!','@','#','$','%','^','&','*','(',')','_','+','\b',// number and backspace
    '\0','Q','W','E','R','T','Y','U','I','O','P','{','}',  //tab and the fist line
    '\n' ,'\0',// the enter is here after the first line and left control
    'A','S','D','F','G','H','J','K','L',':','"','~', //as ps2 said , put '`' here
    '\0',//left shift here
    '|', // represent a '\'
    'Z','X','C','V','B','N','M','<','>','?',    //the third line of character 
    //we do not need to care keypad
    '\0', '\0','\0', ' ','\0'      // so the final line only " " is meanful
};
const unsigned char CAPscantable[scannum] ={      // 64 is the value for scancode store
     '\0',27,     // esc is 27
    '1','2','3','4','5','6','7','8','9','0','-','=','\b',// number and backspace
    '\0','Q','W','E','R','T','Y','U','I','O','P','[',']',  //tab and the fist line
    '\n' ,'\0',// the enter is here after the first line and left control
    'A','S','D','F','G','H','J','K','L',';','\'','`', //as ps2 said , put '`' here
    '\0',//left shift here
    '\\', // represent a '\'
    'Z','X','C','V','B','N','M',',','.','/',    //the third line of character 
    //we do not need to care keypad
    '\0', '\0','\0', ' ','\0'      // so the final line only " " is meanful
};
const unsigned char CAPSHIFTscantable[scannum] ={      // 64 is the value for scancode store
     '\0',27,     // esc is 27
    '!','@','#','$','%','^','&','*','(',')','_','+','\b',// number and backspace
    '\0','q','w','e','r','t','y','u','i','o','p','{','}',  //tab and the fist line
    '\n' ,'\0',// the enter is here after the first line and left control
    'a','s','d','f','g','h','j','k','l',':','"','~', //as ps2 said , put '`' here
    '\0',//left shift here
    '|', // represent a '\'
    'z','x','c','v','b','n','m','<','>','?',    //the third line of character 
    //we do not need to care keypad
    '\0', '\0','\0', ' ', '\0'      // so the final line only " " is meanful
};
unsigned last = 0;
unsigned CAPdown=0;
unsigned SHIFTdown=0;
unsigned CTRLdown=0;
unsigned overflowflag=0;
unsigned alt_flag=0;
// static char* video_mem = (char *)VID_MEMORY;

terminal_type* get_terminal(int index){
    return &(terminal[index]);
}
/* 
 *  Keyboard_init
 *   DESCRIPTION: init the keyboard
 *   INPUTS: nome
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: called by kermnel.c to init
 */

void Keyboard_init(){
	 enable_irq(KEY_IRQ);				// enable the interrupt
	//  last=0;
	//  CAPdown=0;
    //  SHIFTdown=0;
}

/* 
 *  keyboard_handler
 *   DESCRIPTION: handler the interept
 *   INPUTS: nome
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: called by idt.c to interupt
 */
void keyboard_handler(){
	//send_eoi(KEY_IRQ);

	cli();
	
	unsigned scancode = 0;
	
	do{
		scancode = inb(KEY_PORT);			// while 1
		if(scancode>0)
			break; 
	} while (1);
	// if(scancode!=last+0x80){				//the pressed down bootom and press up bottom has 0x80 differentce 
	// 	if(scancode<60){
	// 		character= scantable[scancode];	// put out the result
	// 		putc(character);
	// 	}
	// }
	// last=scancode;
    // if(CTRLdown==1 && scancode==BIG_L){
    //     clear();            // clear first in the lib.c
    //     //put cursor at the top
    //     reset_cursor();
    //     //return;
    // }
    switch (scancode)
    {
    case CTRL_PRESS :               // case for crtl
        CTRLdown=1;
        break;
    case CTRL_RELEASE :             // realse crtl
        CTRLdown=0;
        break;
    case L_SHIFT_PRESS :
        SHIFTdown=1;
        break;
    case R_SHIFT_PRESS :            // we have two shift 
        SHIFTdown=1;
        break;
    case L_SHIFT_RELEASE :          // remmember
        SHIFTdown=0;
        break;
    case R_SHIFT_RELEASE :          // left and shift
        SHIFTdown=0;
        break;
    case CAP_FLAG:
        if(CAPdown==0){
            CAPdown=1;
        }
        else
        {
            CAPdown=0;
        }
        //printf("%d",CAPdown);
        break;
    case ALT:
            alt_flag = 1;
            break;
    case ALT_RElESE:
            alt_flag = 0;
            break;
    default:
        input_hander(scancode);
    }
	 if(CTRLdown==1 && scancode==BIG_L){
        clear();            // clear first in the lib.c
        //put cursor at the top
        terminal[current_terminal_index].buffind=0;
        overflowflag=0;
        reset_cursor();
        //return;
    }
    refresh_cursor(get_x(),get_y());
     terminal[current_terminal_index].screen_x=get_x();
     terminal[current_terminal_index].screen_y=get_y();
	send_eoi(KEY_IRQ);				// tell pic that stop the interrupt
	sti();

}


/* 
 *  input_hander
 *   DESCRIPTION: handler the input
 *   INPUTS: unsigned scancode
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: print the input
 */
void input_hander(unsigned scancode){
    unsigned char character = 0;
    if(scancode==BACKSPACE){
        back_handler();
        return;
    }
    if(scancode==ENTER_PRESSED){
        character= scantable[scancode];	// put out the result
		putc_buf(character);
        enter_handler();
        terminal[current_terminal_index].read_flag = 1;
        return;
    }
    if(scancode!=last+0x80){            
        if(scancode<62 ){                   // 62 for the 62 scancode we used
    
        //if((scancode>1 && scancode<16) || ((scancode>16 && scancode<0x3b)&&(scancode!=0x38)) ) {       // Keyboard handler handles unknown scancodes 
        if(SHIFTdown==0 && CAPdown==0){
            character= scantable[scancode];	// put out the result
			putc_buf(character);
        }
        if(SHIFTdown==1 && CAPdown==0){
            character= SHIFTscantable[scancode];	// put out the result
			putc_buf(character);
           
        }
        if(SHIFTdown==0 && CAPdown==1){
            character= CAPscantable[scancode];	// put out the result
			putc_buf(character);
          
        }
        if(SHIFTdown==1 && CAPdown==1){
            character= CAPSHIFTscantable[scancode];	// put out the result
			putc_buf(character);
          
        }
        if(alt_flag==1 && (scancode == F1 || (scancode == F2) || (scancode == F3))){
            send_eoi(KEY_IRQ);
            //sti();
            // switch(scancode){
            //     case F1:
            //         switch_to_terminal(0);
            //         break;
            //     case F2:
            //         switch_to_terminal(1);
            //         break;
            //     case F3:
            //         switch_to_terminal(2);
            //         break;
            // }
            switch_to_terminal(scancode-0x3B);          // 0x3B is F1
        }
            
            return;
        }
        // else
        // {
        //     printf("unknown scancodes");
        // }
        
         
    }
    last= scancode;
}

/* 
 *  back_handler
 *   DESCRIPTION: delete the character 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: delete the character 
 */
extern void back_handler(){
    if(overflowflag==1){
        int n;
        for ( n = 0; n < 9; n++)
        {
            delete_char();
        }
        //buffind--;
        terminal[current_terminal_index].buffer[terminal[current_terminal_index].buffind]='\0';
        overflowflag=0;
    }
    if(terminal[current_terminal_index].buffind>0){
        terminal[current_terminal_index].buffind--;
        terminal[current_terminal_index].buffer[terminal[current_terminal_index].buffind]='\0';
        delete_char();//delete one character her i write the function in lib.c
    }
}
   /* 
 *  enter_handler
 *   DESCRIPTION: delete the character 
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: handle the enter
 */
extern void enter_handler(){
    terminal[current_terminal_index].buff_for_ter=terminal[current_terminal_index].buffind;
    memset(terminal[current_terminal_index].term_buffer,0,line_buffer_size);
    int i;
    for (i=0;i<line_buffer_size;i++){
        terminal[current_terminal_index].term_buffer[i]=terminal[current_terminal_index].buffer[i];
    }
    terminal[current_terminal_index].buffind=0;
    memset(terminal[current_terminal_index].buffer,0,line_buffer_size);
    if(terminal[current_terminal_index].buff_for_ter==line_buffer_size){
    putc_keyboard('\n');
    }
}    
                     
/* 
 *  putc_buf
 *   DESCRIPTION: putc and save the buffer in line-bufer
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: delete the character 
 */
void putc_buf(unsigned char character){
    if(character!='\0'){
    if(terminal[current_terminal_index].buffind<(line_buffer_size)){
        terminal[current_terminal_index].buffer[terminal[current_terminal_index].buffind] = character;
        terminal[current_terminal_index].buffind++;
        putc_keyboard(character);            // cahneg teh puc in the lib.c
    }
    }
    //printf("%d",current_terminal_index);
    return;
}
/* 
 *  refresh_cursor
 *   DESCRIPTION: refresh the cursor
 *   INPUTS: int column, int row
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: refresh the cursor
 *   reference:https://wiki.osdev.org/Text_Mode_Cursor
 */
extern void refresh_cursor(int column, int row){
    unsigned short postion = (row*columns_number) +column;          // the postion of the colum 
    outb(CURSOR_REG,CURSOR_PORT);                                         
	outb((uint8_t) (postion & LAST_EIGHT),CURSOR_VALUE );                    // need to refresh
	outb(CURSOR_REG2,CURSOR_PORT);
	outb((uint8_t) ((postion >> EIGHT_SHIFT) & LAST_EIGHT),CURSOR_VALUE); 
}

/* 
 *  terminal_open
 *   DESCRIPTION: open the terminal
 *   INPUTS: filename
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: initialize the terminal stuff
 */
int32_t terminal_open(const uint8_t* filename){
    int i;
    for(i=0;i<3;i++){
        terminal[i].index=i;
        terminal[i].buffind=0;
        terminal[i].screen_x = 0;
        terminal[i].screen_y = 0;
        terminal[i].read_flag = 0;
        terminal[i].buff_for_ter = 0;
        memset(terminal[i].term_buffer,0,line_buffer_size);
    }
    current_terminal_index = 0;
    return 0;
}

/* 
 *  terminal_open
 *   DESCRIPTION: DO NOTHING
 *   INPUTS: fd
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: DO NOTHING 
 */
int32_t terminal_close(int32_t fd){
    return 0;                               //do nothing
}

/* 
 *  terminal_read
 *   DESCRIPTION: read the keyboard buffer into the given buffer
 *   INPUTS: fd, buf and nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes read
 *   SIDE EFFECTS: read the keyboard buffer into the given buffer
 */
extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    // printf("?");    
    int32_t count,i;
    // pcb_t * temp_pcb,*temp_pcb2;
    if(nbytes == 0 || buf==NULL || fd==1)
        return -1;                           // if we don't have something to read, return 0
    sti();
    //printf("%d",temp_pcb->current_term);
    while(current_terminal_index!= scheduled_terminal || (!terminal[current_terminal_index].read_flag));
    terminal[current_terminal_index].read_flag = 0;
    // while(temp_pcb->current_p!=(temp_pcb2 = get_current_pcb())->current_p)
    cli();
    unsigned char* change_buff= (unsigned char*)buf;
    if(terminal[current_terminal_index].buff_for_ter < nbytes)
        count = terminal[current_terminal_index].buff_for_ter;
    else
        count = nbytes;
    for(i=0;i<count;i++){
        change_buff[i]=terminal[current_terminal_index].term_buffer[i];      // read the buffer
    }
    change_buff[count] = '\0';
    
    return count;

}

/* 
 *  terminal_write
 *   DESCRIPTION: write the given buffer into the screen
 *   INPUTS: fd, buf and nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: the number of bytes write
 *   SIDE EFFECTS: write the given buffer into the screen
 */
extern int32_t terminal_write(int32_t fd,const void* buf, int32_t nbytes){
    if(buf == NULL || nbytes == 0 ||fd == 0 ){
         return -1;
        
    }
    int i,temp;
    temp=0;
    cli();
    uint8_t* change_buff = (uint8_t*)buf;
    while(change_buff[temp]!=NULL){
        temp ++;
    }
    if(nbytes > temp)
        nbytes = temp;
    for(i=0;i<nbytes;i++){
       if(current_terminal_index == scheduled_terminal )
        putc_keyboard(change_buff[i]);      // if the process terminal is current visiable terminal
        else{
            putc_backup(change_buff[i],scheduled_terminal); // if the process terminal is not visiable, write the buffer into backup memory
        }
    }
    sti();
        return nbytes;               
    }

// extern void init_vid(){
//     int32_t i,j;
//     for(j=0;j<3;j++){
//         for (i = 0; i < NUM_ROWS * NUM_COLS; i++) {
//         *(uint8_t *)(video_mem +TERMINAL_SIZE*j + (i << 1)) = 0x0;
//         *(uint8_t *)(video_mem +TERMINAL_SIZE*j+ (i << 1) + 1) = 0x7;
//     }
//     }
    
// }


/* 
 *   switch_to_terminal(unsigned index)
 *   DESCRIPTION: switch to the target terminal 
 *   INPUTS:the index of destinate terminal
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: switch to the target terminal 
 */
void  switch_to_terminal(unsigned index){
    int store;
    if(index == current_terminal_index)
        return ;
    uint32_t target_address = TERMINAL_VID_MAP + TERMINAL_SIZE * index;
    uint32_t current_address = TERMINAL_VID_MAP + TERMINAL_SIZE * current_terminal_index;
    memcpy((void*)current_address, (void*)VID_MEMORY, FOURTHOUSAND);
    // switch_terminal_paging(index, current_terminal_index);
	memcpy((void*)VID_MEMORY, (void*)target_address, FOURTHOUSAND);
    current_terminal_index = index;
    refresh_cursor(terminal[current_terminal_index].screen_x,terminal[current_terminal_index].screen_y);// efresh the screen cursor 
    set_x(terminal[current_terminal_index].screen_x);
    set_y(terminal[current_terminal_index].screen_y);
    running_terminal[index]=1;
    if(terminal_pid[index][0]==-1){
        
        pcb_t * pcb = get_current_pcb();
        asm volatile("MOVL %%ebp, %%eax;" : "=a" (pcb->ebp));
        asm volatile("MOVL %%esp, %%eax;" : "=a" (pcb->esp));
        /*use int store to store and restore scheduled_terminal*/
        store=scheduled_terminal;
        scheduled_terminal=index;
        execute((const uint8_t*)"shell");
        scheduled_terminal=store;
    }
    // printf("%d",current_terminal_index);
    // printf("%d",scheduled_terminal);
}

