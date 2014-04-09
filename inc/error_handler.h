/* Error handling and definitions */

typedef enum {
  tFATFS,
  tFSMGR
} emgr_err_type;

typedef struct emgr_err_struct{ 
  uint8_t is_error;
  err_type type;
  uint8_t code;
  char desc[32]
};

//init error manager
void init_emgr();

// make error
void emgr_error1(emgr_err_struct * estruct, emgr_err_type type, uint8_t code, char * desc);

// make error without description
void emgr_error2(emgr_err_struct * estruct, emgr_err_type type, uint8_t code);

// make ok result
void emgr_ok(emgr_err_struct * estruct);

//print result to somewhere
void emgr_print(emgr_err_struct * estruct);

// test result on error
uint8_t emgr_is_error(emgr_err_struct * estruct);

// test result on ok
uint8_t emgr_is_ok(emgr_err_struct * estruct);
