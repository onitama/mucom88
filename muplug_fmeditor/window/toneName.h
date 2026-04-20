// 音色ペースト確認用
#pragma once
#include <string>
#include "DialogBase.h"

using namespace std;
class ToneName:public DialogBase {
private:
	string	m_sToneName;
	int		m_iToneNo;
public:
	// コンストラクタ
	ToneName();
	// デストラクタ
	~ToneName();
	// メッセージコールバック
	virtual BOOL CALLBACK Func(HWND hdlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
	// 初期化処理
	void Initialize();
	// 音色名称設定
	void SetToneName(string sName) {
		m_sToneName = sName;
	};
	// 音色名称取得
	string GetToneName() {
		return m_sToneName;
	}
};
