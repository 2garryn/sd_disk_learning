
#include "emgr.h"
#include <stdio.h>
#include "ff.h"




#ifndef _FSMGR_INC
#define _FSMGR_INC 

void init_fsmgr();

void read_file_system(char * path);

void update_index();

#define SIMPLE 0

#define TRACK_RANDOM 1

#define TRACK_REPEAT 2

#define NO_DISK_SPACE 100

#define NO_TRACKS 101

#endif
