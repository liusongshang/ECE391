#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "rtc.h"
#include "init_paging.h"
#include "keyboard.h"
#include "file_system.h"
#include "systemcall.h"
#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// int zero_test(){
// 	TEST_HEADER;
// 	int i=1/0;
// 	return i;
// }

/* deref_null_test
 * 
 * dereference address NULL, shoudld throw exception page fault
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: Exception handler
 * Files: init_paging.h/c
 */
 int deref_null_test(){
	TEST_HEADER;
	int *a = NULL;
	int b = *(a);
	return b;
}
/* deref_nonexist_page_test
 * 
 * dereference nonexist address, shoudld throw exception page fault
 * Inputs: None
 * Outputs: should throw exception page fault
 * Side Effects: None
 * Coverage: Exception handler
 * Files: init_paging.h/c
 */
 int deref_nonexist_page_test(){
	TEST_HEADER;
	int *a = (int*)(START_ADDR_VIDEO - 16);	// check 0-4MB not in video memory randomly
	int b = *a;
	return b;
}
/* deref_video_page_test
 * 
 * dereference video memory address, expect pass
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: if wrong, exception will be thrown
 * Coverage: Exception handler, video memory page
 * Files: init_paging.h/c
 */
int deref_video_page_test(){
	TEST_HEADER;
	int i;
	int d;
	for(i=0;i<=DIRECTORY_TABLE_BOUNDARY-4;i++){
		int *c = (int*)(START_ADDR_VIDEO+i);
		d = *c;
	}
	return d;
}
/* deref_kernel_page_test
 * 
 * dereference kernel memory address, expect pass
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: if wrong, exception will be thrown
 * Coverage: Exception handler, kernel page
 * Files: init_paging.h/c
 */
int deref_kernel_page_test(){
	TEST_HEADER;
	int i;
	int d;
	for(i=0;i<=START_ADDR_KERNEL-4;i++){
		int *c = (int*)(START_ADDR_KERNEL+i);
		d = *c;
	}
	return d;
}
/* check_page_struct
 * 
 * check the directory's present value and page address
 * Inputs: None
 * Outputs: PASS/FAIL, expect pass
 * Side Effects: NONE
 * Coverage: paging directory, paging table
 * Files: init_paging.h/c
 */
// int check_page_struct(){

// 	if((arr_paging_directory[0] & BITMASK_P)!=1 ){
// 		printf("first PDE is not present");
// 		return FAIL;
// 	}
// 	if((arr_paging_directory[1] &  BITMASK_P) !=1){
// 		printf("second PDE is not present");
// 		return FAIL;
// 	}
// 	if((arr_paging_directory[1] & ALIGNED_CORRECTED) != START_ADDR_KERNEL){
// 		printf("wrong address of kernel");
// 		return FAIL;
// 	}
// 	if((arr_paging_directory[0] & ALIGNED_CORRECTED) != (uint32_t)arr_paging_table){
// 		printf("wrong address of page table");
// 		return FAIL;
// 	}
// 	return PASS;
// }

// add more tests here

/* Checkpoint 2 tests */

/* rtc_test
 * 
 * test rtc at each possible frequency
 * Inputs: None
 * Outputs: PASS/FAIL, expect pass
 * Side Effects: NONE
 * Coverage: rtc_open, rtc_write
 */
 int rtc_test(){
	int freq = 2;
	int i;
	if(0 == rtc_open(NULL))
		printf("rtc_open success\n");
	while(freq <= DEFAULT_FREQ){
		for(i=0;i<freq;i++){
			rtc_read(0,NULL,0);
			printf("1");
		}
		printf("\nThe rtc frequency now is: %d",freq);
		freq = freq *2;
		rtc_write(0,(void*)&freq,4);			// 4 is the number of bytes
		printf("\n");
	}							
	if(0 ==rtc_close(0))
		printf("rtc_close success");
	return PASS;
}

/* terminal_read_test
 * 
 * test whether terminal can read from the keyboard buffer
 * Inputs: None
 * Outputs: PASS/FAIL, expect pass
 * Side Effects: NONE
 * Coverage: terminal_read
 */
int terminal_read_test(){
	unsigned char buff[line_buffer_size];
	unsigned char buff2[7];
	int i;
	if( 0 == terminal_open(NULL))
		printf("terminal_open success\n ");
	i=terminal_read(0,buff,columns_number);
	printf("Reading number: %d  \n",i);
	printf("Reading content: %s",buff);
	i=terminal_read(0,buff2,5);
	printf("Reading number: %d  \n",i);
	printf("Reading content: %s",buff2);
	if( 0 == terminal_close(0))
		printf("terminal_close success");
	return PASS;

}

/* terminal_write_test
 * 
 * test whether terminal write a buffer to the screen
 * Inputs: None
 * Outputs: PASS/FAIL, expect pass
 * Side Effects: NONE
 * Coverage: terminal_write
 */
int terminal_write_test(){
	char* buff;
	char* buff2;
	int i;
	buff = "hello" ;
	i = terminal_write(0,buff,5);
	printf("successfully write %d characters\n",i);
	if(i != 5)
		return FAIL;
	printf("\n");
	buff2 = "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh" ;
	i = terminal_write(0,buff2,130);
	printf("successfully write %d characters\n",i);
	if(i!=line_buffer_size)
		return FAIL;
	char* buff3;
	buff3 = "aaa";
	terminal_write(0,buff3,6);
	return PASS;
}

/* dir_read_test_ls
 * 
 * print out all files in the directory
 * Inputs: None
 * Outputs: PASS/FAIL, expect pass
 * Side Effects: NONE
 * Coverage: dir_open(),dir_read(), read_dentry_by_name(),
 * 			read_dentry_by_index()
 * Files: file_system.h/c
 */
int dir_read_test_ls(){
	int fd;
	int cnt;
	int i;
	int j;
	inode_t* inode_ptr;
 	const char * y="file_type:";
	uint8_t buf[max_num_character];		
	fd = dir_open((uint8_t*)".");
	if (fd!=0) {
		return FAIL;
	}

	for(j=0;j<boot_block->num_dir;j++){
		cnt = dir_read(fd,buf,max_num_character);
		if(cnt!=0){
			printf("Read failed");
			return FAIL;
		}
		if(0==cnt){
			printf("file_name: ");
		
			for(i=0;i<max_num_character;i++){
				putc(buf[i]);
			}
			printf("%s%u ",y,temp.file_type);
			
			printf("file_size: ");
			inode_ptr=(inode_t*)(boot_block)+temp.inode_num+1;
			printf("%u",inode_ptr->length_in_B);
			printf("\n");
		}
	}
	return PASS;
}

/* file_read_test_ls
 * 
 * print out the contents of file
 * Inputs: None
 * Outputs: PASS/FAIL, expect pass
 * Side Effects: NONE
 * Coverage: file_open(),file_read(), read_dentry_by_name(),
 * 			read_data()
 * Files: file_system.h/c
 */
//verylargetextwithverylongname
int file_read_test(){
	uint8_t buf[5277];	// size of verylarge....txt
	int fd;
	int i;
	fd=file_open((uint8_t*)"verylargetextwithverylongname.txt");
	if(fd!=0){
		return FAIL;
	}
	file_read(fd,buf,5277);

	for(i=0;i<5277;i++){
		putc(buf[i]);
	}
	// 	}
//printf(buf);
	return PASS;
}
/* read_file_by_idx
 * 
 * print out the contents of file according to the index
 * Inputs: idx
 * Outputs: PASS/FAIL, expect pass
 * Side Effects: NONE
 * Coverage: read_data(),read_dentry_by_name(),
 * 			
 * Files: file_system.h/c
 */
int read_file_by_idx(int idx){
	int fd;
	int i;
	int32_t size;
	inode_t* inode_ptr;
	dentry_t tempden;
	if(idx>=boot_block->num_dir){
		printf("incorrect index");
		return FAIL;
	}
	fd = read_dentry_by_name(boot_block->dir_entries[idx].file_name,&tempden);
	if(fd!=0){
		printf("read dentry by name fail");
		return FAIL;
	}
	if(tempden.file_type!=FILE_TYPE){
		printf("it's not a file");
		return FAIL;
	}
	inode_ptr = (inode_t*)(boot_block)+tempden.inode_num+1;
	size = inode_ptr->length_in_B;
	uint8_t buf[size];
	read_data(tempden.inode_num,0,buf,size);
	for(i=0;i<size;i++){
		putc(buf[i]);
	}
	return PASS;

}

/* Checkpoint 3 tests */
// int syscall_execute_test(){
// 	TEST_HEADER;
// 	char* cmd = "testprint";
// 	printf("command is %s\n",cmd);
// 	execute(cmd);
// 	return PASS;
// }

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */

/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("read_file_by_idx",read_file_by_idx(11));
	//TEST_OUTPUT("file_read_test",file_read_test());
	//TEST_OUTPUT("dir_read_test",dir_read_test_ls());
	//TEST_OUTPUT("zero_test", zero_test());
	//TEST_OUTPUT("deref null",deref_null_test());
	//TEST_OUTPUT("deref nonexist",deref_nonexist_page_test());
	//TEST_OUTPUT("deref video memory",deref_video_page_test());
	//TEST_OUTPUT("deref kernel",deref_kernel_page_test());
	//TEST_OUTPUT("check page structure",check_page_struct());
	//TEST_OUTPUT("rtc",rtc_test());
	//TEST_OUTPUT("terminal_read",terminal_read_test());
	//TEST_OUTPUT("terminal_write",terminal_write_test());
	// launch your tests here
}
