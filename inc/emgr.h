/* Error handling and definitions */

#include <stdint.h>
#include <string.h>
#include <stdio.h>

//#define __DEBUG_MODE

#ifdef __DEBUG_MODE

#include "logger.h"

#endif

#ifndef _EMGR_INC

#define _EMGR_INC 


// Define error or OK ids for emgr_err_struct->is_error
#define OK 0

#define ERROR 2

#define UNDEFINED 1

#define DESC_SIZE 32

// define TRUE and FALSE values
#define TRUE 1

#define FALSE 0

typedef enum {
  tUndefined,
  tFATFS,
  tFSMGR
} emgr_err_type;


typedef struct{ 
  uint8_t is_error;
  emgr_err_type type;
  uint8_t code;
  char desc[DESC_SIZE];
} emgr_err_struct;

extern emgr_err_struct emgr_struct;

extern const char * TYPE_STRING[];


//init error manager
void init_emgr();

// make error
void emgr_error1(emgr_err_struct * estruct, emgr_err_type type, uint8_t code, char * desc);

// make error without description
void emgr_error2(emgr_err_struct * estruct, emgr_err_type type, uint8_t code);

// make ok result
void emgr_ok(emgr_err_struct * estruct, emgr_err_type type);

//print result to somewhere
void emgr_print(emgr_err_struct * estruct);

//print result to somewhere
void emgr_print_ffl(emgr_err_struct * estruct, const char * File, const char * Func, int L);

// test result on error
uint8_t emgr_is_error(emgr_err_struct * estruct);

// test result on ok
uint8_t emgr_is_ok(emgr_err_struct * estruct);

//reset values of error strcut
void emgr_reset(emgr_err_struct * estruct);

// simple print message to debugger
void print_msg(char * String);

//format error to string error
void format_error(char * ErrChar, uint8_t ErrCode);

// return OK from void function
#define RET_OK(type) emgr_ok(&emgr_struct, type); \
  emgr_print_ffl(&emgr_struct, __FILE__, __FUNCTION__, __LINE__);	\
  return

// return ok with result of function
#define RET_VALUE_OK(type, value) emgr_ok(&emgr_struct, type);	\
  emgr_print_ffl(&emgr_struct, __FILE__, __FUNCTION__, __LINE__); \
  return value

// return error as result of function. With descriprion
#define RET_ERROR_DESC(type, code, desc) emgr_error1(&emgr_struct, type, code, desc); \
  emgr_print_ffl(&emgr_struct, __FILE__, __FUNCTION__, __LINE__);		\
  return

// return simple error from function
#define RET_ERROR(type, code) emgr_error2(&emgr_struct, type, code); \
  emgr_print_ffl(&emgr_struct, __FILE__, __FUNCTION__, __LINE__); \
  return

// return with error from current function if nested function returned error
#define RET_ON_PREV_ERROR if(emgr_is_error(&emgr_struct)) {		\
  emgr_print_ffl(&emgr_struct, __FILE__, __FUNCTION__, __LINE__);	  \
  return; }

// return with error from current function if nested function returned error. Possible to change type and code of error
#define RET_ON_PREV_ERROR_NC(type, code) if(emgr_is_error(&emgr_struct)){	\
  emgr_error2(&emgr_struct, type, code); \
  emgr_print_ffl(&emgr_struct, __FILE__, __FUNCTION__, __LINE__);	  \
  return }

// test result on error
#define IS_ERROR emgr_is_error(&emgr_struct)

// test result on ok
#define IS_OK emgr_is_ok(&emgr_struct)


#endif
