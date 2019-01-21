// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
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
	printf( "#Notice %d.\n",cmd );
	return 0;
}


//--------------------------------------------------
//	プラグイン初期化
//--------------------------------------------------
extern "C" __declspec(dllexport) int InitalizePlugin(Mucom88Plugin *plugin, int bootopt )
{
	plugin->info = "MUCOM88 TEST PLUGIN";			// プラグイン情報テキスト
	plugin->type = MUCOM88IF_TYPE_SILENT;			// プラグインタイプを設定(MUCOM88IF_TYPE_*)
	plugin->if_notice = noticeCallback;				// 通知用コールバックを登録する

	printf( "Welcome! plugin!\n" );

	return 0;
}

