/*******************************************************************************
	Model Data Convter Main
*******************************************************************************/

#include	<windows.h>
#include	<commctrl.h>
#include	<crtdbg.h>
#include	"mainwindow.h"
#include	"../FmToneEditorManagerInner.h"

// data segment
#pragma data_seg (".scciseg")
	HINSTANCE g_hInstance = NULL;
	FmToneEditorManager* g_pManager = NULL;
#pragma data_seg()

//--------------------------------------------------
// dll main
//--------------------------------------------------
BOOL WINAPI DllMain
( HINSTANCE hinstDLL, DWORD dwDllMainEvent ,LPVOID lpvReserved )
{
    switch( dwDllMainEvent )
    {    case DLL_PROCESS_ATTACH:
			g_hInstance = hinstDLL;
			if(g_pManager == NULL){
				g_pManager = new FmToneEditorManagerInner(hinstDLL);
			}
			break;
         case DLL_PROCESS_DETACH:
			if(g_pManager != NULL){
				delete g_pManager;
				g_pManager = NULL;
			}
			break;
         case DLL_THREAD_ATTACH:
			 break;
         case DLL_THREAD_DETACH:
			 break;
     }
     return TRUE;
}

//--------------------------------------------------
// 通常エディタモード用
//--------------------------------------------------
extern "C" __declspec(dllexport) int WindowMain(HINSTANCE hInst,HINSTANCE oldhInst,LPSTR cmd,int Show){
	int				ret;
	Application		*app;
	HANDLE			hMutex;
//	----------多重起動チェック----------
		hMutex=CreateMutex(FALSE,0,"FmToneEditorMutexNmae");
		if(GetLastError()==ERROR_ALREADY_EXISTS){
			CloseHandle(hMutex);
			return 0;
		}
//	----------Appvクラス発生----------
	app = new Application(g_hInstance,oldhInst,cmd,Show);
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
	return	ret;
}

//--------------------------------------------------
// 非連携時の動作
//--------------------------------------------------
extern "C" __declspec(dllexport) FmToneEditor* GetManagerInstance()
{
	return reinterpret_cast<FmToneEditor*>(g_pManager);
}

