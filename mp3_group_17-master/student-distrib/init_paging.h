#include "types.h"

#ifndef _INIT_PAGING_H
#define _INIT_PAGING_H

#define DIRECTORY_TABLE_BOUNDARY 4096
#define NUM_0F_PTE  1024
#define NUM_OF_PDE  1024
#define START_ADDR_KERNEL   0x00400000    //4MB
#define START_ADDR_VIDEO    0xB8000
#define START_ADDR_VIRTUAL  0x08000000
#define VIDEO_INDEX     0xB8
#define BITMASK_P 0x0001    // valid pte set to 1
#define BITMASK_RW  0x0002  //SET TO 1
#define BITMASK_US  0x0004  // 1 for user_lvl; 0 for kernel level
#define BITMASK_PWT 0x0008  // pdf said we should set it to 0
#define BITMASK_PCD 0x0010  // 0 for video memory page
#define BITMASK_A   0x0020  // won't use it?
#define BITMASK_D   0x0040  //dirty bit, pdf said set it to 0 ^^ 
#define BITMASK_PAT 0x0080  // set to 0
#define BITMASK_PS  0x0080
#define BITMASK_G   0x0100  
#define FOUR_KB_OFFSET 12
#define ALIGNED_CORRECTED   0xfffff000
#define EIGHT_MB    0x00800000
#define FOUR_MB     0x00400000
#define FOUR_KB_SIZE    0x001000

uint32_t arr_paging_table[NUM_0F_PTE] __attribute__((aligned(DIRECTORY_TABLE_BOUNDARY)));
uint32_t arr_paging_directory[NUM_OF_PDE] __attribute__((aligned(DIRECTORY_TABLE_BOUNDARY)));
uint32_t arr_paging_table_vidmap[NUM_0F_PTE] __attribute__((aligned(DIRECTORY_TABLE_BOUNDARY)));

void init_paging();
void init_paging_directory();
void init_paging_table();
void set_control_regs();
void map_process_to_phys(uint32_t process_id);
void set_up_terminal_pages();
void switch_terminal_paging(int term,int curr_term);
void flush_TLB();
#endif 
