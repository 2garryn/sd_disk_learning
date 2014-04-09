#include "fsmgr.h"
#include "syscalls.c"
#include "integer.h"

#define INDEX_FILE "idx.dat"
#define FILE_LIST "filelist.dat"


FIL index_file;
FIL file_list;
unsigned short int number_files = 0;
unsigned short int current_track = 0;
uint8_t play_mode;



struct index_struct {
  unsigned long address; // position of file name string in tracklist FILE_LIST
  unsigned short int size_path; // length of file name string
};

// initialization of filesystem manager
void init_fsmgr() {
  //mount fs
  FRESULT fr;
  FATFS fs;

  fr = f_mount(&fs, "", 1);

  //return if error
  if (fr != FR_OK) {
    RET_ERROR(tFSMGR, fr);
  };
  
  //try toi update index of files
  update_index();

  //return if error happened
  RET_ON_PREV_ERROR;

  //return ok
  RET_OK(tFSMGR);

};

// set random mode. Tracks will be given randomized from all file index
void set_random_mode() {
  set_mode(TRACK_RANDOM);
};

// set track by track order 
void set_simple_mode() {
  set_mode(SIMPLE);
};

// play and repeat one track
void set_track_repeat_mode(){
  set_mode(TRACK_REPEAT);
};

//get next track 
void get_next_track(char * next_track) {
  if(number_files == 0) {
    RET_ERROR(tFSMGR, NO_TRACKS);
  };

  switch (play_mode) {
  case SIMPLE: calc_simple_next(next_track); break;
  case TRACK_REPEAT: calc_repeat(next_track); break;
  case TRACK_RANDOM: calc_random_next(next_track); break;
  };

};
  
//get prev track 
void get_prev_track(char * prev_track) {
  if(number_files == 0) {
    RET_ERROR(tFSMGR, NO_TRACKS);
  };

  switch (play_mode) {
  case SIMPLE: calc_simple_prev(prev_track); break;
  case TRACK_REPEAT: calc_repeat(prev_track); break;
  case TRACK_RANDOM: calc_random_next(prev_track); break;
  };

};



// set track selection mode. SIMPLE, TRACK_REPEAT or TRACK_RANDOM
void set_mode(uint8_t p_mode){
  play_mode = p_mode;
};

// get next track by regular order
void calc_simple_next(char * buff_track){
  if (current_track == number_files) {
    current_track = 1;
  }
  else {
    ++current_track;
  };
  get_track_by_number(current_track, buff_track);

};

// get prev track by regular order
void calc_simple_prev(char * buff_track){
  if (current_track == 1) {
    current_track = number_files;
  }
  else {
    --current_track;
  };
  get_track_by_number(current_track, buff_track);

};

//get current track
void calc_repeat(char * buff_track){
  get_track_by_number(current_track, buff_track);

};

// get random track
void calc_random_next(char * buff_track) {
  // simple stack TODO: implement random
  calc_simple_next(buff_track);
};

//select track from index with number. Error if there are tracks
void get_track_by_number(unsigned short int track_number, char * buff_name) {
  
  FRESULT fr;
  UINT br;
  index_struct ins;
  
  fr = f_lseek(index_file, (track_number - 1) * sizeof(index_struct));
  if(fr != FR_OK) {
    RET_ERROR(tFSMGR, fr);
  };

  fr = f_read(index_file, (void *) &ins, (UINT) sizeof(ins), &br);
  
  if(fr != FR_OK) {
    RET_ERROR(tFSMGR, fr);
  };
  
  fr = f_lseek(file_list, ins.address);
  
  if(fr != FR_OK) {
    RET_ERROR(tFSMGR, fr);
  };

  fr = f_read(file_list, (void *) buff_name, ins.size_path, &br);
  if(fr != FR_OK) {
    RET_ERROR(tFSMGR, fr);
  };

};

  

// update index before start of usage
void update_index() {
  FRESULT fr;
  
  ensure_remove(FILE_LIST);
  RET_ON_PREV_ERROR;

  ensure_remove(INDEX_FILE);
  RET_ON_PREV_ERROR;

  ensure_open_file(INDEX_FILE, index_file);
  RET_ON_PREV_ERROR;

  ensure_open_file(FILE_LIST, file_list);
  RET_ON_PREV_ERROR;
  
  index_file_system("", &index_file, &file_list);
  RET_ON_PREV_ERROR;

  if(number_files == 0) {
    RET_ERROR(tFSMGR, NO_TRACKS);
  };

  RET_OK(tFSMGR);

};

//ensure remove file
void ensure_remove( char * file_path ){
  FILINFO fno;
  FRESULT fr;
  
  //test on file exist
  fr = f_stat(file_path, &fno);
  if (fr == FR_NO_FILE) {
    //FR_NO_FILE is ok. there is no file
    RET_OK(tFSMGR);
  }
  else {
    if(fr == FR_OK) {
      fr = f_unlink(file_path);
      if (fr == FR_OK){
	RET_OK(tFSMGR);
      }
      else {
	RET_ERROR(tFSMGR, fr);
      }
    } 
    else {
      RET_ERROR(tFSMGR, fr);
    }
  }
};     

//save open file function
void ensure_open_file(char * file_path, FIL * fd) {
  FRESULT fr;
  
  fr = f_open(fd, file_path, FA_READ | FA_WRITE);
  if(fr != FR_OK) {
    RET_ERROR(tFSMGR, fr);
  };
  RET_OK(tFSMGR);
};

// read and index file system
void index_file_system(char * current_path, FIL * index_file, FIL * file_list, ) {
  char path[2048];
  char trackfile[2048];
  FILINFO fno;
  FRESULT fr;
  DIR dir;
  int i;
  char *fn;   /* This function is assuming non-Unicode cfg. */
  // we need to work on new path, otherwise will get all thrash in current_path
  strcpy(path, current_path);

  static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
  fno.lfname = lfn;
  fno.lfsize = sizeof lfn;
  //try to open directory
  fr = f_opendir(&dir, path); 
  // if it is no ok, return error of fatfs
  if (fr != FR_OK) {
    RET_ERROR(tFSMGR, fr);
  };
  i = strlen(path);
  
  for (;;) {
    fr = f_readdir(&dir, &fno);                   /* Read a directory item */
    if (fr != FR_OK) {      /* Break on error or end of dir */
      f_closedir(&dir);  
      RET_ERROR(tFSMGR, fr);
    };
    
    if (fno.fname[0] == 0) {
      f_closedir(&dir);
      RET_OK(tFSMGR);
    };

    if (fno.fname[0] == '.') continue; 
    fn = *fno.lfname ? fno.lfname : fno.fname;
    if (fno.fattrib & AM_DIR) {                    /* It is a directory */

      sprintf(&path[i], "/%s", fn);

      // make recursive
      index_file_system(path, index_file, file_list);
      // if scan_files return error or end of dir

      if (IS_ERROR) {
	f_closedir(&dir); 
       	RET_ON_PREV_ERROR;
      };

      path[i] = 0;
    } else {                                       /* It is a file. */
      sprintf(trackfile, "%s/%s", path, fn);

      add_file_index(trackfile, index_file, file_list);
      RET_ON_PREV_ERROR;
      
    };
  };
};

// add file to index system
void add_file_index( char * trackfile, FiL * index_file, FIL * file_list){
  DWORD current_pos;
  UINT written_bytes;
  index_struct new_ins;
  FRESULT fr;

  ++number_files; //update file counter
  //fill structure to write index data to index file
  new_ins.size_path = (strlen(trackfile) + 1) * sizeof(char); // number of characters with zero * size of one char
  new_ins.address   = f_tell(file_list); // get file address to get from file 
  
  fr = f_write(file_list, trackfile, (UINT) new_ins.size_path, &written_bytes);
  if(fr != FR_OK) {
    RET_ERROR(tFSMGR, fr);
  };

  if(written_bytes < new_ins.size_path) {
    RET_ERROR(tFSMGR, NO_DISK_SPACE);
  };

  fr = f_write(index_file, &new_ins, (UINT) sizeof(new_ins), &written_bytes);
  if(fr != FR_OK) {
    RET_ERROR(tFSMGR, fr);
  };

  if(written_bytes < new_ins.size_path) {
    RET_ERROR(tFSMGR, NO_DISK_SPACE);
  };
  RET_OK(tFSMGR);
};
  
  
