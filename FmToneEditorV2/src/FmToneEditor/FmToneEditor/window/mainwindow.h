//----------------------------------------------------------------------
// mml window
//----------------------------------------------------------------------
#pragma once
#include <Windows.h>
#include "windowbase.h"
#include <commctrl.h>
#include "../util/DibModule.h"
#include "../editor/editor.h"

#define _RICHEDIT_VER	0x0210
#include <richedit.h>		//	リッチエディット

#pragma comment(lib,"comctl32.lib")

class MainWindow;

// class
class Application:public ApplicationBase{
private:
public:
	Application(HINSTANCE inst,HINSTANCE prev,LPTSTR cmd,int sh);
	~Application();
	LONG_PTR	 Msg(void);
	BOOL Init(void);
	void	SetSyncMode(BOOL bSync);
	MainWindow *GetWindowInstance(){
		return reinterpret_cast<MainWindow*>(m_pWindow);
	}
};

// class
#define		EDIT_LINE_WIDTH	(48)
class EditSub;
class MainWindow:public WindowBase{
private:
	HMODULE	m_hREdit;
	HWND	m_hEdit;
	HWND	m_hStatusBar;
	DibModule	*m_pDib;
	FmToneEditor *m_pEditor;
	// 初期化完了フラグ
	BOOL	m_bInitialized;
	// オプション項目
	BOOL m_bAgedeyesMode;
	BOOL m_bMucom;
	BOOL m_bMmldrv;
	BOOL m_bSCCI;
	// 編集中のトーン番号
	int	m_iToneNo;
	// 編集モード
	BOOL	m_bSyncMode;

protected:
public:
	MainWindow();
	~MainWindow();
/*
	virtual ATOM	Register(	LPCTSTR cname = NULL,
						LPCTSTR menu = NULL,HBRUSH backg = (HBRUSH)(COLOR_WINDOW + 1),
						HICON icon = NULL,HCURSOR curcor = NULL);
*/
	LONG_PTR CALLBACK Func(UINT msg,WPARAM wParam,LPARAM lParam);
	// 初期化
	BOOL		initialize(WPARAM wParam,LPARAM lParam);
	BOOL		unInitialize(WPARAM wParam,LPARAM lParam);
	void		SetSyncMode(BOOL bSync);
	BOOL		Paint(WPARAM wParam,LPARAM lParam);
	BOOL		EditSetFont(void);
	BOOL		Size(WPARAM wParam,LPARAM lParam);
	BOOL		command(WPARAM wParam,LPARAM lParam);
	LONG_PTR	syscommand(WPARAM wParam,LPARAM lParam);
	BOOL		dropFile(WPARAM wParam,LPARAM lParam);
	BOOL		Copy();
	BOOL		Paste();

	// パラメータの取得処理
	BOOL		GetMucomParameter(std::string ,VOICEFORMAT &param);
	BOOL		GetMmldrvParameter(std::string ,VOICEFORMAT &param);

	// パラメータを設定する
	BOOL		SetToneParam(WPARAM wParam,LPARAM lParam);

	// エディタークラスのインスタンス取得
	FmToneEditor* GetEditorInstance(){
		return m_pEditor;
	}

	// 初期化状態を取得する
	BOOL		IsInitialized(){
		return m_bInitialized;
	}

};

