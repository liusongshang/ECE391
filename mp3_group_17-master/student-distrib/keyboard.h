//check point1: functionality:Multiple Keyboard Interrupts (all numbers and lower case letters appear on screen) 
#ifndef KEYBOARD_H
#define KEYBOARD_H 
#include "types.h"
//all numbers and lower case letters appear on screen
//so what we meed first is 
//reference: http://lxr.linux.no/linux+v3.5.4/drivers/input/keyboard/atkbd.c
            //https://www.bing.com/search?q=ascii+table&form=EDGTCT&qs=LC&cvid=fad8aa792b1948d1a9f6eb0dde014d12&refig=2008efafbc014d72e022fd44993cb7be&cc=US&setlang=en-US&plvar=0
            //lask.sina.com.cn/b/2YVO8ksgHR.html
            //https://wiki.osdev.org/PS/2_Keyboard
            //

// first we need to scan the keyboard sancode table;
#define scannum 60 //totally 0x3A 

//the varible of keyboard
#define KEY_IRQ 0x01
#define KEY_PORT 0x60
#define L_SHIFT_PRESS 0x2A
#define R_SHIFT_PRESS 0x36
#define L_SHIFT_RELEASE 0xAA
#define R_SHIFT_RELEASE 0xB6
#define CAP_FLAG    0x3A
#define ALT         0x38
#define ALT_RElESE  0xB8
#define F1          0x3B
#define F2          0x3C
#define F3          0x3D
#define CTRL_PRESS 0x1D
#define CTRL_RELEASE 0x9D
#define BIG_L 0x26
#define BACKSPACE 0x0E
#define line_buffer_size 128
#define columns_number 80
#define CURSOR_PORT     0x3D4
#define CURSOR_VALUE    0x3D5
#define LAST_EIGHT      0xFF
#define CURSOR_REG      0x0F
#define CURSOR_REG2     0x0E
#define EIGHT_SHIFT     8
#define ENTER_PRESSED  0x1C
#define KEYSIZE     60
#define VID_MEMORY  0xB8000
#define TERMINAL_VID_MAP    0xB9000
#define DISPLAY_VID_MAP     0xBC000
#define TERMINAL_SIZE       0x1000
#define FOURTHOUSAND        4096
#define NUM_COLS    80
#define NUM_ROWS    25

typedef struct terminal_type{
    int index;
    char buffer[line_buffer_size];
    char term_buffer[line_buffer_size];
    unsigned buff_for_ter;
    unsigned buffind;
    int screen_x;
    int screen_y;    
    int read_flag;
    int rtc_freq;
}terminal_type;                                 // for further use
extern int32_t terminal_open(const uint8_t* filename);
extern int32_t terminal_close(int32_t fd);
extern int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
// the function of keyboard
extern void Keyboard_init();
extern void keyboard_handler();
extern void input_hander(unsigned scancode);
extern void keyboard_interrupt();
extern void back_handler();
extern void enter_handler();
extern void refresh_cursor(int column, int row);
extern void putc_buf(unsigned char character);
char line_buffer[line_buffer_size];
char term_buffer[line_buffer_size];
extern int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
extern terminal_type* get_terminal(int index);
void  switch_to_terminal(unsigned index);
extern void init_vid();
// int flag[3];
#endif /* TESTS_H */

