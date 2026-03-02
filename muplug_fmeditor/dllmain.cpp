// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

#include	"window/mainwindow.h"
#include	"FmToneEditorManagerInner.h"

// data segment
#pragma data_seg (".scciseg")
HINSTANCE g_hInstance = NULL;
FmToneEditorManager* g_pManager = NULL;
#pragma data_seg()


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		g_hInstance = hModule;
		break;
	case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

//--------------------------------------------------
//	プラグイン機能コールバック
//--------------------------------------------------

static int noticeCallback(void *instance, int cmd, void *p1, void *p2)
{
	Mucom88Plugin *plg = (Mucom88Plugin *)instance;
	//printf( "#Notice %d.\n",cmd );
	switch(cmd) {
	case MUCOM88IF_NOTICE_BOOT:				// 初期化時
		if (g_pManager == NULL) {
			g_pManager = new FmToneEditorManagerInner(g_hInstance);
			g_pManager->SetPlugin(plg);
			if (g_pManager->Initialize()) {
				g_pManager->Show(true);
			}
			else {
				g_pManager = NULL;
			}
		}
		break;
	case MUCOM88IF_NOTICE_TERMINATE:		// 解放時
		if (g_pManager != NULL) {
			g_pManager->UnInitialize();
			delete g_pManager;
			g_pManager = NULL;
		}
		break;
	case MUCOM88IF_NOTICE_INTDONE:			// 演奏ルーチン実行後
		if (g_pManager != NULL) {
			g_pManager->StartAnalyze();
		}
		break;
	case MUCOM88IF_NOTICE_PREPLAY:			// 演奏スタート前
		if (g_pManager != NULL) {
			g_pManager->StartPlayback();
		}
		break;
	case MUCOM88IF_NOTICE_TOOLSTART:		// ツール表示・非表示リクエスト
		if (g_pManager != NULL) {
			g_pManager->Show(true);
		}
		break;
	case MUCOM88IF_NOTICE_TOOLHIDE:		// ツール表示・非表示リクエスト
		if (g_pManager != NULL) {
			g_pManager->Show(false);
		}
		break;
		
	}
	return 0;
}


//--------------------------------------------------
//	プラグイン初期化
//--------------------------------------------------
extern "C" __declspec(dllexport) int InitalizePlugin(Mucom88Plugin *plugin, int bootopt )
{
	plugin->info = "MUCOM88 FMEDITOR PLUGIN";		// プラグイン情報テキスト
	plugin->type = MUCOM88IF_TYPE_TOOL;				// プラグインタイプを設定(MUCOM88IF_TYPE_*)
	plugin->if_notice = noticeCallback;				// 通知用コールバックを登録する

	//printf( "Welcome! plugin!\n" );

	return 0;
}

