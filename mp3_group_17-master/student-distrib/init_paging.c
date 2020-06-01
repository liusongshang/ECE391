/* init_paging.c - Fuctions to initialize paging , create a page directory and a page table with valid PDEs and PTEs.
*/
#include"init_paging.h"
#include"lib.h"

//uint32_t arr_paging_table[NUM_0F_PTE] __attribute__((aligned(DIRECTORY_TABLE_BOUNDARY)));
//uint32_t arr_paging_directory[NUM_OF_PDE] __attribute__((aligned(DIRECTORY_TABLE_BOUNDARY)));
/*  init_paging()
 *  Description: Initialize the paging directory, paging table. set kernel page and video memory page
 * Input:none
 * Output:none
 * Side effect: fill the array of paging directory and page table
 */
void init_paging(){
    init_paging_table();
    init_paging_directory();
    // Build the 4MB kernel page
    uint32_t ker_page = arr_paging_directory[1];
    ker_page |= BITMASK_P;  // mark as present
    ker_page |= BITMASK_PS; // mark as 4M pde
    ker_page |= START_ADDR_KERNEL;
    arr_paging_directory[1] = ker_page;
    
    // Build the 4KB video memory page 
    uint32_t vid_page = 0 | BITMASK_RW;
    vid_page |= BITMASK_P;
    vid_page |= START_ADDR_VIDEO;
    arr_paging_table[VIDEO_INDEX] = vid_page;
    // Set the entrance to PT   i.e pde
    uint32_t curr_pde = arr_paging_directory[0];
    curr_pde |= BITMASK_P;
    curr_pde |= ((uint32_t)(arr_paging_table));
    arr_paging_directory[0] = curr_pde;   
    // Set corresponding control registers. 
    set_up_terminal_pages();
    set_control_regs();
}

/*  init_paging_directory 
 *  Description: Initialize the paging directory, 0-4 MB is broken into 4KB pages with video memory inside
 * 4-8 MB is a single 4MB kernel page
 * Input:none
 * Output:none
 * Side effect: fill the array of paging directory, set R/W to 1
 */
void init_paging_directory(){
    int i;
    uint32_t curr_pde = 0 | BITMASK_RW;
    for(i = 0;i<NUM_OF_PDE;i++){
        arr_paging_directory[i] = curr_pde;
    }
}
/*  init_paging_table
 *  Description: Initialize the paging table, 0-4 MB is broken into 4KB pages with video memory inside
 * 4-8 MB is a single 4MB kernel page
 * Input:none
 * Output:none
 * Side effect: fill the array of paging table
 */
void init_paging_table(){
    int i;
    uint32_t curr_pte = 0 | BITMASK_RW; // set r/w to 1
    for(i = 0;i<NUM_0F_PTE;i++){
        arr_paging_table[i] = curr_pte;
    }

}
/*  set_control_regs
 *  Description: set the control registers to get to the memory
 * Input:none
 * Output:none
 * Side effect: change cr0, cr3 and cr4
 */
    
void set_control_regs(){
    asm volatile( 
         
            // load PD address to the cr3
            "movl    %0, %%eax;"
            "movl    %%eax,  %%cr3;"
            // enable 4MB paging, which is bit4 of cr4
            "movl   %%cr4,%%eax;"
           "orl    $0x00000010,%%eax;"
            "movl    %%eax,  %%cr4;"
            // enable paging, which is bit31 of cr0
            "movl   %%cr0,%%eax;"
            "orl   $0x80000000,%%eax;"
            "movl    %%eax,%%cr0;"
            :
            : "r"(arr_paging_directory)
            : "eax"
    );
}
/*  map_process_to_phys
 *  Description: set the paging of process (4MB page)
 * Input:process_id
 * Output:none
 * Side effect: fill paging directory
 */
void map_process_to_phys(uint32_t process_id){
    uint32_t curr_pde = 0;
    curr_pde |= BITMASK_RW; //read&write
    curr_pde |= BITMASK_P;  //present
    curr_pde |= BITMASK_US; //user level
    curr_pde |= BITMASK_PS; // 4mb page
    curr_pde |= EIGHT_MB+FOUR_MB*process_id;    // 8MB +4MB*process id
    arr_paging_directory[START_ADDR_VIRTUAL/FOUR_MB] = curr_pde;
}
/* flush_TLB
 * Description: flush the TLB
 * Input:none
 * Output:none
 * return: none
 */
void flush_TLB(){
    asm volatile(
        "movl %%cr3,%%eax;"
        "movl %%eax,%%cr3;"
        :
        :
        :"eax"
    );
}

/*----------------------------------- Helper function for checkpoint 3.5 Scheduling-------------------------- */
/* set_up_terminal_pages
 * Description:  Set up pages to map terminal video memory to physical. For now, initialize them to backstore
 * Input:none
 * Output:none
 * return: none
 */
void set_up_terminal_pages(){
    uint32_t curr_pte = 0 | BITMASK_P | BITMASK_RW;
    curr_pte |= START_ADDR_VIDEO + FOUR_KB_SIZE;
    arr_paging_table[VIDEO_INDEX+1] = curr_pte; // first terminal

    curr_pte = 0 | BITMASK_P | BITMASK_RW;
    curr_pte |= START_ADDR_VIDEO+2*FOUR_KB_SIZE;
    arr_paging_table[VIDEO_INDEX+2] = curr_pte; //second terminal
   
    curr_pte = 0 | BITMASK_P | BITMASK_RW;
    curr_pte |= START_ADDR_VIDEO+3*FOUR_KB_SIZE;
    arr_paging_table[VIDEO_INDEX+3] = curr_pte; //third terminal
    flush_TLB();
}
/* switch_terminal_paging   (WRONG FUCTION, ABANDONED!)
 * Description:  Set up pages to map terminal video memory to physical. For now, initialize them to backstore
 * Input:none
 * Output:none  
 * return: none
 */
void switch_terminal_paging(int term,int curr_term){
// change the current video memory page to the corresponding inactive terminal page
    uint32_t curr_pte = 0 | BITMASK_P | BITMASK_RW |BITMASK_US;
    curr_pte |= START_ADDR_VIDEO+curr_term*FOUR_KB_SIZE;
    arr_paging_table[VIDEO_INDEX+curr_term] = curr_pte;
    flush_TLB();

 
    curr_pte = 0|BITMASK_P|BITMASK_RW| BITMASK_US|START_ADDR_VIDEO;
    arr_paging_table[VIDEO_INDEX+term] = curr_pte;
    flush_TLB();
}   
