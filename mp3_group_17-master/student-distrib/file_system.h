#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include "lib.h"
#include "types.h"
#include "keyboard.h"
#include "init_paging.h"
#include "pcb.h"
#include "schedule.h"
#include "pit.h"
/*Each directory entry gives a name (up to 32 characters, zero-padded*/
#define max_num_character 32
/*for every directory entry, there are 24 B reserved at last*/
#define reserved_length_in_dentry 24
/*for boot block, there are 52 B reserved*/
#define reserved_length_in_boot_block 52
/*boot block is 4kb, both statistics and each directory entry occpy 64B, so 4*1024/64-1=63*/
#define max_num_files 63
/*inode is 4 kb and first 4 B used to store length, so (4*1024-4)/4=1023*/
#define max_num_data_block 1023
/*block size is 4kb, which is also 4096 bytes*/
#define block_size 4096
#define RTC_TYPE 0
#define DIR_TYPE 1
/*it's number for file*/
#define FILE_TYPE 2
#define MAGIC_0	0x7f
#define MAGIC_1	0x45
#define MAGIC_2	0x4c
#define MAGIC_3 0x46
#define	ENTRY_0	24
#define	ENTRY_1 25
#define	ENTRY_2 26
#define	ENTRY_3 27
#define BYTE_SIZE	8
#define	PROGRAM_OFFSET 0x00048000
#define ENTRY_BUFFER_SIZE	40

/*define struct of directory entry*/
typedef struct dentry_t{
	uint8_t file_name[max_num_character];
	uint32_t file_type;
	uint32_t inode_num;
    uint8_t reserved[reserved_length_in_dentry];
}dentry_t;


/*define struct of boot block */
typedef struct boot_block_t{
	uint32_t num_dir;	//
	uint32_t num_inodes;
	uint32_t num_data_blocks;
	uint8_t reserved[reserved_length_in_boot_block];
	dentry_t dir_entries[max_num_files];
}boot_block_t;

/*define struct of inode*/
typedef struct inode_t{
    int32_t length_in_B;
    int32_t data_block[max_num_data_block];
}inode_t;


boot_block_t *boot_block;
dentry_t judge;
/*to count how many directory we have read*/
int file_count; 
dentry_t temp;

/*initialize the file system*/
void init_filesystem(uint32_t addr);

/*three routines for file system*/
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

/* function about loading program*/
// int32_t check_magic(const uint8_t*fname);
int32_t program_loader(const uint8_t* fname);
int32_t get_entry_point(const uint8_t* fname);
/*file drivers*/
int32_t file_open(const uint8_t *filename);
int32_t file_close(int32_t fd);
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes);
int32_t file_read(int32_t fd,void *buf, int32_t nbytes);

/*dir drivers*/
int32_t dir_open(const uint8_t *filename);
int32_t dir_close(int32_t fd);
int32_t dir_write(int32_t fd,const void *buf, int32_t nbytes);
int32_t dir_read(int32_t fd,void *buf, int32_t nbytes);
int32_t strlen_mod(const uint8_t* s);
extern int32_t current_terminal_index;
#endif
