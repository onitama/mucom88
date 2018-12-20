
//
//	mucomerror.cpp header
//
#ifndef __mucomerror_h
#define __mucomerror_h

// エラーコード

#define MUCOMERR_NONE 0
#define MUCOMERR_MAX 17

char *mucom_geterror(int error);
char *mucom_geterror_j(int error);
int mucom_geterror(char *orgerror);

#endif
