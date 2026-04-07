// FmToneEditorマネージャー呼び出して意義
#pragma once
#include <Windows.h>

// 構造体定義
typedef struct tagFmRegisterCommand {
	DWORD	reg;
	DWORD	data;
} FmRegisterCommand;

//----------------------------------------
// エディターマネージャー
//----------------------------------------
class FmToneEditorManager {
	// 公開関数
public:
	// 初期化
	virtual BOOL __stdcall Initialize() = 0;
	// 開放
	virtual BOOL __stdcall UnInitialize() = 0;
	// 表示非表示
	virtual BOOL __stdcall Show(BOOL bShow) = 0;
	// 演奏後の割り込み通知
	virtual void __stdcall StartAnalyze(void) = 0;
	// 演奏前の通知
	virtual void __stdcall StartPlayback(void) = 0;

};

//----------------------------------------
// マネージャー取得用関数
//----------------------------------------
typedef FmToneEditorManager* (__stdcall *editormanager)(void);

