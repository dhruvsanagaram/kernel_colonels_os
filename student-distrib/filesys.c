#include "filesys.h"

#define SUCCESS 0
#define FAILURE 1


//PTR to filesystem's base address

superblock_t *boot_base_addr;
inode_t* inode_start_ptr;
dentry_t* dentry_start_ptr;
uint32_t* data_block_ptr;
uint32_t file_pos_in_dir;
/* 
  * init_fs
  * inputs: unsigned int filesys_addr -- address of base of filesystem
  * outputs: n/a
  * return val: status flag (success: 0 if fs mounted properly, -1 else)
  * effects: set up the filesystem's data structure
  * STATUS: works
*/
int32_t init_fs(unsigned int filesys_addr){
  int i;
  boot_base_addr = (superblock_t*)filesys_addr;
  uint32_t num_inodes = boot_base_addr->inode_ct;
  dentry_start_ptr = boot_base_addr->dentries;
  inode_start_ptr = (inode_t*)(boot_base_addr + 1);
  data_block_ptr = (uint32_t*)(inode_start_ptr + num_inodes);

  //initialize file descriptor table for bookkeeping
  //no need for this ^^^^^
  // for(i=0; i < MAX_NUM_FILES; ++i){
  //   fd_arr[i].inode_num = -1; //fdarr not in use yet
  //   fd_arr[i].position = 0; //initial position
  // }
  return SUCCESS; //filesys properly intialize
}
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////             DIR SYSCALLS            //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
int32_t dir_open(const uint8_t *filename){
  dentry_t the_dentry; //the dentry that represents the directory containing the file
  if(read_dentry_by_name(filename, &the_dentry) == -FAILURE){
    return -FAILURE; //file does not exist
  }
  file_pos_in_dir = 0;
  return SUCCESS; //file found;
}


int32_t dir_close(int32_t fd){
  return SUCCESS; //exit out
}


///// PREV IMPL ////
// int32_t fd_idx;
  // int32_t bytes_name_to_cpy;
  // dentry_t dentry_read; //the dentry to read based on calced pos
  // if(buf == NULL || nbytes == 0){
  //   return -FAILURE;
  // }
  // //check for valid fd
  // if(fd < 0 || fd >= MAX_NUM_FILES){
  //   return -FAILURE;
  // }

  // //note current position of the file descriptor in the bookkeeping tbl
  // fd_idx = fd_arr[fd].position;
  // if (read_dentry_by_index(fd_idx, &dentry_read) == -FAILURE){
  //   return SUCCESS; //end of dir reached
  // }

  // //Copy the filename into the buffer
  // bytes_name_to_cpy = nbytes < FILENAME_LEN ? nbytes : FILENAME_LEN;
  // strncpy((int8_t*)buf, (int8_t*)&dentry_read.filename, bytes_name_to_cpy);

  // //null-terminate the string to cap extra space
  // if(bytes_name_to_cpy < nbytes){
  //   ((char*)buf)[bytes_name_to_cpy] = '\0';
  // }

  // //go to next position in the dir
  // fd_arr[fd].position++;
  // return bytes_name_to_cpy;

int32_t dir_read(int32_t fd, void *buf, int32_t nbytes){
  dentry_t the_dentry;
  int32_t bytes_to_copy;
  int len_filename;
  if(buf == NULL || nbytes == 0){
    return -FAILURE;
  }
  if(read_dentry_by_index(file_pos_in_dir, &the_dentry) == -FAILURE){
    // file_pos_in_dir = 0;
    return 0; //the directory couldn't be read so no bytes were read because the dentry wasnt found/read
  }
  len_filename = strlen((int8_t*)the_dentry.filename);
  if(len_filename > FILENAME_LEN){
    len_filename = FILENAME_LEN;
  }
  //Copy the filename into the buffer after we clipped
  bytes_to_copy = nbytes < len_filename ? nbytes : len_filename;
  strncpy((int8_t*)buf, (int8_t*)the_dentry.filename, bytes_to_copy);

  //Null terminate the string if space is avail
  if(bytes_to_copy < nbytes){
    ((char*)buf)[bytes_to_copy] = '\0';
  }

  
  file_pos_in_dir++;
  return bytes_to_copy;
}

int32_t dir_write(int32_t fd, const void *buf, int32_t nbytes){
  return -FAILURE;
  ////DIRECTORIES ARE READ ONLY////
}



////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////           FILE SYSCALLS             //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

int32_t file_open(const uint8_t *filename){
  dentry_t the_dentry; //the dentry that represents the directory containing the file
  int32_t idx = read_dentry_by_name(filename, &the_dentry);
  if(idx == -FAILURE){
    return -FAILURE; //file does not exist
  }
  return idx; //file found;
}

int32_t file_close(int32_t fd){
  return SUCCESS;
}

/// PREV IMPL ///
// if (fd < 0 || fd >= MAX_NUM_FILES || nbytes < 0 || fd_arr[fd].inode_num == -1){
//       return -FAILURE; //Invalid 
//   }
//   file_d_t *file_d = &fd_arr[fd];
//   int32_t byte_r = read_data(file_d->inode_num, file_d->position, buf, nbytes);
//   if(byte_r <= 0){
//     return -FAILURE;
//   }
//   file_d->position += byte_r; //Update
//   return byte_r;

int32_t file_read(int32_t fd, void *buf, int32_t nbytes){
  //do something with read_data for file
  int32_t bytes_to_read;
  int32_t bytes_actually_read;
  if(fd < 0 || nbytes < 0 || buf == NULL){
    return -FAILURE;
  }
  dentry_t the_dentry;
  if (read_dentry_by_index(fd, &the_dentry) == -FAILURE){
    return -FAILURE;
  }
  // printf("FD index: %d\n", fd);
  // printf("dentry inode_num: %d\n", the_dentry.inode_num);
  // printf("the original: %d\n", boot_base_addr->dentries[fd].inode_num);

  inode_t* inode = &inode_start_ptr[the_dentry.inode_num];
  if(file_pos_in_dir >= inode->len){
    return 0; //EOF has been hit
  }

  //Calculate the number of bytes to read
  bytes_to_read = nbytes;
  if(file_pos_in_dir + nbytes > inode->len){
    bytes_to_read = inode->len - file_pos_in_dir; //Adjust bytes that will be read if file size exceeded
  }

  //Pull data from file
  // printf("inode_number: %d\n", the_dentry.inode_num);
  // printf("file position: %d\n", file_pos_in_dir);
  bytes_actually_read = read_data(the_dentry.inode_num, file_pos_in_dir, buf, bytes_to_read);
  if(bytes_actually_read < 0){
    return -FAILURE;
  }
  file_pos_in_dir += bytes_actually_read;
  return bytes_actually_read;
}

int32_t file_write(int32_t fd, const void *buf, int32_t nbytes){
  return -FAILURE; //fails as this is a read only file system
}


/////////////////////// API FUNCS ///////////////////////////// 

/* 
  * read_dentry_by_name
  * Description: searches the system from dentry fname
  * inputs:  fname -- the directory filename
             dentry -- pointer to the dentry object
  * outputs: n/a
  * return val: status flag (success: 0 if fs mounted properly, -1 else)
  * effects: set up the filesystem's data structure
  * STATUS: WORKS for fname in bound
*/
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry){
  if(fname == NULL){
    return -FAILURE;
  }
  int len_filename = strlen((int8_t*)fname);
  //check if filename is between 32 and 0 chars
  if(len_filename > 32 || len_filename <= 0){
    return -FAILURE;
  }
  int i;
  for(i = 0; i < SUPERBLOCK_SIZE; ++i){
    ////////>>>>>>>>>> 10-22-2023:: page fault happens while we are here at 182
    uint8_t dentry_len = strlen((int8_t*)dentry_start_ptr[i].filename);
    /////// <<<<<<<<<< reason: i did not set the FS address so dentry_start_ptr
    /////// <<<<<<<<<<         so this pointed to NULL.
    /////// <<<<<<<<<< FIX: set a filesys addr mod in kernel.c
    if(dentry_len >= FILENAME_LEN){
      //clip down to 32 chars for security
      dentry_len = FILENAME_LEN;
    }
    if(dentry_len == len_filename){
      if(strncmp((int8_t*)fname, (int8_t*)dentry_start_ptr[i].filename, len_filename) == 0){
        if (read_dentry_by_index(i, dentry) == -FAILURE){
          return -FAILURE;
        }
        return SUCCESS; //we finished reading
      }
    }
  }
  return -FAILURE; //filename not found
}


// STATUS: works for fname in bound
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry){
  if (index >= NUM_DENTRIES_BOOT) {
    return -FAILURE;// out of bounds or dentry null
  }
  *dentry = boot_base_addr->dentries[index]; //set eq
  return SUCCESS; // works
}


/* 
  * read_data
  * Description: reads given data from a file's inode indx
  * inputs: inode -- index/inode number to read data from
            offset -- offset from beginning marker to read to EOF
            buf -- pointer to buffer to read data into
            length -- number of bytes to read
  * outputs: n/a
  * return val: status flag (success: 0 if fs mounted properly, -1 else)
  * effects: set up the filesystem's data structure

*/

// read data from the file given inode num //
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    uint32_t num_bytes_read = 0; //counter for number of bytes read
    uint32_t datablock_curr = offset / EXT2_DATA_BLOCK_SIZE;
    uint32_t byte_curr = offset % EXT2_DATA_BLOCK_SIZE; //the current byte to read
    uint32_t buf_idx = 0; //current index in buf we are writing to after read
    uint32_t NUM_INODES = boot_base_addr->inode_ct; //number of inodes in the system
    uint32_t NUM_DATABLOCKS = boot_base_addr->data_block_ct; //number of data blocks in the system
    
    int i;
    if (inode >= NUM_INODES || inode < 0) {
        return -FAILURE; // invalid
    }
    //inode_t* curr_inode = (inode_t*)(inode_start_ptr + inode);
    inode_t* curr_inode = &inode_start_ptr[inode];
    if( offset >= curr_inode->len){
      return -FAILURE; // invalid
    }
    if (offset + length > curr_inode->len) {
        length = curr_inode->len - offset; // bytes read
    }


    // uint32_t num_bytes_read; //counter for number of bytes read
    // uint32_t byte_curr; //the current byte to read
    // int i;
    for(i=0; i < length; ++i){
      
      if(i + offset >= curr_inode->len){
        return num_bytes_read; //reached EOF
      }
      if(byte_curr >= EXT2_DATA_BLOCK_SIZE){
        byte_curr = 0; // reset cur byte in that file, reset
        datablock_curr++; //go to the next datablock
      }
      uint32_t data_block_num = curr_inode->data_block_num[datablock_curr];
      if(data_block_num >= NUM_DATABLOCKS){
        return -FAILURE; // invalid
      }

      /////// THERE IS A PROBLEM IN BLOCK CALC
      uint8_t* block = (uint8_t*)(data_block_ptr + data_block_num * EXT2_DATA_BLOCK_SIZE);
      // buf[buf_idx] = block[block_offset]; 
      memcpy(&buf[buf_idx], &block[byte_curr], 1);
      buf_idx++; byte_curr++; num_bytes_read++;
    }

    return num_bytes_read;
}

////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////             TESTS                ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

int32_t dir_test(){
	uint8_t empty_filename;
  int32_t empty_fd;
  int32_t empty_nbytes;
  uint8_t buf[FILENAME_LEN];
  int i;
  int file_i;
  dentry_t* cur_d;
  inode_t* cur_inode;
	dir_open(&empty_filename);
	uint32_t dentry_num = boot_base_addr->dentry_ct;
  for(i=0; i < dentry_num; i++){
    cur_d = (dentry_t*)&(boot_base_addr->dentries[i]);
    cur_inode = (inode_t*)(inode_start_ptr + (cur_d->inode_num));
    int32_t size = cur_inode->len;
    dir_read(empty_fd,buf,empty_nbytes);
    printf("%d - name: ", i);
    for(file_i = 0; file_i < FILENAME_LEN; file_i++){
      if(buf[file_i] != '\0'){
        putc(buf[file_i]);
      } else {
        putc(' ');
        break;
      }
    }
    printf("filesize: %d\n", size);
  }
  dir_close(empty_fd);
  return 0;
}

int32_t fileRead_Test(){
  int32_t fileIdxInDir = 10;
  uint8_t filename[32] = "frame0.txt";
  int numReadBytes = 0;
  int numTotalBytes = 0;
  uint8_t buf[4096];
  int i;

  if(file_open((uint8_t*)filename) == -FAILURE){
    // printf("This file DNE\n");
    return SUCCESS;
  }
  numReadBytes = file_read(fileIdxInDir, buf, 4096);

  while(1){
    numReadBytes = file_read(fileIdxInDir, buf, 4096);
    i = 0;
    for(i = 0; i < numReadBytes; i++){
      if(buf[i] != '\0'){
        putc(buf[i]);
      }
    }
    numTotalBytes += numReadBytes;
    printf("Num Bytes Read: %d\n", numReadBytes);

    if(numReadBytes == 0){
      printf("EOF\n");
      break;
    }
  }
  // printf("Num Total Bytes: %d\n", numTotalBytes);

  if(file_write(fileIdxInDir, buf, numReadBytes) != -FAILURE){
    return -FAILURE;
  }

  file_close(0);
  return SUCCESS;
}

int32_t readTextFile(){
  int32_t fileIdxInDir = 10; //the fileindx of frame0.txt
  uint8_t filename[32] = "frame0.txt";
  int numReadBytes;
  uint8_t buf[187];
  if(file_open((uint8_t *)filename) == -FAILURE){
    printf("Cannot read file :/");
    return -FAILURE;
  }
  numReadBytes = file_read(fileIdxInDir, buf, 187);
  printf("Num bytes read: %d\n", numReadBytes);
  int i;
  for(i = 0; i < 187; i++){
    putc(buf[i]);
  }
  printf("\nframe0.txt\n");
}
