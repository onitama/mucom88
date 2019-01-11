
//
//		MUCOM88 plugin interface
//			Windows version by onion software/onitama 2018/12
//

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "plugin.h"

int dummyCommand(void *instance, int cmd, int prm1, int prm2, void *prm3, void *prm4)
{
	return 0;
}

int dummyCallback(void *instance, int cmd)
{
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
	if_init = dummyCallback;
	if_term = dummyCallback;
	if_notice = dummyCallback;

	//	汎用ファンクション
	//
	if_mucomvm = dummyCommand;
	if_editor = dummyCommand;

	//	クラス情報 (バージョンで内容変更の可能性があります)
	vm = NULL;
	mucom = NULL;

}

Mucom88Plugin::~Mucom88Plugin()
{
}

