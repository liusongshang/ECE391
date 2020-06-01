#include "file_system.h"

/* init_filesystem
 * DESCRIPTION:use actual address of the file system to initialize filesystem
 * INPUT: addr:address of the file system
 * OUTPUT: none
 * RETURN VALUE: none
 */
void init_filesystem(uint32_t addr){
    boot_block=(boot_block_t*)addr;
}

/*
 * read_dentry_by_name
 *   DESCRIPTION: reads dir_entry by name in file system, store the corresponding dentry into parameter.
 *   INPUTS:fname:file name
            dentry:a directory entry to be filled with file name,file type ,inode number
 *   OUTPUTS:none
 *   RETURN VALUE:0 for success
 *				  -1 for failure
 *   SIDE EFFECTS: copy directory entry inyo dentry
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    int name_length;
    int i;
 //   dentry_t cur_dentry;
    /*return -1 on failure, indicate a non-existent file or invalid pointer*/
    
    if((fname==NULL)){
        return -1;
    }//printf("fname not null");
    name_length=strlen((const char*)fname);
    if(name_length>max_num_character){
        name_length=max_num_character;
    }
    /*use strncmp to compare two strings, if they are same in first name_length bits,they will return 0*/
    for(i=0;i<max_num_files;i++){
        if(strncmp((int8_t*)boot_block->dir_entries[i].file_name,(int8_t*)fname,max_num_character)==0){
           strncpy((char*)dentry->file_name,(char*)boot_block->dir_entries[i].file_name,name_length);
            // if(i>=boot_block->num_dir){
            // return -1;
            // }
			dentry->file_type=boot_block->dir_entries[i].file_type;
			dentry->inode_num=boot_block->dir_entries[i].inode_num;
            /*reutn 0 for success, no need to copy reserved*/
			return 0;
        }
    }
    //dentry->file_type=boot_block->dir_entries[0].file_type;
    /*not find corresponding name in directory, return -1*/
    return -1;
}


/*
 * read_dentry_by_index
 *   DESCRIPTION: reads dir_entry by index in file system, store the corresponding dentry into  the paraemeter
 *   INPUTS:fname:file name
            dentry:a directory entry to be filled with file name,file type ,inode number
 *   OUTPUTS:none
 *   RETURN VALUE:0 for success
 *				  -1 for failure
 *   SIDE EFFECTS: copy directory entry inyo dentry
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){
    int i;
    /*return -1 on failure, indicate a  invalid index or invalid pointer*/
    if((index>boot_block->num_dir)||(index<0)||(dentry==NULL)){
        return -1;
    }
    if(index==boot_block->num_dir){
        return 0;
    }
    /*copy name, type, inode_num to dentry*/
    for(i=0;i<max_num_character;i++){
    dentry->file_name[i]=boot_block->dir_entries[index].file_name[i];   
    }
    dentry->file_type=boot_block->dir_entries[index].file_type;
	dentry->inode_num=boot_block->dir_entries[index].inode_num;
    return 1;
}


/*
 *  read_data
 *   DESCRIPTION: using inode to read the length bytes file data from offset and put into buffer 
 *   INPUTS:inode: inode of file to be read
 *			 offset:start from position offset in the file
 *			 buf:place bytes read into buffer
 *			 length:number of bytes to be read
 *   OUTPUTS: none
 *   RETURN VALUE: num of bytes read for success
 *				   -1 for failure
 *   SIDE EFFECTS:read date from file and place into buf
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    inode_t* inode_ptr;
    uint8_t* data_ptr;
    int32_t i;
    int32_t start_block_idx;
    int32_t data_block_index;
    uint32_t data_internal_index;

    data_block_index=offset/block_size;
    data_internal_index=offset%block_size;
     /*return -1 on failure, indicate a invalid inode or invalid pointer*/
    if(inode>=boot_block->num_inodes){
        return -1;
    }
   
    /*get the pointer to the inode,+1 because of boot block,which is 4kb*/
    inode_ptr=(inode_t*)(boot_block)+inode+1;
  
    start_block_idx = (int32_t)inode_ptr->data_block[data_block_index];
    /*get the pointer to the first data to read*/
    
    data_ptr=(uint8_t*)boot_block+(1+start_block_idx+boot_block->num_inodes)*block_size+data_internal_index;
    for(i=0;i<length;i++){
        if((i+offset)>=inode_ptr->length_in_B){
            return i;
        }
        /*write into buf and turn to next data and add the internal offset as block counter*/
        buf[i]=*data_ptr;
        data_internal_index+=1;

        if(data_internal_index%block_size==0){
            data_block_index+=1;
            /*check bad block*/
            start_block_idx = (int32_t)inode_ptr->data_block[data_block_index];
            if(data_block_index>=boot_block->num_data_blocks){
                return -1;
            }
			data_ptr=(uint8_t*)boot_block+(1+start_block_idx+boot_block->num_inodes)*block_size;
            }
        else{
            data_ptr+=1;
            }
        }
    return i;
    //return 1;
    }

/*
 *  check_magic
 *   DESCRIPTION: 1. Check ELF magic constant 2.Find the first instruction’s address
 *   INPUTS: fname-- file name of executeable
 *   OUTPUTS: none
 *   RETURN VALUE: entry point address for success
 *				   -1 for failure
 *   SIDE EFFECTS:copy file contents to physical address
 */
// int32_t check_magic(const uint8_t* fname){
//     dentry_t tdentry;
//     uint8_t buf[40];
//     int32_t ret;
//     ret = read_dentry_by_name(fname,&tdentry);
//     if(ret!=0 || tdentry.file_type!=FILE_TYPE){ 
//         return -1;
//     }
//     ret = read_data(tdentry.inode_num,0,buf,40);
//     if(ret==-1){
//         return -1;
//     }
//     // firstly, check elf magic number
//     if(buf[0]!=MAGIC_0 || buf[1]!=MAGIC_1 || buf[2]!=MAGIC_2||buf[3]!=MAGIC_3){
//         printf("Wrong magic number.\n");
//         return -1;
//     }
//      return 0;  
// }
/*
 *   get_entry_point
 *   DESCRIPTION: Find the first instruction’s address
 *   INPUTS: fname-- file name of executeable
 *   OUTPUTS: none
 *   RETURN VALUE: entry point address for success
 *				   -1 for failure
 *   SIDE EFFECTS:none
 */
int32_t get_entry_point(const uint8_t* fname){
    dentry_t tdentry;
    uint8_t buf[ENTRY_BUFFER_SIZE];
    int32_t ret;
    uint32_t entry_point;
    ret = read_dentry_by_name(fname,&tdentry);
    if(ret!=0 || tdentry.file_type!=FILE_TYPE){ 
        return -1;
    }
    ret = read_data(tdentry.inode_num,0,buf,ENTRY_BUFFER_SIZE);
    if(ret==-1){
        return -1;
    }
    entry_point = buf[ENTRY_0] + (buf[ENTRY_1]<<BYTE_SIZE) + (buf[ENTRY_2]<<2*BYTE_SIZE) + (buf[ENTRY_3]<<3*BYTE_SIZE);
    return entry_point;  
}
/*
 *  program_loader
 *   DESCRIPTION: copy file image to physics address
 *   INPUTS: fname-- file name of executeable
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *				   -1 for failure
 *   SIDE EFFECTS:copy file contents to physical address
 */
int32_t program_loader(const uint8_t*fname){
    dentry_t tdentry;
    int32_t ret;
    ret = read_dentry_by_name(fname,&tdentry);
    if(ret==-1){
        return -1;
    }
    read_data(tdentry.inode_num, 0, (uint8_t*)(START_ADDR_VIRTUAL+PROGRAM_OFFSET), FOUR_MB);
    return 0;
}
/*
 *  file_open
 *   DESCRIPTION: open the file, initilalize the temp dentry
 *   INPUTS: filename -- the filename of the file to be opened
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *				   -1 for failure
 *   SIDE EFFECTS:use read_dentry_by_name to store the file info into judge
 */
int32_t file_open(const uint8_t *filename){
    int32_t ret;
    ret=read_dentry_by_name(filename,&judge);    // 0 for success
    if(judge.file_type!=FILE_TYPE){
        //printf("Can't open target. Target is not a file.\n");
        return -1;
    }
    if(ret!=0){
        printf("open file failed\n");
        return -1;
    }
    return ret;
}
/*
 *  file_close
 *   DESCRIPTION: close the file
 *   INPUTS: filename -- the filename of the file to be closed
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *				   -1 for failure
 *   SIDE EFFECTS:none
 */
int32_t file_close(int32_t fd){
    return 0;
}
/*
 *  file_write
 *   DESCRIPTION: Since it's read-only, we do nothing
 *   INPUTS: filename
 *           buf
 *           nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: always return -1
 *   SIDE EFFECTS:none
 */
int32_t file_write(int32_t fd, const void *buf, int32_t nbytes){
    return -1;
}
/*
 *  file_read
 *   DESCRIPTION: use read_data to read nbytes into buf
 *   INPUTS: buf -- the buffer to store the read bytes
 *           nbytes -- how large we read into buffer
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *				   -1 for failure
 *   SIDE EFFECTS: fill the buf with nbytes of information
 */

int32_t file_read(int32_t fd,void *buf, int32_t nbytes){
     int32_t ret;
     int pid;
    // ret=read_dentry_by_name(filename,&judge);
    // if(ret==-1){
    //     return -1;
    // }
    // if(judge.file_type!=FILE_TYPE){
    //     return -1;
    // }
    //printf("sdads");
/////////////////////////////////////////////////////////////////////////
    // pid=terminal_running_pid(current_terminal_index);
    pid=terminal_running_pid(scheduled_terminal);
    pcb_t* pcb=get_pcb_by_pid(pid);
//////////////////////////////////////////////////////////////////////////
    ret=read_data(pcb->file_arr[fd].inode,pcb->file_arr[fd].f_pos,buf,nbytes);
    pcb->file_arr[fd].f_pos+=ret;
    return ret;

}

/*
 *   dir_open
 *   DESCRIPTION: open the directory, initilalize the temp counts
 *   INPUTS: filename -- the name of the file to be opened
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *				   -1 for failure
 *   SIDE EFFECTS: store the file's name into buf
 */
int32_t dir_open(const uint8_t *filename){
    int32_t ret;
    file_count=0;
    ret=read_dentry_by_name(filename,&judge);
    if(judge.file_type!=DIR_TYPE){
        return -1;
    }
    if(ret!=0){
        printf("open unsucessfully\n");
    }
    return ret;
}
/*
 *  dir_close
 *   DESCRIPTION: undo the open operation
 *   INPUTS: filename   -- directory to e closed
 *   OUTPUTS: none
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:none
 */
int32_t dir_close(int32_t fd){
    return 0;
}
/*
 *  dir_write
 *   DESCRIPTION: Since it's read-only, we do nothing
 *   INPUTS: filename
 *           buf
 *           nbytes
 *   OUTPUTS: none
 *   RETURN VALUE: always return -1
 *   SIDE EFFECTS:none
 */
int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes){
    return -1;
}
/*
 *  dir_read
 *   DESCRIPTION: use read_data to read nbytes into buf
 *   INPUTS: buf -- the buffer to store the read bytes
 *           nbytes -- how large we read into buffer -- normally 32 for name
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success
 *				   -1 for failure
 *   SIDE EFFECTS: fill the buf with nbytes of information
 */
int32_t dir_read(int32_t fd,void *buf, int32_t nbytes){
     /*max number of characters in name*/
    if(nbytes>max_num_character){
        nbytes=max_num_character;
    }
    int ret=read_dentry_by_index(file_count,&temp);
    file_count++;
    if(ret==-1){return -1;}
    if(ret==1){
        memcpy(buf,temp.file_name,nbytes);
        return strlen_mod(temp.file_name);
    }
    return 0;
}
/*
 *  strlen_mod
 *   DESCRIPTION: strlen to handler situation larger than32
 *   INPUTS: s- str
 *   OUTPUTS: none
 *   RETURN VALUE: len
 *   SIDE EFFECTS: strlen to handler situation larger than32
 */
int32_t
strlen_mod(const uint8_t* s)
{
	register uint32_t len = 0;
	while(s[len] != '\0' && len < max_num_character)
		len++;
	return len;
}
