
#ifndef __UNICODE_INC
#define __UNICODE_INC 111

#include "ff.h"
WCHAR ff_convert (	/* Converted character, Returns zero on error */
	WCHAR	chr,	/* Character code to be converted */
	UINT	dir		/* 0: Unicode to OEMCP, 1: OEMCP to Unicode */
			);

WCHAR ff_wtoupper (	/* Upper converted character */
	WCHAR chr		/* Input character */
			);


#endif
