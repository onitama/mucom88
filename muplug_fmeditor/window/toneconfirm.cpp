// 音色ペースト確認用
#pragma once
#include <regex>
#include "../resource.h"
#include "toneconfirm.h"

// コンストラクタ
ToneConfirm::ToneConfirm(){
	m_sToneName = "";
	m_iToneNo = -1;
}

// デストラクタ
ToneConfirm::~ToneConfirm(){
}

// メッセージコールバック
BOOL CALLBACK ToneConfirm::Func(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		case WM_INITDIALOG:
			Initialize();
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDOK:
					// 音色名を取得する
					{
						char str[7];
						memset(str, 0x00, 7);
						GetDlgItemText(m_hDlg, IDC_EDIT1, str, 7);
						// 音色名をメンバーに設定する
						m_sToneName = str;
						// ここで、音色チェックする
						regex reg("^[\x20-\x7e]+$");
						smatch match;
						if (regex_match(m_sToneName, match, reg) == false) {
							// エラーを出す
							MessageBox(m_hDlg, "NameはASCIIコードで入力可能な文字のみ指定できます。", "入力エラー", MB_OK | MB_ICONERROR);
							break;
						}
					}
					// エディットボックスのテキストを取得する
					EndDialog(hDlg, LOWORD(wParam));
				break;
				case IDCANCEL:
				EndDialog(hDlg, IDCANCEL);
				break;
			}
			break;
		case WM_CLOSE:
			EndDialog(hDlg, IDCANCEL);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

// 初期化処理
void ToneConfirm::Initialize() {
	// センターリングする
	SetCenterPosition();
	// 入力できる文字数を設定する
	SendDlgItemMessage(m_hDlg, IDC_EDIT1, EM_LIMITTEXT, 6, 0);
	// ここで、初期化処置を行う
	char str[256];
	wsprintf(str, "音色番号[%d]にクリップボードの音色を設定しますか？", m_iToneNo);
	SetDlgItemText(m_hDlg, IDC_STATIC1, str);
	SetDlgItemText(m_hDlg, IDC_EDIT1, m_sToneName.c_str());
}

