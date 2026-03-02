#include	<stdio.h>
#include	<shlobj.h>
#include	"fileutil.h"

#pragma warning(disable : 4996)

char	*MemLoad(const char *fname,DWORD *sz){
	HANDLE	hFile;
	DWORD	size,readsize;
	char	*mem;
	//	----------ファイルをオープンする----------
	hFile = CreateFile(fname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == NULL){
		return	NULL;
	}
	//	----------ファイルサイズ確保を行う----------
	size = GetFileSize(hFile,NULL);
	if(size == 0xffffffff){
		CloseHandle(hFile);
		return	NULL;
	}
	//	----------メモリー確保を行う----------
	mem = new char[size];
	if(mem == NULL){
		CloseHandle(hFile);
		return	NULL;
	}
	//	----------ファイルを読込む----------
	if(ReadFile(hFile,mem,size,&readsize,NULL) == FALSE){
		free(mem);
		CloseHandle(hFile);
		return	NULL;
	}

	//	----------終了処理----------
	CloseHandle(hFile);
	if(sz != NULL) *sz = size;
	return	mem;
}

// ファイルを保存する
BOOL	MemSave(const char *fname,LPVOID pBuffer,DWORD size){
	HANDLE	hFile;
	DWORD	writesize;
	//	----------ファイルをオープンする----------
	hFile = CreateFile(fname,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile == NULL){
		return	FALSE;
	}
	//	----------ファイルを読込む----------
	if(WriteFile(hFile,pBuffer,size,&writesize,NULL) == FALSE){
		CloseHandle(hFile);
		return	FALSE;
	}
	//	----------終了処理----------
	CloseHandle(hFile);
	return	TRUE;
}

// ファイル保存ダイアログ表示
BOOL	FileSaveDialog(HWND hWnd,LPTSTR fname,LPTSTR path,LPTSTR title,LPTSTR type,LPTSTR lpstrDefExt){
	OPENFILENAME	fo;
	char	buf[MAX_PATH];
	if(path == NULL){
		buf[0] = 0;
	}else{
		lstrcpy(buf,path);
	}
	//	----------ファイル名取得ダイアログ初期化-----------
	ZeroMemory(&fo,sizeof(OPENFILENAME));
	fo.lStructSize		= sizeof(OPENFILENAME);				//	自分のサイズ
	fo.hwndOwner 		= hWnd;								//	親ウィンドウ
	fo.hInstance		= GetModuleHandle(NULL);
	fo.lpstrFilter		= type;
	fo.nFilterIndex		= 0;
	fo.lpstrTitle		= title;
	fo.Flags			= OFN_HIDEREADONLY;
	fo.lpstrFile		= buf;
	fo.nMaxFile			= MAX_PATH;
	fo.lpstrDefExt		= lpstrDefExt;
	fo.Flags			= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	//	----------ダイアログ起動----------
	if(GetSaveFileName(&fo) == FALSE) return FALSE;
	lstrcpy(fname,buf);
	return TRUE;
}
