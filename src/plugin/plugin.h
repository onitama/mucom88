
//
//	MUCOM88 plugin manager
//
#ifndef __PLUGIN_h
#define __PLUGIN_h

#include "mucom88if.h"

int Mucom88Plugin_Init( void *hwnd, mucomvm *vm, CMucom *mucom );
void Mucom88Plugin_Term( void );
void Mucom88Plugin_Update( int prm );
void Mucom88Plugin_Play( int prm );
void Mucom88Plugin_Stop( int prm );
void Mucom88Plugin_ToolExec( char *name, int prm );

int Mucom88Plugin_GetInterfaceMax( void );
int Mucom88Plugin_GetInterfaceId( char *name );
MUCOM88IF *Mucom88Plugin_GetInterface( int id );

#endif


