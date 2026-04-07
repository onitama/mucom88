/*******************************************************************************
	Model Data Convter Main
*******************************************************************************/

#include	<windows.h>
#include	<commctrl.h>
#include	<crtdbg.h>
#include	"mainwindow.h"
#include	"../FmToneEditorManagerInner.h"

//--------------------------------------------------
// 通常エディタモード用
//--------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	LONG_PTR ret;
	Application		*app;
	HANDLE			hMutex;
//	----------多重起動チェック----------
	hMutex=CreateMutex(FALSE,0,"FmToneEditorV2MutexNmae");
	if(GetLastError()==ERROR_ALREADY_EXISTS){
		CloseHandle(hMutex);
		return 0;
	}
//	----------Appvクラス発生----------
	app = new Application(hInstance,hPrevInstance,lpCmdLine,nCmdShow);
//	----------ウィンドウ作成----------
	app->Init();
//	----------メッセージループ----------
	ret = app->Msg();	//	メッセージループ
//	----------開放処理----------
	delete	app;
	//ReleaseMutex(hMutex);
//	----------メモリリークチェック---------
	_CrtDumpMemoryLeaks();
	//	----------抜け出し----------
	return	(int)ret;
}


