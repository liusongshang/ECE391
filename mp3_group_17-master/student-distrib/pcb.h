#ifndef _PCB_H
#define _PCB_H

#include "lib.h"
#include "file_system.h"
#include "keyboard.h"
#include "rtc.h"
#define MIN_FILES 2
#define MAX_FILES 8
#define MAX_blank_FILES 6
#define address_8MB   0x800000
#define address_8KB   0x2000
#define pcb_mark   0xffe000
#define arg_size  130
typedef struct {
    
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
}file_table;

typedef struct{
    file_table* file_op_pointer; //file operation point
    int32_t inode;  //inode 
    int32_t f_pos; //file position
    int32_t flag;  //flags 
}file_t;

typedef struct{
  file_t file_arr[MAX_FILES];
  int current_p;// the current process index
  int parent_p; //the parent process index
  int parent_esp;  //store current esp and ebp
  int parent_ebp;
  int esp;
  int ebp;
  int8_t arg[arg_size];

}pcb_t;


pcb_t* creat_pcb(int pid);
pcb_t* get_current_pcb();
pcb_t* get_pcb_by_pid(int32_t pid);

#endif
