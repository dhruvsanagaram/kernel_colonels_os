#include "filesys.h"
#include "process.h"

#define SUCCESS 0
#define FAILURE 1


//PTR to filesystem's base address

superblock_t *boot_base_addr;
inode_t* inode_start_ptr;
dentry_t* dentry_start_ptr;
uint32_t* data_block_ptr;
// uint32_t file_pos_in_dir;
/* 
  * init_fs
  * inputs: unsigned int filesys_addr -- address of base of filesystem
  * outputs: n/a
  * return val: status flag (success: 0 if fs mounted properly, -1 else)
  * effects: set up the filesystem's data structure
  * STATUS: works
*/
int32_t init_fs(unsigned int filesys_addr){
  boot_base_addr = (superblock_t*)filesys_addr;
  printf("Num bytes in superblock: %d\n", sizeof(superblock_t));
  uint32_t num_inodes = boot_base_addr->inode_ct;
  dentry_start_ptr = (dentry_t*)(boot_base_addr->dentries);
  inode_start_ptr = (inode_t*)(boot_base_addr + 1); //boot_base_addr + size of the superblock (4kB)
  data_block_ptr = (uint32_t*)(inode_start_ptr + num_inodes);
  //initialize file descriptor table for bookkeeping

  return SUCCESS; //filesys properly intialize
}
////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////             DIR SYSCALLS            //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

/* 
  directory_open
  * inputs: unsigned int filename
  * outputs: n/a
  * return val: status flag (success: 0 if dir file opened, -1 else)
  * effects: opens up the directory file and prepares to read from the 
  *          directory from reading the first file in the directory
  * STATUS: works
*/
int32_t directory_open(const uint8_t *filename){
  dentry_t the_dentry; //the dentry that represents the directory containing the file
  if(read_dentry_by_name(filename, &the_dentry) == -FAILURE){
    return -FAILURE; //file does not exist
  }
  return SUCCESS; //file found;
}

/* 
  directory_close
  * inputs: unsigned int fd
  * outputs: n/a
  * return val: status flag (success: 0 if dir file closed, -1 else)
  * effects: close up the directory file
  * STATUS: works
*/
int32_t directory_close(int32_t fd){
  return SUCCESS; //exit out
}


/* 
  directory_read
  * inputs: unsigned int fd -- file descriptor
            buf -- buffer that we write to after reading
            nbytes -- number of bytes we read
  * outputs: n/a
  * return val: status flag (success: 0 if name read properly, -1 else)
  * effects: read the filename at the current position into buf
  * STATUS: works
*/

int32_t directory_read(int32_t fd, void *buf, int32_t nbytes){
  dentry_t the_dentry;
  int32_t bytes_to_copy;
  int len_filename;
  pcb_t* cur_pcb = getRunningPCB(); //check the current process
  int32_t file_pos_in_dir = cur_pcb->fd_arr[fd].fpos; //check file position
  // printf("Dir read called\n");
  //Check the validity of dir args
  if(buf == NULL || nbytes <= 0){
    return -FAILURE;
  }
  if(read_dentry_by_index(file_pos_in_dir, &the_dentry) == -FAILURE){
    return 0; //the directory couldn't be read so no bytes were read because the dentry wasnt found/read
  }

  len_filename = strlen((int8_t*)the_dentry.filename);

  //Truncation of filename so that we can work with stuff like verylargetextwithverylargename

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
  cur_pcb->fd_arr[fd].fpos = file_pos_in_dir; //update fileread position


  return bytes_to_copy;
}


/* 
  directory_write
  * inputs: unsigned int fd -- file descriptor
            buf -- buffer that we read from and write to file from
            nbytes -- number of bytes we read
  * outputs: n/a
  * return val: status flag (success: 0 if written properly, -1 else)
  * effects: DO NOT WRITE TO FILE!!!!!
  * STATUS: works
*/
int32_t directory_write(int32_t fd, const void *buf, int32_t nbytes){
  return -FAILURE;
  ////DIRECTORIES ARE READ ONLY////
}



////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////           FILE SYSCALLS             //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


/* 
  file_open
  * inputs: filename -- the name of file to open
  * return val: status flag (success: 0 if opened properly, -1 else)
  * effects: open the file for processing
  * STATUS: works
*/
int32_t file_open(const uint8_t *filename){
  dentry_t the_dentry; //the dentry that represents the directory containing the file
  int32_t idx = read_dentry_by_name(filename, &the_dentry);
  if(idx == -FAILURE){
    return -FAILURE; //file does not exist
  }
  return idx; //file found;
}

/* 
  file_close
  * inputs: fd -- the descriptor for the file to close up
  * return val: status flag (success: 0 if closed properly, -1 else)
  * effects: close up the file
  * STATUS: works
*/
int32_t file_close(int32_t fd){
  return SUCCESS;
}


/* 
  file_read
  * inputs: unsigned int fd -- file descriptor
            buf -- buffer that we write to after reading
            nbytes -- number of bytes we read
  * return val: status flag (success: 0 if file data read properly, -1 else)
  * effects: read the data contained in each file
  * STATUS: works
*/
int32_t file_read(int32_t fd, void *buf, int32_t nbytes){
  //do something with read_data for file
  int32_t bytes_to_read;
  int32_t bytes_actually_read;

  //check if the args are valid
  if(fd < 0 || nbytes < 0 || buf == NULL){
    return -FAILURE;
  }
  //1. get the pcb running currently and read the data for its inode
  pcb_t* cur_pcb = getRunningPCB();
  // printf("FD index: %d\n", fd);
  // printf("dentry inode_num: %d\n", cur_pcb->fd_arr[fd].inode_num);
  // printf("the original: %d\n", boot_base_addr->dentries[fd].inode_num);
  inode_t* inode = &inode_start_ptr[cur_pcb->fd_arr[fd].inode_num];
  if(cur_pcb->fd_arr[fd].fpos >= inode->len){
    return 0; //EOF has been hit
  }
  //Calculate the number of bytes to read
  bytes_to_read = nbytes;
  if(cur_pcb->fd_arr[fd].fpos + nbytes > inode->len){
    bytes_to_read = inode->len - cur_pcb->fd_arr[fd].fpos; //Adjust bytes that will be read if file size exceeded
  }
  //Pull data from file
  // printf("inode_number: %d\n", cur_pcb->fd_arr[fd].inode_num);
  // printf("file position: %d\n", file_pos_in_dir);
  bytes_actually_read = read_data(cur_pcb->fd_arr[fd].inode_num, cur_pcb->fd_arr[fd].fpos, buf, bytes_to_read);
  if(bytes_actually_read < 0){
    return -FAILURE;
  }
  cur_pcb->fd_arr[fd].fpos += bytes_actually_read;
  return bytes_actually_read;
}

/* 
  file_write
  * inputs: unsigned int fd -- file descriptor
            buf -- buffer that we read from and write into the file
            nbytes -- number of bytes we write
  * return val: status flag (success: 0 if file data written properly, -1 else)
  * effects: DO NOT WRITE TO THE FILE
  * STATUS: works
*/
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
  * return val: status flag (success: 0 if read properly, -1 else)
  * effects: read into dentry given name
  * STATUS: WORKS
*/
int32_t read_dentry_by_name(const uint8_t *fname, dentry_t *dentry){
  if(fname == NULL){
    return -FAILURE;
  }
  int len_filename = strlen((int8_t*)fname);
  //check if filename is between 32 and 0 chars
  if(len_filename <= 0){
    return -FAILURE;
  }
  if(len_filename > FILENAME_LEN){
    len_filename = FILENAME_LEN;
  }

  int i;
  for(i = 0; i < SUPERBLOCK_SIZE; ++i){
    uint8_t dentry_len = strlen((int8_t*)dentry_start_ptr[i].filename);

    if(dentry_len - 1 >= FILENAME_LEN){
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

/* 
  * read_dentry_by_name
  * Description: searches the system from dentry fname
  * inputs:  index -- the dentry index
             dentry -- pointer to the dentry object
  * outputs: n/a
  * return val: status flag (success: 0 if read, -1 else)
  * effects: read info into the dentry object given index
  * STATUS: WORKS
*/
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
  * return val: status flag (success: 0 if data read, -1 else)
  * effects: read file datablocks

*/

// read data from the file given inode num //
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    uint32_t num_bytes_read = 0; //counter for number of bytes read
    uint32_t datablock_curr = offset / EXT2_DATA_BLOCK_SIZE;
    uint32_t byte_curr = offset % EXT2_DATA_BLOCK_SIZE; //the current byte to read
    //uint32_t buf_idx = 0; //current index in buf we are writing to after read
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

      /////// pointer arith for data blocks
      uint32_t block = (uint32_t)data_block_ptr;
      block += (data_block_num*EXT2_DATA_BLOCK_SIZE);
      block += byte_curr; //calculate address to read from based on ptr and block
      // printf(block);
      // buf[buf_idx] = block[block_offset]; 
      memcpy(buf, (uint32_t*)block, 1);
      //buf_idx++;
      buf++; 
      byte_curr++; num_bytes_read++;
    }

    return num_bytes_read;
}

////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////             TESTS                ////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////

int32_t directory_test(){
  uint8_t buf[FILENAME_LEN];
  int i;
  int file_i;
  dentry_t* cur_dent;
  inode_t* cur_inode;
  uint8_t file0;
  int32_t fd0;
  int32_t noneb;

	directory_open(&file0);
	uint32_t dentry_num = boot_base_addr->dentry_ct;

  for(i=0; i < dentry_num; ++i){

    cur_dent = (dentry_t*)&(boot_base_addr->dentries[i]);
    cur_inode = (inode_t*)(inode_start_ptr + (cur_dent->inode_num));
    int32_t size = cur_inode->len;

    directory_read(fd0,buf,noneb);
    printf("%d - NAME: ", i);
    
    for(file_i = 0; file_i < FILENAME_LEN; file_i++){
      if(buf[file_i] != '\0'){
        putc(buf[file_i]);
      } else {
        putc(' ');
        break;
      }
    }
    printf("SIZE: %d\n", size);
  }

  directory_close(0);
  printf("TEST PASSED :D");
  return 0;
}

int32_t readTextFile(){
  int32_t fileIdxInDir = 10; //the fileindx of frame0.txt
  uint8_t filename[FILENAME_LEN] = "frame0.txt";
  int numReadBytes;
  uint8_t buf[TEST_BUFFER_SIZE];
  if(file_open((uint8_t *)filename) == -FAILURE){
    printf("Cannot read file :/");
    return -FAILURE;
  }
  numReadBytes = file_read(fileIdxInDir, buf, MAX_FILE_SIZE);
  printf("Num bytes read: %d\n", numReadBytes);
  int i;
  for(i = 0; i < numReadBytes; i++){
    putc(buf[i]);
  }
  printf("\nframe0.txt\n");
  if(file_write(fileIdxInDir, buf, numReadBytes) != -FAILURE){
    return -FAILURE;
  }
  file_close(fileIdxInDir);
  printf("Test passed :D");
  return 0;
}

int32_t readTextFileLarge(){
  int32_t fileIdxInDir = 11; //the fileindx of verylargetextwithverylongname.txt
  uint8_t filename[33] = "verylargetextwithverylongname.txt";
  int numReadBytes;
  uint8_t buf[TEST_BUFFER_SIZE];
  if(file_open((uint8_t *)filename) == -FAILURE){
    printf("Cannot read file :/");
    return -FAILURE;
  }
  numReadBytes = file_read(fileIdxInDir, buf, MAX_FILE_SIZE);
  printf("Num bytes read: %d\n", numReadBytes);
  int i;
  for(i = 0; i < numReadBytes; i++){
    putc(buf[i]);
  }
  printf("\nverylargetextwithverylongname.txt\n");
  if(file_write(fileIdxInDir, buf, numReadBytes) != -FAILURE){
    return -FAILURE;
  }
  file_close(fileIdxInDir);
  printf("Test passed :D");
  return 0;
}


int32_t readBin(){
  int32_t fileIdxInDir = 3; //the fileidx of pingpong
  uint8_t filename[32] = "grep";
  int numReadBytes;
  uint8_t buf[10000];
  uint8_t elfCheck[3];
  if(file_open((uint8_t *)filename) == -FAILURE){
    printf("Cannot read file :/");
    return -FAILURE;
  }
  numReadBytes = file_read(fileIdxInDir, buf, MAX_FILE_SIZE);
  printf("Num bytes read: %d\n", numReadBytes);
  int i;
  for(i = 0; i < numReadBytes; i++){
    putc(buf[i]);
    if(buf[i] == 'E' || buf[i] == 'L' || buf[i] == 'F'){
      elfCheck[i] = buf[i];
    }
  }
  printf("\ngrep\n");
  if(file_write(fileIdxInDir, buf, numReadBytes) != -FAILURE){
    return -FAILURE;
  }
  file_close(fileIdxInDir);
  for(i = 0; i < 4; i++){
    putc(elfCheck[i]);
  }
  printf("\nTest passed :D");
  return 0;
}
