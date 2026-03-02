/*******************************************************************************

	mainwin.cpp header file

*******************************************************************************/

#pragma once
#include	<windows.h>
#include	<process.h>

class	WindowBase;

/*************************************
	アプリケーションメインクラス
*************************************/
class	ApplicationBase{
protected:
	HINSTANCE		m_hInst,m_hPrev;		//	インスタンスハンドル
	int				m_show;				//	ショーモード
	WindowBase		*m_pWindow;	//	メインウィンドウポインタ
public:
	LPTSTR			m_cmdline;		//	コマンドライン
	static			ApplicationBase *app;		//	１つのオブジェクト
	//	----------初期化----------
	ApplicationBase(HINSTANCE inst,HINSTANCE prev,LPTSTR cmd,int sh);
	//	----------Desctructer---------
	~ApplicationBase();
	//	----------インスタンス取得関数----------
	HINSTANCE	GetInstance(void){
		return	m_hInst;
	};
	//	----------アプリケーション初期化関数----------
	virtual	BOOL	Init(void);
	//	----------メッセージループ関数----------
	virtual	LONG_PTR		Msg(void);
	//　---------- ウィンドウハンドル取得 ----------
	virtual HWND getWindoHandle();
};

/**********************************
	基本ウィンドウクラス
**********************************/
class	WindowBase{
protected:
	int			reg;				//	レジスト情報？
//	----------共通関数----------
	static	LONG_PTR CALLBACK VFunc(HWND hw,UINT msg,WPARAM wParam,LPARAM lParam);

public:
	WindowBase() : reg(0) {}

	PAINTSTRUCT	ps;						//	ペイント用構造体
	HWND		m_hWnd;					//	ウィンドウハンドル
	HMENU		m_hMenu;				//	メニュー
	DWORD		m_Style;				//	スタイル
	DWORD		m_ExStyle;				//	拡張スタイル
	BOOL		m_ActiveMessageLoop;	//	全快メッセージループモード
//	----------クラス名----------
	CHAR		m_cClassname[128];			//	クラス名は、１２８文字まで
	//	----------クラス登録----------
	virtual ATOM	Register(	LPCTSTR cname = NULL,
						LPCTSTR menu = NULL,HBRUSH backg = (HBRUSH)(COLOR_WINDOW + 1),
						HICON icon = NULL,HCURSOR curcor = NULL);
	//	----------クリエート-----------
	HWND	Create(	LPCTSTR name,LONG style = WS_OVERLAPPEDWINDOW,
					DWORD ExStyle = NULL,HMENU menu = NULL,HWND hw = NULL,
					int x = CW_USEDEFAULT,int y = CW_USEDEFAULT,
					int w = CW_USEDEFAULT,int h = CW_USEDEFAULT);
	//	---------- ウィンドウサイズ ---------
	BOOL	SetSize(int w,int h);
	//	---------ＳＨＯＷウィンドウ----------
	void	Show(int sh){
		ShowWindow(m_hWnd,sh);
	}
	//	----------Ｕｐｄａｔｅウィンドウ----------
	void	Update(void){
		UpdateWindow(m_hWnd);
	}
	//	----------ウィンドウサイズ取得----------
	BOOL	GetWindowBaseSize(POINT *pt){
		RECT	rc;
		BOOL	ret;
		GetWindowRect(m_hWnd,&rc);
		pt->x = rc.right - rc.left;
		pt->y = rc.bottom - rc.top;
		return	ret;
	}
	//	----------クライアントサイズ取得----------
	BOOL	GetClientSize(POINT *pt){
		RECT	rc;
		BOOL	ret;
		GetClientRect(m_hWnd,&rc);
		pt->x = rc.right - rc.left;
		pt->y = rc.bottom - rc.top;
		return	ret;
	}
	//	---------- ウィンドウスタイル追加 ----------
	BOOL	AddWindowStyle(HWND hwnd,DWORD style,BOOL ex = FALSE);
	//	---------- ウィンドウスタイル削除 ----------
	BOOL	DeleteWindowStyle(HWND hwnd,DWORD style,BOOL ex = FALSE);
	//	----------コールバックルーチン----------
	virtual	LONG_PTR CALLBACK Func(UINT msg,WPARAM wParam,LPARAM lParam);
};

/***********************************************
	ウィンドウサブクラス化クラス
***********************************************/
//	----------ウィンドウをサブクラス化する関数----------
class	SubWindowBase{
public:
	HWND		m_hWnd;		//	ハンドル保存用
	HWND		m_hOwnWnd;	//	オーナーウィンドウ
	WNDPROC		proc;			//	オリジナルコールバック保存用
	//	----------コンストラクター----------
	SubWindowBase(HWND hw,HWND hOwnWnd){
		m_hWnd = hw;							//	ハンドル保存
		m_hOwnWnd = hOwnWnd;
		proc = (WNDPROC)GetWindowLongPtr(hw,GWLP_WNDPROC);	//	オリジナルコールバック取得
		SetWindowLongPtr(hw,GWLP_WNDPROC,(LONG)Hook);	//	設定なり自分自身をフック
		SetWindowLongPtr(hw,GWLP_USERDATA,(LONG)this);				//	自分登録
	};
	//	----------デスクトラクター----------
	virtual ~SubWindowBase(){
		SetWindowLongPtr(m_hWnd,GWLP_WNDPROC,(LONG)proc);	//	ポインタ復元
	}
	//	----------コールバックフック用----------
	static	LONG_PTR CALLBACK Hook(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
	//	----------受け取り関数----------
	virtual	LONG_PTR CALLBACK Func(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam);
};


