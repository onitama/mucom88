#pragma once
#include	<windows.h>

extern	char	*MemLoad(const char *fname,DWORD *sz = NULL);
extern	BOOL	MemSave(const char *fname,LPVOID pBuffer,DWORD size);
extern	BOOL	FileSaveDialog(HWND hWnd,LPTSTR fname,LPTSTR path,LPTSTR title,LPTSTR type,LPTSTR lpstrDefExt);
