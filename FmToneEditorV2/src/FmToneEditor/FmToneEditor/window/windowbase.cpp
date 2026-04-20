/*******************************************************************************

	ウィンドウなり

*******************************************************************************/

#include	<windows.h>
#include	<stdio.h>
#include	<commctrl.h>
#include	"windowbase.h"

/******************************
	ＡＰＰクラスメンバー
******************************/
ApplicationBase	*ApplicationBase::app = NULL;				//	初期化？

//	---------- コンストラクタ ----------
ApplicationBase::ApplicationBase(HINSTANCE inst,HINSTANCE prev,LPTSTR cmd,int sh){
	m_hInst = inst;
	m_hPrev = prev;
	m_cmdline = cmd;
	m_show = sh;
	app = this;				//	自分自身のポインタを取得する
	m_pWindow = NULL;
}

//	---------- デスクトラクタ ----------
ApplicationBase::~ApplicationBase(){
	delete	m_pWindow;		//	メインウィンドウを消す
}

//	----------初期化？----------
BOOL	ApplicationBase::Init(void){
	//	---------Boot Window----------
	m_pWindow = new WindowBase();
	if(m_pWindow->Create("BaseWindow",WS_OVERLAPPEDWINDOW) == FALSE) return FALSE;
	m_pWindow->Show(m_show);
	m_pWindow->Update();
	return	TRUE;
}
//	----------メッセージループ----------
LONG_PTR	ApplicationBase::Msg(void){
	MSG	msg;
	BOOL bRes;
	while((bRes = GetMessage(&msg,NULL,NULL,NULL))){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return	msg.wParam;
}

//	----------ウィンドウハンドル取得----------
HWND ApplicationBase::getWindoHandle(){
	return m_pWindow->m_hWnd;
}


/******************************************************************************

	基本ウィンドウ関係メンバー関数

******************************************************************************/
//	----------ウィンドウクラス設定-----------
ATOM	WindowBase::Register(LPCTSTR cname,LPCTSTR menu,HBRUSH backg,HICON icon,HCURSOR cursor){
	WNDCLASS	wc;
	wsprintf(m_cClassname,cname);
	reg = 1;
	wc.style		= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc	= WindowBase::VFunc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= ApplicationBase::app->GetInstance();
	wc.hIcon		= icon;
	wc.hCursor		= cursor ? cursor : LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = backg;
	wc.lpszMenuName = menu;
	wc.lpszClassName = m_cClassname;
	return RegisterClass(&wc);
}

//	----------ウィンドウ作成----------
HWND	WindowBase::Create(LPCTSTR name,LONG style,DWORD ExStyle,HMENU menu,HWND hw,int x,int y,int w,int h){
	if(!reg) Register(name);
	m_hMenu = menu;
	m_Style = style;
	m_ExStyle = ExStyle;
	m_ActiveMessageLoop = FALSE;
	m_hWnd = CreateWindowEx(ExStyle,m_cClassname,name,style,x,y,w,h,hw,menu,ApplicationBase::app->GetInstance(),(LPVOID)this);
	return	m_hWnd;
}

//	---------- ウィンドウサイズ決定 ----------
BOOL	WindowBase::SetSize(int w,int h){
	RECT	rc,rc2;
	rc.left = 0;
	rc.top = 0;
	rc.right = w;
	rc.bottom = h;
	AdjustWindowRectEx(&rc,m_Style,((m_hMenu) ? TRUE : FALSE),m_ExStyle);
	GetWindowRect(m_hWnd,&rc2);
	return	MoveWindow(m_hWnd,rc2.left,rc2.top,rc.right - rc.left,rc.bottom - rc.top,FALSE);
}

//	----------共通受け取り関数----------
LONG_PTR 	CALLBACK WindowBase::VFunc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam){
	WindowBase *pWin = NULL;
	//	----------初回各ウィンドウ毎にコールバックを振り分けする----------
	if(msg == WM_NCCREATE){
		LPCREATESTRUCT cs = (LPCREATESTRUCT)lParam;
		::SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)((WindowBase*)cs->lpCreateParams));
		pWin = (WindowBase*)cs->lpCreateParams;
		pWin->m_hWnd = hwnd;
	}
	//	----------呼び出しを行う-----------
	pWin = (WindowBase*)(::GetWindowLongPtr(hwnd,GWLP_USERDATA));
	if(pWin == NULL){
		return DefWindowProc(hwnd,msg,wParam,lParam);		//	CallBack Run
	}
	//	----------終了の場合の処理を掛ける---------
	return	pWin->Func(msg,wParam,lParam);
}

//	----------基本受け取りルーチン----------
LONG_PTR	CALLBACK WindowBase::Func(UINT msg,WPARAM wParam,LPARAM lParam){
	switch(msg){
		case	WM_CREATE:	//	クリエート
			break;
		case	WM_CLOSE:	//	クローズ
			DestroyWindow(m_hWnd);
			break;
		case	WM_DESTROY:	//	デストロイ
			PostQuitMessage(0);
			break;
		default: return DefWindowProc(m_hWnd,msg,wParam,lParam);
	}
	return	0L;
}

//	---------- ウィンドウスタイル追加 ----------
BOOL	WindowBase::AddWindowStyle(HWND hwnd,DWORD style,BOOL ex){
	DWORD	result;
	DWORD	st;
	
	if(ex == FALSE){		//	ノーマル
		st = GetWindowLong(hwnd,GWL_STYLE);
		st |= style;
		result = SetWindowLong(hwnd,GWL_STYLE,st);
	}else{					//	拡張
		st = GetWindowLong(hwnd,GWL_EXSTYLE);
		st |= style;
		result = SetWindowLong(hwnd,GWL_EXSTYLE,st);
	}
	return	TRUE;
}

//	---------- ウィンドウスタイル削除 ----------
BOOL	WindowBase::DeleteWindowStyle(HWND hwnd,DWORD style,BOOL ex){
	DWORD	result;
	DWORD	st;
	DWORD	mask;
	//	--------- マスク作製 ---------
	mask = !style;
	if(ex == FALSE){		//	ノーマル
		st = GetWindowLong(hwnd,GWL_STYLE);
		st = st & mask;
		result = SetWindowLong(hwnd,GWL_STYLE,st);
	}else{					//	拡張
		st = GetWindowLong(hwnd,GWL_EXSTYLE);
		st = st & mask;
		result = SetWindowLong(hwnd,GWL_EXSTYLE,st);
	}
	return	TRUE;
}

/******************************************************************************
	ウィンドウのサブクラス化用のクラス
******************************************************************************/
//	----------メインフック用のスタティック関数----------
LONG_PTR CALLBACK SubWindowBase::Hook(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
	SubWindowBase	*SubWin;
	SubWin = (SubWindowBase*)GetWindowLongPtr(hWnd,GWLP_USERDATA);	//	ＴＨＩＳポインタ取得
	return	SubWin->Func(hWnd,uMsg,wParam,lParam);
}

//	----------基本コールバック受け取り関数----------
LONG_PTR CALLBACK SubWindowBase::Func(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam){
#if	0
	switch(uMsg){
		//	----------フックしない物を引き渡す----------
		default: //	----------フックしない場合の処理を行う----------
			return CallWindowProc((WNDPROC)proc,hWnd,uMsg,wParam,lParam);
	}
#endif
	return	0L;
}

