#include "fsmgr2.h"

// field offset in bytes
#define FIELD_OFFSET(field_name)  offsetof(struct idx_struct, field_name)

#define IDX "index.dat"
#define PTH "pth.dat"

#define PRINT_ERR(err) print_error((uint8_t)err)

/* 
Initialization of file system manager

*/
FSMGR_RESULT init_fsmgr(fsmgr_handler * fsmgr_h){
  FRESULT fr;
  FATFS fs;
  FSMGR_RESULT fsmgr_res;

  fr = f_mount(&fs, "", 1);
  
  if (fr != FR_OK) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };
  
  fsmgr_res = update_index(&(fsmgr_h->idx_fd), &(fsmgr_h->pth_fd));
  
  if (fsmgr_res != FSMGR_OK){
    PRINT_ERR(fsmgr_res);
    return fsmgr_res;
  };
  
  return FSMGR_OK;
  
};  


FSMGR_RESULT get_next(fsmgr_handler * fsmgr_h){
  if(fsmgr_h->next_idx == 0) {
    return END_OF_DIR;
  };

  return get_by_idx(fsmgr_h, fsmgr_h->next_idx);

};


FSMGR_RESULT get_prev(fsmgr_handler * fsmgr_h){
  if(fsmgr_h->prev_idx == 0) {
    return START_OF_DIR;
  };

  return get_by_idx(fsmgr_h, fsmgr_h->prev_idx);
};


FSMGR_RESULT cd_in(fsmgr_handler * fsmgr_h){
  if(fsmgr_h->ftype != T_DIR) {
    return NOT_DIR;
  };
  if(fsmgr_h->child_idx == 0) {
    return EMPTY_DIR;
  };

  return get_by_idx(fsmgr_h, fsmgr_h->child_idx);
};


FSMGR_RESULT cd_out(fsmgr_handler * fsmgr_h){
  if(fsmgr_h->parent_idx == 0) {
    return IS_ROOT_DIR;
  };

  return get_by_idx(fsmgr_h, fsmgr_h->parent_idx);
};


FSMGR_RESULT get_rel_path(fsmgr_handler * fsmgr_h, char * path_str){
  FRESULT fr;
  UINT rw_bytes;

  fr = f_lseek(&(fsmgr_h->pth_fd), fsmgr_h->pth_idx);
  if(fr != FR_OK) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };

  fr = f_read(&(fsmgr_h->pth_fd),(void *) path_str, (UINT) (fsmgr_h->pth_len * sizeof(char)), &rw_bytes);
  
  if(fr != FR_OK) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };

  if(rw_bytes < (fsmgr_h->pth_len * sizeof(char))) {
      PRINT_ERR(EOF_REACHED);
      return EOF_REACHED;
  };
  
  return FSMGR_OK;
};


FSMGR_RESULT get_by_idx(fsmgr_handler * fsmgr_h, unsigned long idx){
  FRESULT fr;
  idx_struct idx_st;
  UINT rw_bytes;

  fr = f_lseek(&(fsmgr_h->idx_fd), idx);
  if(fr != FR_OK) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };

  fr = f_read(&(fsmgr_h->idx_fd), 
	      (void *) &idx_st, 
	      (UINT) sizeof(idx_st), &rw_bytes);
  if(fr != FR_OK) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };

  if(rw_bytes < sizeof(idx_struct)) {
      PRINT_ERR(EOF_REACHED);
      return EOF_REACHED;
  };
  
  fsmgr_h->parent_idx = idx_st.parent_idx;
  fsmgr_h->next_idx   = idx_st.next_idx;
  fsmgr_h->prev_idx   = idx_st.prev_idx;
  fsmgr_h->child_idx  = idx_st.child_idx;
  fsmgr_h->pth_idx    = idx_st.pth_idx;
  fsmgr_h->pth_len    = idx_st.pth_len;
  
  fsmgr_h->ftype      = idx_st.ftype;
  
  return FSMGR_OK;
  
};
  

/* 
======================================================
INTERNAL FUNCTIONS
======================================================
*/


/*
Prepare index files end execute recursive update
 */
FSMGR_RESULT update_index(FIL * idx_fd, FIL * pth_idx_fd){
  FSMGR_RESULT fsmgr_res;
   
  fsmgr_res = recreate_file(IDX, idx_fd);
  if(fsmgr_res != FSMGR_OK){
    PRINT_ERR(fsmgr_res);
    return fsmgr_res;
  };

  fsmgr_res = recreate_file(PTH, pth_idx_fd);
  if(fsmgr_res != FSMGR_OK){
    PRINT_ERR(fsmgr_res);
    return fsmgr_res;
  };
  
  fsmgr_res = index_fs("", idx_fd, pth_idx_fd, 0);
  if(fsmgr_res != FSMGR_OK){
    PRINT_ERR(fsmgr_res);
    return fsmgr_res;
  };
  
  return FSMGR_OK;
};



//ensure remove file
FSMGR_RESULT recreate_file( char * file_path, FIL * fd ){
  FILINFO fno;
  FRESULT fr;

  //test on file exist
  fr = f_stat(file_path, &fno);
  if ((fr != FR_OK) && (fr != FR_NO_FILE)) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };
  // remove file if it is exist
  if(fr == FR_OK) {
    fr = f_unlink(file_path);
    if (fr != FR_OK){
      PRINT_ERR(fr);
      return FATFS_ERROR;
    }
  }; 
  // create file
  fr = f_open(fd, file_path, FA_READ | FA_WRITE);
  if(fr != FR_OK) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };
  
  return FSMGR_OK;
};  


FSMGR_RESULT index_fs(char * current_path,
		      FIL * idx_fd,
		      FIL * pth_idx_fd,
		      unsigned long parent_idx){
  char path[2048];
  FILINFO fno;
  FRESULT fr;
  DIR dir;
  FSMGR_RESULT fsmgr_res;
  int i;
  char *fn;   /* This function is assuming non-Unicode cfg. */
  unsigned long prev_idx = 0;
  unsigned long temp_prev_idx = 0;
  
  
  // we need to work on new path, otherwise will get all thrash in current_path
  strcpy(path, current_path);
  static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
  fno.lfname = lfn;
  fno.lfsize = sizeof lfn;
  //try to open directory
  fr = f_opendir(&dir, path); 
  // if it is no ok, return error of fatfs
  
  if (fr != FR_OK) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };
  i = strlen(path);
  
  for (;;) {
    fr = f_readdir(&dir, &fno);                   /* Read a directory item */
    if (fr != FR_OK) {
      f_closedir(&dir);  
      PRINT_ERR(fr);
      return FATFS_ERROR;
    };
    
    //check have we files in directory or not.
    if (fno.fname[0] == 0) {
      f_closedir(&dir);
      return FSMGR_OK;
    };
  
    if (fno.fname[0] == '.') continue;  
    fn = *fno.lfname ? fno.lfname : fno.fname;
    if (fno.fattrib & AM_DIR) {                    /* It is a directory */

      temp_prev_idx = f_tell(idx_fd); //save current position. Will be new struct pos
      fsmgr_res = add_to_index(idx_fd, //fd of index file
                               pth_idx_fd, //fd of path file
                               parent_idx, //parent index str addr
                               prev_idx,   //previous idx addr
                               fn,         //string with new pathname
                               T_DIR);

      if(fsmgr_res != FSMGR_OK){
        PRINT_ERR(fsmgr_res);
        return fsmgr_res;
      };
      
      if(prev_idx != 0) {
	//this is not first child dir ->  
	//we should update 'next_idx' pointer in previous idx structure
	fsmgr_res =update_idx_nl_field(idx_fd, 
				       prev_idx,  
				       FIELD_OFFSET(next_idx), 
				       temp_prev_idx);
      } else {
	// this is first child dir
	// set pointer to this structture in parent idx
	fsmgr_res =update_idx_nl_field(idx_fd, 
				       parent_idx,  
				       FIELD_OFFSET(child_idx), 
				       temp_prev_idx);
      };

      if(fsmgr_res != FSMGR_OK){
        PRINT_ERR(fsmgr_res);
        return fsmgr_res;
      };
      
      prev_idx = temp_prev_idx;
      
      sprintf(&path[i], "/%s", fn);

      // make recursive
      fsmgr_res = index_fs(path, idx_fd, pth_idx_fd, prev_idx);
      // if scan_files return error or end of dir
      if (fsmgr_res != FSMGR_OK) {
        f_closedir(&dir); 
       	return fsmgr_res;
      };

    } else {                                       /* It is a file. */
      temp_prev_idx = f_tell(idx_fd);
      fsmgr_res = add_to_index(idx_fd, //fd of index file
                               pth_idx_fd, //fd of path file
                               parent_idx, //parent index str addr
                               prev_idx,   //previous idx addr
                               fn,         //string with new pathname
                               T_FILE);

      if(fsmgr_res != FSMGR_OK){
        PRINT_ERR(fsmgr_res);
        return fsmgr_res;
      };

      if(prev_idx != 0) {
	//this is not first child file ->  
	//we should update 'next_idx' pointer in previous idx structure
	fsmgr_res =update_idx_nl_field(idx_fd, 
				       prev_idx,  
				       FIELD_OFFSET(next_idx), 
				       temp_prev_idx);
      } else {
	// this is first child file
	// set pointer to this structture in parent idx
	fsmgr_res =update_idx_nl_field(idx_fd, 
				       parent_idx,  
				       FIELD_OFFSET(child_idx), 
				       temp_prev_idx);
      };

      if(fsmgr_res != FSMGR_OK){
        PRINT_ERR(fsmgr_res);
        return fsmgr_res;
      };
      
      prev_idx = temp_prev_idx;
    };
  };
  
};


FSMGR_RESULT add_to_index(FIL * idx_fd,
			  FIL * pth_fd,
			  unsigned long parent_idx,
                          unsigned long prev_idx,
                          char * fn,
                          file_type type){

  idx_struct idx;
  UINT written_bytes;
  FRESULT fr;
  
  idx.pth_len = (strlen(fn) + 1) * sizeof(char);
  idx.pth_idx = f_tell(pth_fd);
  idx.parent_idx = parent_idx;
  idx.prev_idx   = prev_idx;
  idx.next_idx   = 0;
  idx.child_idx  = 0;
  idx.ftype      = type;
  
  fr = f_write(pth_fd,(void *) fn, (UINT) (idx.pth_len * sizeof(char)), &written_bytes);
  
  if(fr != FR_OK) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };

  if(written_bytes < idx.pth_len) {
    PRINT_ERR(NO_DISK_SPACE);
    return NO_DISK_SPACE;
  };

  fr = f_write(idx_fd, &idx, (UINT) sizeof(idx), &written_bytes);
  
  if(fr != FR_OK) {
    PRINT_ERR(fr);
    return FATFS_ERROR;
  };

  if(written_bytes < sizeof(idx_struct)) {
    PRINT_ERR(NO_DISK_SPACE);
    return NO_DISK_SPACE;
  };
  
  return FSMGR_OK;
    
  
};

FSMGR_RESULT update_idx_nl_field(FIL * idx_fd, unsigned long dst_idx_ost,
				 uint8_t offset, unsigned long new_value){
    FRESULT fr;
    idx_struct idx_st;
    UINT rw_bytes;
    unsigned long current_idx;
    // save current read/write pointer
    current_idx = f_tell(idx_fd);

    // go to offset of updatable index
    fr = f_lseek(idx_fd, dst_idx_ost);
    if(fr != FR_OK) {
      PRINT_ERR(fr);
      return FATFS_ERROR;
    };

    fr = f_read(idx_fd, (void *) &idx_st, (UINT) sizeof(idx_st), &rw_bytes);
    if(fr != FR_OK) {
      PRINT_ERR(fr);
      return FATFS_ERROR;
    };

    if(rw_bytes < sizeof(idx_struct)) {
      PRINT_ERR(EOF_REACHED);
      return EOF_REACHED;
    };

    *((unsigned long *)((uint8_t * )&idx_st + offset)) = new_value;

    fr = f_lseek(idx_fd, dst_idx_ost);
    if(fr != FR_OK) {
      PRINT_ERR(fr);
      return FATFS_ERROR;
    };

    fr = f_write(idx_fd, (void *) &idx_st, (UINT) sizeof(idx_st), &rw_bytes);

    if(fr != FR_OK) {
      PRINT_ERR(fr);
      return FATFS_ERROR;
    };

    if(rw_bytes < sizeof(idx_struct)) {
      PRINT_ERR(NO_DISK_SPACE);
      return NO_DISK_SPACE;
    };

    fr = f_lseek(idx_fd, current_idx);
    if(fr != FR_OK) {
      PRINT_ERR(fr);
      return FATFS_ERROR;
    };

    return FSMGR_OK;

};

void print_error(uint8_t err) {
  return;
};

















