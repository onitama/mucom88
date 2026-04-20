//----------------------------------------------------------------------
// scci window
//----------------------------------------------------------------------
#pragma once
#include "mainwindow.h"
#include <commctrl.h>
#include "../resource.h"
#include "../Util/fileutil.h"
#include "../Util/profile.h"
#include "../FmToneEditorManagerInner.h"
#include "../resource.h"
#include "../editor/ToneParam.h"
#include "toneconfirm.h"
#include "toneName.h"
#include "../midiin.h"

// ウィンドウサイズ定義
#define	WINDOW_WIDTH	640
#define WINDOW_HEIGHT	480

//------------------------------------
// コンストラクタ
//------------------------------------
Application::Application(HINSTANCE inst,HINSTANCE prev,LPTSTR cmd,int sh):ApplicationBase(inst,prev,cmd,sh){
}

//------------------------------------
// デストラクタ
//------------------------------------
Application::~Application(){
}

//------------------------------------
// 初期化
//------------------------------------
BOOL	Application::Init(void){
	//	---------Boot Window----------
	m_pWindow = new MainWindow();
	m_pWindow->Register("FMTONEEDITOR_WINDOW_CLASS",(LPCTSTR)IDR_MENU1,(HBRUSH)(COLOR_MENU+1),LoadIcon(GetInstance(),MAKEINTRESOURCE(IDI_ICON1)));
	if(m_pWindow->Create("FmToneEditor",WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX) == FALSE) return FALSE;
	m_pWindow->Show(m_show);
	m_pWindow->Update();
	return	TRUE;
}

//------------------------------------
// メッセージループ
//------------------------------------
LONG_PTR	Application::Msg(void){
	MSG	msg;

	HACCEL hAccelTable;
	static ACCEL sAccel[] = {
		{FCONTROL | FVIRTKEY,'S',ID_40006},
	};
	// アクセラレータ設定する
	hAccelTable = CreateAcceleratorTable(sAccel, sizeof(sAccel) / sizeof(ACCEL));

	while (GetMessage(&msg, NULL, NULL, NULL)) {
		if (!TranslateAccelerator(m_pWindow->m_hWnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return	msg.wParam;
}

//------------------------------------
// コンストラクタ
//------------------------------------
MainWindow::MainWindow(){
	m_hStatusBar	= NULL;
	m_pDib			= NULL;
	m_pEditor		= NULL;
	m_bAgedeyesMode = FALSE;
	m_bMucom		= TRUE;
	m_bMmldrv		= FALSE;
	m_bSCCI			= TRUE;
	m_iToneNo		= 0;
	m_bSyncMode		= TRUE;
	m_bInitialized	= FALSE;
}

//------------------------------------
// デストラクタ
//------------------------------------
MainWindow::~MainWindow(){
}

//------------------------------------
// デストラクタ
//------------------------------------
//	----------ウィンドウクラス設定-----------
ATOM	MainWindow::Register(LPCTSTR cname,LPCTSTR menu,HBRUSH backg,HICON icon,HCURSOR cursor){
	WNDCLASS	wc;
	wsprintf(m_cClassname,cname);
	reg = 1;
	wc.style		= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
	wc.lpfnWndProc	= WindowBase::VFunc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 8;
	wc.hInstance	= ApplicationBase::app->GetInstance();
	wc.hIcon		= icon;
	wc.hCursor		= cursor ? cursor : LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = backg;
	wc.lpszMenuName = menu;
	wc.lpszClassName = m_cClassname;
	return RegisterClass(&wc);
}

//------------------------------------
// コールバック関数
//------------------------------------
LONG_PTR CALLBACK MainWindow::Func(UINT msg,WPARAM wParam,LPARAM lParam){
	LONG_PTR	lRet = 0L;
	switch(msg){
		case WM_CREATE:
			if(initialize(wParam,lParam) == false)
			{
				SendMessage(m_hWnd,WM_CLOSE,NULL,NULL);
			}
			break;
		case WM_CLOSE:
			if(m_bSyncMode == TRUE){
				// 終了させない
			}else{
				//　終了時チェック
				DestroyWindow(m_hWnd);
			}
			break;
		case WM_DESTROY:
			unInitialize(wParam,lParam);
			PostQuitMessage(0);
			break;
		case	WM_SIZE:
			Size(wParam,lParam);
			break;
		case	WM_SETFOCUS:
			break;
		case WM_ERASEBKGND:
			break;
		case WM_COMMAND:
			command(wParam,lParam);
			break;
		case WM_PAINT:
			Paint(wParam,lParam);
			break;
		case WM_NOTIFY:
			break;
		case WM_DROPFILES:
			dropFile(wParam,lParam);
			InvalidateRect(m_hWnd,NULL,FALSE);
			break;
		case WM_KEYDOWN:
			if((GetKeyState(VK_LCONTROL) & 0x8000) == 0x8000 || (GetKeyState(VK_RCONTROL) & 0x8000) == 0x8000){
				if(wParam == 'C'){
					Copy();
					break;
				}else if(wParam == 'V'){
					Paste();
					break;
				}
			}
			if(m_pEditor){
				if(m_pEditor->KeyDown(wParam,lParam)){
					SetWindowText(m_hStatusBar,"");
					InvalidateRect(m_hWnd,NULL,FALSE);
				}
			}
			break;
		case WM_KEYUP:
			if(m_pEditor){
				if(m_pEditor->KeyUp(wParam,lParam)){
					InvalidateRect(m_hWnd,NULL,FALSE);
				}
			}
			break;
		case WM_MOUSEWHEEL:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONUP:
			// マウス関連の処理をエディターへ送る
			if (m_pEditor) {
				if (m_pEditor->MouseEvent(msg, wParam, lParam)) {
					InvalidateRect(m_hWnd, NULL, FALSE);
				}
			}
			break;
		case TONE_EDIT_MSG_SET_TONEDATA:
			// トーンパラメータを設定する
			SetToneParam(wParam,lParam);
			InvalidateRect(m_hWnd,NULL,FALSE);
			break;
		case TONE_EDIT_MSG_SET_ORGTONE:
			if (m_pEditor) {
				// トーンパラメータを設定する
				m_pEditor->SetOrgToneData((VOICEFORMAT*)lParam);
				InvalidateRect(m_hWnd, NULL, FALSE);
			}
			break;
		case TONE_EDIT_MSG_SET_TONENAME:
			// トーンパラメータを設定する
			NameSet(wParam,lParam);
			break;
		case WM_TIMER:
			if (m_pEditor) {
				m_pEditor->Timer();
			}
			// タイマーのタイミングで画面を再描画する
			InvalidateRect(m_hWnd,NULL,FALSE);
				break;
		default:
			return DefWindowProc(m_hWnd,msg,wParam,lParam);
	}
	return lRet;
}

// 初期化
BOOL	MainWindow::initialize(WPARAM wParam,LPARAM lParam){
	// ステータスバーを生成する
	m_hStatusBar = CreateStatusWindow(WS_CHILD | WS_VISIBLE | CCS_BOTTOM | SBT_TOOLTIPS,"",m_hWnd,100);

	SetWindowText(m_hWnd, "FmToneEditor(Mucom Sync Mode)");

	Profile cProf;
	cProf.setModulePath(Application::app->GetInstance());	// モジュールのパスを設定する
	// 各種オプション項目取得
	m_bAgedeyesMode = cProf.getParam("option","AgedeyesMode");
	m_bMucom = cProf.getParam("option","mucom",TRUE);
	m_bMmldrv = cProf.getParam("option","mmldrv");
	m_bSCCI = cProf.getParam("option","SCCI");

	// ウィンドウサイズを設定する
	RECT rc;
	GetWindowRect(m_hStatusBar,&rc);
	int iStatusBarHeight = rc.bottom - rc.top;
	iStatusBarHeight += GetSystemMetrics(SM_CYMENU);
	if(m_bAgedeyesMode == TRUE){
		SetSize(WINDOW_WIDTH * 2,WINDOW_HEIGHT * 2 + iStatusBarHeight);
	}else{
		SetSize(WINDOW_WIDTH,WINDOW_HEIGHT + iStatusBarHeight);
	}

	// Dibを初期化する
	m_dispScale = m_bAgedeyesMode ? 2 : 1;
	m_pDib = new DibModule();
	m_pDib->initialize(WINDOW_WIDTH * m_dispScale, WINDOW_HEIGHT * m_dispScale);
	m_pDib->SetFont(12 * m_dispScale, "MS ゴシック");

	// チェックボックスを入れる
	HMENU hMenu = GetMenu(m_hWnd);
	CheckMenuItem(hMenu,ID_40004,MF_BYCOMMAND | (m_bAgedeyesMode ? MFS_CHECKED : MFS_UNCHECKED));
	CheckMenuItem(hMenu,ID_40002,MF_BYCOMMAND | (m_bMucom ? MFS_CHECKED : MFS_UNCHECKED));
	CheckMenuItem(hMenu,ID_40003,MF_BYCOMMAND | (m_bMmldrv ? MFS_CHECKED : MFS_UNCHECKED));
	CheckMenuItem(hMenu,ID_40005,MF_BYCOMMAND | (m_bSCCI ? MFS_CHECKED : MFS_UNCHECKED));

	// ドラッグ＆ドロップさせるモード
	DragAcceptFiles(m_hWnd,TRUE);

	// エディタクラス生成
	m_pEditor = new FmToneEditor();
	m_pEditor->Initialize(m_hWnd);
	m_pEditor->SetScale(m_dispScale);

	// テストデータ
	VOICEFORMAT param;
	ZeroMemory(&param,sizeof(param));
	param.ar_op1 = 31;
	param.dr_op1 = 18;
	param.sr_op1 = 0;
	param.rr_op1 = 6;
	param.sl_op1 = 2;
	param.tl_op1 = 36;
	param.ks_op1 = 0;
	param.ml_op1 = 10;
	param.dt_op1 = 3;

	param.ar_op2 = 31;
	param.dr_op2 = 14;
	param.sr_op2 = 4;
	param.rr_op2 = 6;
	param.sl_op2 = 2;
	param.tl_op2 = 45;
	param.ks_op2 = 0;
	param.ml_op2 = 0;
	param.dt_op2 = 3;

	param.ar_op3 = 31;
	param.dr_op3 = 10;
	param.sr_op3 = 4;
	param.rr_op3 = 6;
	param.sl_op3 = 2;
	param.tl_op3 = 18;
	param.ks_op3 = 1;
	param.ml_op3 = 0;
	param.dt_op3 = 3;

	param.ar_op4 = 31;
	param.dr_op4 = 10;
	param.sr_op4 = 3;
	param.rr_op4 = 6;
	param.sl_op4 = 2;
	param.tl_op4 = 0;
	param.ks_op4 = 1;
	param.ml_op4 = 0;
	param.dt_op4 = 3;

	param.fb = 7;
	param.al = 0;

	m_pEditor->SetToneData(param);

	// タイマーを開始する
	SetTimer(m_hWnd,0,20,NULL);

	// 初期化完了にする
	m_bInitialized	= TRUE;

	return true;
}

/// 開放
BOOL	MainWindow::unInitialize(WPARAM wParam,LPARAM lParam){

	// タイマーを終了する
	KillTimer(m_hWnd,0);

	// 現状のウィンドウサイズを取得する
	RECT rc;
	GetWindowRect(m_hWnd,&rc);

	Profile cProf;
	cProf.setModulePath(Application::app->GetInstance());	// モジュールのパスを設定する

	// オプションを保存する
	cProf.setParam("option","AgedeyesMode",m_bAgedeyesMode);
	cProf.setParam("option","mucom",m_bMucom);
	cProf.setParam("option","mmldrv",m_bMmldrv);
	cProf.setParam("option","SCCI",m_bSCCI);

	// エディタクラスの解放処理
	m_pEditor->UnInitialize();

	if(m_hStatusBar){
		DestroyWindow(m_hStatusBar);
		m_hStatusBar = NULL;
	}
	// DIBの削除
	if(m_pDib){
		delete m_pDib;
		m_pDib = NULL;
	}
	if(m_pEditor){
		delete m_pEditor;
		m_pEditor = NULL;
	}
	return TRUE;
}

// ---------- 画面描画 ----------
BOOL	MainWindow::Paint(WPARAM wParam,LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hWnd,&ps);
	// エディタクラスの描画処理を行う
	if(m_pEditor){
		m_pEditor->Paint(m_pDib);
	}
	// DIBをウィンドウに表示する
	m_pDib->paint(hdc);
	EndPaint(m_hWnd,&ps);
	return TRUE;
}

//	---------- サイズ ----------
BOOL	MainWindow::Size(WPARAM wParam,LPARAM lParam){
	// ステータスバーのサイズを取得する
	int	iStatusBarHeight = 0;
	if(m_hStatusBar != NULL){
		RECT rc;
		GetWindowRect(m_hStatusBar,&rc);
		iStatusBarHeight = rc.bottom - rc.top;
	}
	if(m_hStatusBar != NULL){
		SendMessage(m_hStatusBar,WM_SIZE,wParam,lParam);
	}
	return	TRUE;
}

// ---------- メニュー関連 ----------
BOOL	MainWindow::command(WPARAM wParam,LPARAM lParam){
	HMENU hMenu = GetMenu(m_hWnd);
	switch(LOWORD(wParam)){
		case ID_40001:
			SendMessage(m_hWnd,WM_CLOSE,NULL,NULL);
			break;
		case ID_40002:	// MUCOMモード
			m_bMucom = TRUE;
			m_bMmldrv = FALSE;
			CheckMenuItem(hMenu,ID_40002,MF_BYCOMMAND | (m_bMucom ? MFS_CHECKED : MFS_UNCHECKED));
			CheckMenuItem(hMenu,ID_40003,MF_BYCOMMAND | (m_bMmldrv ? MFS_CHECKED : MFS_UNCHECKED));
			break;
		case ID_40003:	// mmldrvモード
			m_bMucom = FALSE;
			m_bMmldrv = TRUE;
			CheckMenuItem(hMenu,ID_40002,MF_BYCOMMAND | (m_bMucom ? MFS_CHECKED : MFS_UNCHECKED));
			CheckMenuItem(hMenu,ID_40003,MF_BYCOMMAND | (m_bMmldrv ? MFS_CHECKED : MFS_UNCHECKED));
			break;
		case ID_40004:	// 老眼モード
			if(m_bAgedeyesMode){
				m_bAgedeyesMode = FALSE;
			}else{
				m_bAgedeyesMode = TRUE;
			}
			CheckMenuItem(hMenu,ID_40004,MF_BYCOMMAND | (m_bAgedeyesMode ? MFS_CHECKED : MFS_UNCHECKED));
			{
				// DIBの再生成を実施する
				m_pDib->uninitialize();
				delete m_pDib;
				m_dispScale = m_bAgedeyesMode ? 2 : 1;
				m_pDib = new DibModule();
				m_pDib->initialize(WINDOW_WIDTH * m_dispScale, WINDOW_HEIGHT * m_dispScale);
				m_pDib->SetFont(12 * m_dispScale, "MS ゴシック");
				RECT rc;
				GetWindowRect(m_hStatusBar,&rc);
				int iStatusBarHeight = rc.bottom - rc.top;
				iStatusBarHeight += GetSystemMetrics(SM_CYMENU);
				if(m_bAgedeyesMode == TRUE){
					SetSize(WINDOW_WIDTH * 2,WINDOW_HEIGHT * 2 + iStatusBarHeight);
				}else{
					SetSize(WINDOW_WIDTH,WINDOW_HEIGHT + iStatusBarHeight);
				}
				m_pEditor->SetScale(m_dispScale);
				InvalidateRect(m_hWnd,NULL,false);
			}
			break;
		case ID_40005:
			if(m_bSCCI){
				m_bSCCI = FALSE;
			}else{
				m_bSCCI = TRUE;
			}
			CheckMenuItem(hMenu,ID_40005,MF_BYCOMMAND | (m_bSCCI ? MFS_CHECKED : MFS_UNCHECKED));
			break;
		case ID_40006:
			// 保存処理
			if (m_pEditor){
				m_pEditor->SaveDatFiele(m_hWnd);
			}
			break;
	}
	return TRUE;
}

// システムコマンドの処理
LONG_PTR	MainWindow::syscommand(WPARAM wParam,LPARAM lParam){
	return DefWindowProc(m_hWnd,WM_SYSCOMMAND,wParam,lParam);;
}


// ドロップファイル処理
BOOL	MainWindow::dropFile(WPARAM wParam,LPARAM lParam){
	char	FileName[MAX_PATH];
	DWORD	fileSize = 0;
	// ファイル名を取得する
	DragQueryFile((HDROP)wParam,0,FileName,sizeof(FileName));

	// 同期モード以外ではドラッグを有効化する
	if(m_bSyncMode == FALSE){
		// ファイルを読み込む
		void *pFile = MemLoad(FileName,&fileSize);
		// 今のところ実装無しにする
		if(fileSize == 8192){
			string fileName  = FileName;
			fileName = fileName.substr(fileName.rfind("\\") + 1);
			// 音色データファイルとして認識する
			m_pEditor->SetDatFile(fileName,reinterpret_cast<VOICEFORMAT*>(pFile));
			// 設定した音色データのインフォメーションを表示
			char strBuff[MAX_PATH + 256];
			wsprintf(strBuff,"音色データ「%s」を読み込みました。",fileName.c_str());
			SetWindowText(m_hStatusBar,strBuff);
		}
		// メモリを開放する
		free(pFile);
	}
	// ドラッグを終了させる
	DragFinish((HDROP)wParam);

	// ここで、ファイルを読み込んでテキストに設定する
	return TRUE;
}

// コピー処理
BOOL	MainWindow::Copy(){
	HGLOBAL hMem;
	char	*pText = new char[1024];
	// クリア
	ZeroMemory(pText,1024);
	// ここで、出力する音色データを作りこむ
	VOICEFORMAT param = m_pEditor->GetToneData();
	// 音色番号を取得する
	m_iToneNo = m_pEditor->GetToneNo();
	// パラメータを取得する
	ToneParam toneParam;
	if(m_bMucom == TRUE){
		lstrcpy(pText, toneParam.CopyMucomParams(param, m_iToneNo).c_str());
	}else if(m_bMmldrv == TRUE){
		lstrcpy(pText, toneParam.CopyMmldrvParams(param, m_iToneNo).c_str());
	}

	// クリップボードにテキストを格納する
	LPSTR lpBuff;
	hMem = GlobalAlloc((GHND | GMEM_SHARE),lstrlen(pText) + 1);
	if(hMem != NULL){
		lpBuff = (LPSTR)GlobalLock(hMem);
		if(lpBuff != NULL){
			// 共有メモリにデータをコピーする
			lstrcpy(lpBuff,pText);
			GlobalUnlock(hMem);
			// クリップボードを開く
			if( OpenClipboard(m_hWnd) ){
				// クリップボードをクリアする
				EmptyClipboard();
				SetClipboardData(CF_TEXT,hMem);
				CloseClipboard();
			}
			// スタータスバーにメッセージを表示する
			if(m_bMucom){
				SetWindowText(m_hStatusBar,"音色データをMUCOM形式のテキストでクリップボードへコピーしました。");
			}else if(m_bMmldrv){
				SetWindowText(m_hStatusBar,"音色データをmmldrv形式のテキストでクリップボードへコピーしました。");
			}
		}
	}
	return TRUE;
}

// ペースト処理
BOOL	MainWindow::Paste(){
	// クリップボードからパラメータ受け取る
	// クリップボードにテキストがあるかチェックする
	BOOL bRet = IsClipboardFormatAvailable(CF_TEXT);
	if(bRet == FALSE){
		SetWindowText(m_hStatusBar, "");
		return FALSE;
	}
	// クリップボードを開く
	bRet = OpenClipboard(m_hWnd);
	if(bRet == FALSE){
		return FALSE;
	}
	// クリップボードデータを取得
	HGLOBAL hGlobal = (HGLOBAL)GetClipboardData(CF_TEXT);
	if(hGlobal == NULL){
		CloseClipboard();
		return FALSE;
	}
	// クリップボードの情報を取得
	LPSTR pClipText = (LPSTR)GlobalLock(hGlobal);
	if(pClipText == NULL){
		GlobalUnlock(hGlobal);
		CloseClipboard();
		return FALSE;
	}
	// 波形データか解析を行う
	std::string str = pClipText;

	// クリップボードは閉じる
	GlobalUnlock(hGlobal);
	CloseClipboard();

	// 形式チェック
	ToneParam toneParam;
	int iType = toneParam.CheckParam(str);
	if (iType == TYPE_NONE) {
		SetWindowText(m_hStatusBar, "クリップボードのデータは貼り付けが行えるテキストではありません。");
		return FALSE;
	}
	VOICEFORMAT param;
	string statusMsg = "";
	switch(iType){
		case	TYPE_MUCOM:
			bRet = GetMucomParameter(str,param);
			if(bRet){
				statusMsg = "MUCOM形式の音色データから設定されました。";
			}
			break;
		case	TYPE_MMLDRV:
			bRet = GetMmldrvParameter(str,param);
			if(bRet){
				statusMsg = "mmldrv形式の音色データから設定されました。";
			}
			break;
	}
	ToneConfirm toneConfirm;
	char name[7];
	int iToneNo = m_pEditor->GetToneNo();
	if (iToneNo < 0 || iToneNo > 255) {
		return FALSE;
	}
	memset(name, 0x00, 7);
	strncpy_s(name, param.name, 6);
	toneConfirm.SetToneNo(iToneNo);
	toneConfirm.SetToneName(name);
	// ペーストするかの確認を行う
	long lResult = toneConfirm.Dialog(ApplicationBase::app->GetInstance(), MAKEINTRESOURCE(IDD_DIALOG1),m_hWnd);
	// OKが選択された場合
	if (lResult == IDOK) {
		// 名前をダイアログから取得する
		string sName = toneConfirm.GetToneName();
		strncpy_s(param.name, sName.c_str(), 6);
		m_pEditor->SetToneData(param, -1);
		// 画面更新を指示する
		InvalidateRect(m_hWnd, NULL, FALSE);
	}

	return TRUE;
}

// 名前を設定する
BOOL MainWindow::NameSet(WPARAM wParam, LPARAM lParam) {
	ToneName toneName;
	toneName.SetToneName((char*)wParam);
	long lResult = toneName.Dialog(ApplicationBase::app->GetInstance(), MAKEINTRESOURCE(IDD_DIALOG2), m_hWnd);
	// OKが選択された場合
	if (lResult == IDOK) {
		// 名前をダイアログから取得する
		string sName = toneName.GetToneName();
		m_pEditor->SetToneName(sName);
		// 画面更新を指示する
		InvalidateRect(m_hWnd, NULL, FALSE);
	}
	return TRUE;
}


// MUCOMのパラメータを取得する
BOOL	MainWindow::GetMucomParameter(std::string str,VOICEFORMAT &param){
	ToneParam toneParam;
	return toneParam.GetMucomParameter(str, param, m_iToneNo);
}

// mmldrvのパラメータを取得する
BOOL	MainWindow::GetMmldrvParameter(std::string str,VOICEFORMAT &param){
	ToneParam toneParam;
	return toneParam.GetMmldrvParameter(str, param, m_iToneNo);
}

// トーンパラメータを設定する
BOOL	MainWindow::SetToneParam(WPARAM wParam,LPARAM lParam){
	// ファイルを読み込む
	void *pFile = reinterpret_cast<void*>(lParam);
	string fileName  = reinterpret_cast<char*>(wParam);
	fileName = fileName.substr(fileName.rfind("\\") + 1);
	// 音色データファイルとして認識する
	m_pEditor->SetDatFile(fileName,reinterpret_cast<VOICEFORMAT*>(pFile));
	return TRUE;
}

