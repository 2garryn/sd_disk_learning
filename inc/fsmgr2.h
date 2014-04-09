#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "syscalls.c"
#include "integer.h"
#include "ff.h"

#ifndef _FSMGR_INC
#define _FSMGR_INC 

typedef uint8_t file_type;

#define T_DIR 1
#define T_FILE 2

typedef struct {
  FIL idx_fd;
  FIL pth_fd;
  
  unsigned long parent_idx;

  unsigned long next_idx;

  unsigned long prev_idx;

  unsigned long child_idx;
  
  file_type ftype;
  
  unsigned long pth_idx;

  unsigned char pth_len;

} fsmgr_handler;


//internal index file struct
typedef struct idx_struct{
  unsigned long pth_idx; // offset of file name
  unsigned char pth_len; // length of file name string
  
  unsigned long parent_idx; //parent index data addr
  unsigned long prev_idx; //prev idx in current path
  unsigned long next_idx; //next idx in current path
  unsigned long child_idx; //first child idx. Only for directories
  
  file_type ftype; //type of idx entity - file or dir
  
} idx_struct;


typedef uint8_t FSMGR_RESULT;

#define FSMGR_OK 100
#define FATFS_ERROR 101
#define NO_DISK_SPACE 102
#define EOF_REACHED 103
#define END_OF_DIR 104 //there is no next file/dir
#define START_OF_DIR 105 //there is no previous file/dir
#define NOT_DIR 106 //it is not directory. can't go into this
#define EMPTY_DIR 107//empty dir, can't go into this
#define IS_ROOT_DIR 108// is root dir. Can't to parent dir


//API
FSMGR_RESULT get_next(fsmgr_handler * fsmgr_h);

FSMGR_RESULT get_prev(fsmgr_handler * fsmgr_h);

FSMGR_RESULT cd_in(fsmgr_handler * fsmgr_h);

FSMGR_RESULT cd_out(fsmgr_handler * fsmgr_h);

FSMGR_RESULT get_rel_path(fsmgr_handler * fsmgr_h, char * path_str);

FSMGR_RESULT get_by_idx(fsmgr_handler * fsmgr_h, unsigned long idx);

//prototypes
FSMGR_RESULT init_fsmgr(fsmgr_handler * fsmgr_h);

FSMGR_RESULT update_index(FIL * idx_fd, FIL * pth_idx_fd);

FSMGR_RESULT recreate_file(char * file_path, FIL * fd);

FSMGR_RESULT index_fs(char * current_path,
		      FIL * idx_fd,
		      FIL * pth_idx_fd,
		      unsigned long parent_idx);

FSMGR_RESULT add_to_index(FIL * idx_fd,
			  FIL * pth_fd,
			  unsigned long parent_idx,
                          unsigned long prev_idx,
                          char * fn,
                          file_type type);

FSMGR_RESULT update_idx_nl_field(FIL * idx_fd, 
				 unsigned long dst_idx_ost,
				 uint8_t offset, 
				 unsigned long new_value);
void print_error(uint8_t err);

#endif
