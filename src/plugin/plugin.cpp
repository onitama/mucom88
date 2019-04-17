
//
//		MUCOM88 plugin interface
//			Windows version by onion software/onitama 2018/12
//

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "plugin.h"

int dummyCallback(void *instance, int cmd, void *p1, void *p2)
{
	Mucom88Plugin *plg = (Mucom88Plugin *)instance;
	return 0;
}

Mucom88Plugin::Mucom88Plugin()
{
	hwnd = NULL;
	version = MUCOM88IF_VERSION;		// MUCOM88IFバージョン
	filename[0] = 0;					// プラグインファイル名
	instance = NULL;					// DLLインスタンス

	type = MUCOM88IF_TYPE_NONE;			// プラグインタイプ(*)
	info = "";							// プラグイン情報テキストのポインタ(*)

	//	コールバックファンクション
	if_notice = dummyCallback;

	//	汎用ファンクション
	//
	if_mucomvm = NULL;
	if_editor = NULL;

	//	クラス情報 (バージョンで内容変更の可能性があります)
	vm = NULL;
	mucom = NULL;

}

Mucom88Plugin::~Mucom88Plugin()
{
}

