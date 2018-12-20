
//
//	MUCOM88 plugin interface structures
//
#ifndef __CMucom88IF_h
#define __CMucom88IF_h

#include <windows.h>

/*------------------------------------------------------------*/
//	for MUCOM88Win interface
/*------------------------------------------------------------*/

//		ファンクション型
//
typedef int (__stdcall *MUCOM88IF_COMMAND) (int,int,int,void *);
typedef int (__stdcall *MUCOM88IF_CALLBACK)(int);

#define MUCOM88IF_VERSION	0x100		// 1.0

#define MUCOM88IF_TYPE_NONE 0
#define MUCOM88IF_TYPE_TOOL 1			// 起動させるタイプの外部ツール

#define	MUCOM88IF_COMMAND_NONE 0
#define	MUCOM88IF_COMMAND_PASTECLIP 1	// エディタにクリップボードテキストを貼り付け

class mucomvm;
class CMucom;

typedef struct {

	//	Memory Data structure
	//	(*) = DLL側で書き換え可
	//
	HWND hwnd;							// メインウィンドウハンドル
	int version;						// MUCOM88IFバージョン
	char name[16];						// プラグイン名(英文字)
	int	type;							// プラグインタイプ
	char *info;							// プラグイン情報テキスト(*)
	unsigned char *chmute;				// チャンネルミュートテーブル(*)
	unsigned char *chstat;				// チャンネルステータステーブル
	int cur_count;						// 現在演奏中のカウンタ
	int max_count;						// カウンタの長さ

	//	コールバックファンクション
	int (__stdcall *if_term) (int);			// 解放
	int (__stdcall *if_update) (int);		// フレーム処理
	int (__stdcall *if_play) (int);			// 演奏開始
	int (__stdcall *if_stop) (int);			// 演奏停止
	int (__stdcall *if_tool) (int);			// ツール起動

	//	クラス情報 (変更の可能性があります)
	mucomvm *vm;
	CMucom *mucom;

	//	汎用ファンクション(仮)
	//
	int(__stdcall* if_editor) (int, int, int, void *);				// エディタ系のサービス
	int(__stdcall* if_fmreg) (int, int, int, void *);				// FM音源のレジスタ書き込み

} MUCOM88IF;

typedef int(__stdcall *MUCOM88IF_INIT)(MUCOM88IF *);				// プラグイン初期化

#endif
