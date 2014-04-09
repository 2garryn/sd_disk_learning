#include "emgr.h"


emgr_err_struct emgr_struct;

const char* TYPE_STRING[] = {
    "tUndefined", 
    "tFATFS", 
    "tFSMGR"
};


void init_emgr(){
  //Initialization code
  emgr_reset( &emgr_struct );

  //#ifdef __DEBUG_MODE
  logger_initialize();
  //#endif

};

void emgr_error1(emgr_err_struct * estruct, emgr_err_type type, uint8_t code, char * desc) {
  estruct->is_error = ERROR;
  estruct->type = type;
  estruct->code = code;
  memcpy(estruct->desc, desc, sizeof(char) * DESC_SIZE);
};

void emgr_error2(emgr_err_struct * estruct, emgr_err_type type, uint8_t code){
  estruct->is_error = ERROR;
  estruct->type = type;
  estruct->code = code;
  estruct->desc[0] = 0;
};


// make ok result
void emgr_ok(emgr_err_struct * estruct, emgr_err_type type){
  estruct->is_error = OK;
  estruct->type = type;
  estruct->code = 0;
  estruct->desc[0] = 0;
};
  
// test result on error
uint8_t emgr_is_error(emgr_err_struct * estruct){
  if (estruct->is_error == ERROR){
    return TRUE;
  }
  else {
    return FALSE;
  };
};

// test result on ok
uint8_t emgr_is_ok(emgr_err_struct * estruct){
  if (estruct->is_error == OK){
    return TRUE;
  }
  else {
    return FALSE;
  };
};

void emgr_reset(emgr_err_struct * estruct) {
  estruct->is_error = UNDEFINED;;
  estruct->type = tUndefined;;
  estruct->code = 0;
  memset(estruct->desc, 0, sizeof(estruct->desc));
}

//print result to somewhere
void emgr_print(emgr_err_struct * estruct){
#ifdef __DEBUG_MODE

  char buffer[256] = "";
  char ErrChar[6];
  uint8_t str_size = 0;

  format_error(ErrChar, estruct->is_error);
  
  str_size = sprintf(buffer, "Result:%s type:%s code:%d\n",
		     ErrChar, TYPE_STRING[estruct->type], estruct->code);
  buffer[str_size] = 0;
  logger_print(str_size, buffer);

#endif
};

void emgr_print_ffl(emgr_err_struct * estruct, const char * File, const char * Func,  int Line){
#ifdef __DEBUG_MODE

  char buffer[256] = "";
  char ErrChar[6];
  uint8_t str_size = 0;

  format_error(ErrChar, estruct->is_error);
  
  str_size = sprintf(buffer, "Result:%s type:%s code:%d file:%s func:%s line:%d\n",
		     ErrChar, TYPE_STRING[estruct->type], estruct->code,
		     File, Func, Line);
  buffer[str_size] = 0;
  logger_print(str_size, buffer);


#endif
};

void print_msg(char * String) {
  logger_print(strlen(String), String);
};

void format_error(char * ErrChar, uint8_t ErrCode){
  if (ErrCode == OK){
    ErrChar = "OK";
  } else if (ErrCode == ERROR){
    ErrChar = "Error";
  } else if (ErrCode == UNDEFINED){
    ErrChar = "Undef";
  };
};
