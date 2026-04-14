#include "editor.h"
#include "../resource.h"
#include "../window/windowbase.h"
#include "../window/mainwindow.h"
#include "../util/fileutil.h"
#include "../util/profile.h"
#include "ToneParam.h"

#pragma comment(lib, "winmm.lib")

#define RATE 55467				// Sampling Rate 55K
#define BUFSIZE 200				// Stream Buffer 200ms
#define baseclock 7987200		// Base Clock

// 固定リソース
LPCSTR FmToneEditor::m_cAlList[8] = {
	MAKEINTRESOURCE(IDB_BITMAP2),
	MAKEINTRESOURCE(IDB_BITMAP3),
	MAKEINTRESOURCE(IDB_BITMAP4),
	MAKEINTRESOURCE(IDB_BITMAP5),
	MAKEINTRESOURCE(IDB_BITMAP6),
	MAKEINTRESOURCE(IDB_BITMAP7),
	MAKEINTRESOURCE(IDB_BITMAP8),
	MAKEINTRESOURCE(IDB_BITMAP9)
};

// レンジ用パラメータ
int FmToneEditor::m_iRange[10][4][2] = {
	{{0,31},{0,31},{0,31},{0,31}},
	{{0,31},{0,31},{0,31},{0,31}},
	{{0,31},{0,31},{0,31},{0,31}},
	{{0,15},{0,15},{0,15},{0,15}},
	{{0,15},{0,15},{0,15},{0,15}},
	{{0,127},{0,127},{0,127},{0,127}},
	{{0,3},{0,3},{0,3},{0,3}},
	{{0,15},{0,15},{0,15},{0,15}},
	{{0,7},{0,7},{0,7},{0,7}},
	{{0,7},{0,7},{0,7},{0,7}}
};


// コンストラクタ
FmToneEditor::FmToneEditor(){
	// 変数の初期化など
	m_pFont = NULL;
	ZeroMemory(&m_pAlImage,sizeof(m_pAlImage));
	ZeroMemory(&m_VoiceData,sizeof(m_VoiceData));
	m_iToneNo = 0;

	// 選択位置
	m_SelX = 0;
	m_SelY = 0;
	m_MouseSelX = -1;
	m_MouseSelY = -1;
	// 鍵盤
	m_DownKey = 0x00;
	// 入力中
	m_bInputParam = FALSE;
	m_bInputMouseFarst = FALSE;
	m_bInputFarst = FALSE;
	// 編集中文字列
	ZeroMemory(m_cDispParam,sizeof(m_cDispParam));
	// 入力中のカーソルポジション
	m_iInputPos = 0;		// 後ろから？

	// サウンド関連の初期化
	m_Octave = 3;
	sending = false;
	playflag = false;

	// リアルチップ関連
	m_bUseScci = FALSE;
	m_pChip = NULL;

	// DATファイル関連
	m_sCurrentPath = "";
	m_sDatFileName = "";
	m_sCurrentPath = "";
	m_pVoiceData = NULL;
	m_pOrgVoiceData = NULL;
	m_vMmlToneList.clear();

	// デバッグ用
	m_iDebugCount = 0;

	// ボリューム
	m_iVolume = 15;

	// CH毎の再生中音色
	for(int i = 0; i < 6; i++){
		m_iChToneNum[i] = -1;
	}
	// コールバック関係クリア
	m_pVmCallback = NULL;
	m_pCallbackParam = NULL;
	m_TimerId = 0;

	// 倍率設定
	m_iScale = 1;
	// ターゲットトラック
	m_iTargetTrack = 0;

	// ブリンク関連
	m_iBlink = 1;
	m_iBlinkTimer = 12;

	// オフセット
	m_octOffset = -2;

}

// デストラクタ
FmToneEditor::~FmToneEditor(){
	// 基本記載は無いはず
}

// SCCI設定
void	FmToneEditor::SetUseSCCI(BOOL bScci){
	m_bUseScci = bScci;
}

// 初期化
BOOL FmToneEditor::Initialize(HWND hWnd){
	// オーナーウィンドウハンドルの記憶
	m_hOwnerWin = hWnd;
	// 必要なリソース等の読込み
	// フォントデータ読込み
	if(m_pFont == NULL){
		m_pFont = new GraphicData();
		HRSRC hRes = FindResource(ApplicationBase::app->GetInstance(),MAKEINTRESOURCE(IDB_BITMAP1),RT_RCDATA);
		m_pFont->loadBitmapResource(ApplicationBase::app->GetInstance(),hRes);
		m_pFont->setColorKeyPos(0,0);
	}
	// アルゴリズム用イメージ読込み
	for(int i = 0; i < 8; i++){
		if(m_pAlImage[i] == NULL){
			m_pAlImage[i] = new GraphicData();
			HRSRC hRes = FindResource(ApplicationBase::app->GetInstance(),m_cAlList[i],RT_RCDATA);
			m_pAlImage[i]->loadBitmapResource(ApplicationBase::app->GetInstance(),hRes);
			m_pAlImage[i]->setColorKeyPos(0,0);
		}
	}
	// 実チップの使用
	m_pChip = new realchip();
	m_pChip->Initialize();

	// サウンド周りの初期化
	SetRegister(0x2d, 0);
	SetRegister(0x27, 0);

	playflag = true;
	// クリティカルセッション初期化
	InitializeCriticalSection(&cs);
	// タイマー精度を1msecに設定
	timeBeginPeriod(1);
	// キー情報を初期化する
	keyInitialize();
	// MIDI開く
	m_midiIn.open(0, (LPSTR)"", (DWORD_PTR)FmToneEditor::midiCallBack, (DWORD_PTR)this);
	// MIDI制御用のスレッドを起動する
	m_TimerId = timeSetEvent(1,1,(LPTIMECALLBACK)FmToneEditor::vTimerCallback,(LONG_PTR)this,TIME_PERIODIC);
	return TRUE;
}

// 開放
BOOL FmToneEditor::UnInitialize(){
	// スレッドを停止
	timeKillEvent(m_TimerId);
	DeleteCriticalSection(&cs);
	m_midiIn.close();
	if (playflag) {
		for (int i = 0; i < 300 && sending; i++) Sleep(10);
		playflag = false;
	}
	// 開放処理
	// フォント開放
	if(m_pFont != NULL){
		delete m_pFont;
		m_pFont = NULL;
	}
	for(int i = 0; i < 8; i++){
		if(m_pAlImage[i] != NULL){
			delete m_pAlImage[i];
			m_pAlImage[i] = NULL;
		}
	}
	CoUninitialize();
	// サウンドチップの開放
	if(m_pChip){
		m_pChip->UnInitialize();
		delete m_pChip;
		m_pChip = NULL;
	}
	// オリジナルのデータ
	if(m_pOrgVoiceData){
		delete [] m_pOrgVoiceData;
		m_pOrgVoiceData = NULL;
	}
	// DATファイルがあったら開放する
	if(m_pVoiceData){
		delete [] m_pVoiceData;
		m_pVoiceData = NULL;
	}
	return TRUE;
}

// フォント表示
void	FmToneEditor::DrawFont(DibModule *pDib,int x,int y,DWORD dColor,char *pFormat,...){
	va_list ap;
	char	drawStr[1024];
	// フォントが無ければ終了
	if(m_pFont == NULL){
		return;
	}
	if(pDib == NULL){
		return;
	}
	//	---------print main---------
	va_start(ap, pFormat);
	wvsprintf(drawStr, pFormat, ap);
	va_end(ap);
	// 描画する
	char *pStr = drawStr;
	// ループ
	pDib->drawFont(x,y,drawStr);
}

// 描画処理
void FmToneEditor::Paint(DibModule *pDib){
	// フォントを設定する
	pDib->SetFont(12 * m_iScale,"MSゴシック");

	// 背景を消去する
	pDib->fillBox(0, 0, 640 * m_iScale, 480 * m_iScale, DIBMODULE_RGB(0x00,0x00,0x40));
	
	// パラメータ上のカーソル表示
	DrawCursor(pDib);

	// パラメータ表示
	DrawParamaters(pDib);

	// アルゴリズムの表示
	DrawAlgorithm(pDib);

	// エンベロープ系の表示
	DrawEnvelope(pDib);

	// 直接入力時の操作
	DrawInputBox(pDib);
	// 下部のセパレータ
	pDib->line(0 * m_iScale, 400 * m_iScale - 1, 640 * m_iScale - 1, 400 * m_iScale - 1, DIBMODULE_RGB(0x40, 0x40, 0x40));

	// 同期モードの場合は鍵盤を表示しない
	DrawKeyboard(pDib);

	// インフォメーション表示
	DrawIntormation(pDib);

	// プレビューインフォメーション表示
	DrawPreviewInformation(pDib);

}

// カーソル表示
void	FmToneEditor::DrawCursor(DibModule *pDib){
	// 表示位置にカーソルを表示
	int x = 128 + 16 + m_SelX * 24;
	int y = 40 + m_SelY * 32; 

	// カーソル位置の背景を出す（横線）
	pDib->fillBox(16 * m_iScale,y * m_iScale,266 * m_iScale,12 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	// 縦線
	pDib->fillBox(x * m_iScale,18 * m_iScale,18 * m_iScale,328 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));

	// カーソルを出してみる
	pDib->fillBox(x * m_iScale,y * m_iScale,18 * m_iScale,12 * m_iScale, DIBMODULE_RGB(96,96,128));

	// マウスカーソルが存在する場合
	if (m_MouseSelX >= 0 && m_MouseSelY >= 0) {
		x = 128 + 16 + m_MouseSelX * 24;
		y = 40 + m_MouseSelY * 32;
		pDib->fillBox(x * m_iScale, y * m_iScale, 18 * m_iScale, 12 * m_iScale, DIBMODULE_RGB(128, 128, 64));
	}
}

// パラメータ部分の表示
void	FmToneEditor::DrawParamaters(DibModule *pDib){
	// OPの表示
	int iPos = 8;
	int iAlign = 16;
	int ihOffset = 128;
	// パラメータ部分見出し
	pDib->setFontColor(0x00ffffff);
	pDib->drawFont(8 * m_iScale,iPos * m_iScale,"FM Paramaters");

	// 各部見出し
	pDib->line((iAlign + 112)  * m_iScale,0 * m_iScale,(iAlign + 112) * m_iScale,360 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));				// チェック用ガイドライン
	iPos += 16;
	pDib->setFontColor(DIBMODULE_RGB(192, 192, 255));
	pDib->drawFont((iAlign + ihOffset)  * m_iScale,iPos * m_iScale, "OP1 OP2 OP3 OP4   Range");
	iPos += 16;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "Atack Rate                             0- 31");
	iPos += 32;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "Decay Rate                             0- 31");
	iPos += 32;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "Sustine Rate                           0- 31");
	iPos += 32;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "Release Rate                           0- 15");
	iPos += 32;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "Sustin Level                           0- 15");
	iPos += 32;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "Total Level                            0-127");
	iPos += 32;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "KeyScale Rate                          0-  3");
	iPos += 32;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "Multiple                               0- 15");
	iPos += 32;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "Detune                                 0-  7");
	iPos += 32;
	pDib->drawFont(iAlign * m_iScale,iPos * m_iScale, "Feedback/Algorithm                     0-  7");

	// パラメータの表示
	iPos = 40;
	pDib->setFontColor(DIBMODULE_RGB(255, 255, 255));
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d %3d %3d %3d",m_VoiceData.ar_op1,m_VoiceData.ar_op2,m_VoiceData.ar_op3,m_VoiceData.ar_op4);
	iPos += 32;
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d %3d %3d %3d",m_VoiceData.dr_op1,m_VoiceData.dr_op2,m_VoiceData.dr_op3,m_VoiceData.dr_op4);
	iPos += 32;
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d %3d %3d %3d",m_VoiceData.sr_op1,m_VoiceData.sr_op2,m_VoiceData.sr_op3,m_VoiceData.sr_op4);
	iPos += 32;
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d %3d %3d %3d",m_VoiceData.rr_op1,m_VoiceData.rr_op2,m_VoiceData.rr_op3,m_VoiceData.rr_op4);
	iPos += 32;
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d %3d %3d %3d",m_VoiceData.sl_op1,m_VoiceData.sl_op2,m_VoiceData.sl_op3,m_VoiceData.sl_op4);
	iPos += 32;
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d %3d %3d %3d",m_VoiceData.tl_op1,m_VoiceData.tl_op2,m_VoiceData.tl_op3,m_VoiceData.tl_op4);
	iPos += 32;
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d %3d %3d %3d",m_VoiceData.ks_op1,m_VoiceData.ks_op2,m_VoiceData.ks_op3,m_VoiceData.ks_op4);
	iPos += 32;
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d %3d %3d %3d",m_VoiceData.ml_op1,m_VoiceData.ml_op2,m_VoiceData.ml_op3,m_VoiceData.ml_op4);
	iPos += 32;
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d %3d %3d %3d",m_VoiceData.dt_op1,m_VoiceData.dt_op2,m_VoiceData.dt_op3,m_VoiceData.dt_op4);
	iPos += 32;
	pDib->drawFont((iAlign + ihOffset) * m_iScale,iPos * m_iScale,"%3d --- %3d ---",m_VoiceData.fb,m_VoiceData.al);

}

// アルゴリズム表示
void	FmToneEditor::DrawAlgorithm(DibModule *pDib){
	int ixOffset = 24;
	// ラベル部分の表示
	pDib->setFontColor(DIBMODULE_RGB(255, 255, 255));
	pDib->drawFont((470 + ixOffset) * m_iScale,24 * m_iScale,"Algorithm");
	// アルゴリズムの画を表示
	pDib->draw((430 + ixOffset)  * m_iScale, 36 * m_iScale, m_pAlImage[m_VoiceData.al]->m_iWidth * m_iScale, m_pAlImage[m_VoiceData.al]->m_iHeight * m_iScale,
		m_pAlImage[m_VoiceData.al],0,0,m_pAlImage[m_VoiceData.al]->m_iWidth, m_pAlImage[m_VoiceData.al]->m_iHeight);

	// 枠線の表示
	pDib->line((424 + ixOffset) * m_iScale,30 * m_iScale,(424 + ixOffset) * m_iScale,190 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	pDib->line((424 + ixOffset) * m_iScale,190 * m_iScale,(594 + ixOffset) * m_iScale,190 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	pDib->line((594 + ixOffset) * m_iScale,190 * m_iScale,(594 + ixOffset) * m_iScale,30 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	// 文字の両脇
	pDib->line((424 + ixOffset) * m_iScale,30 * m_iScale,(466 + ixOffset) * m_iScale,30 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	pDib->line((528 + ixOffset) * m_iScale,30 * m_iScale,(594 + ixOffset) * m_iScale,30 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));

}

// エンベロープの表示
void	FmToneEditor::DrawEnvelope(DibModule *pDib){
	int iPos = 16;
	// ラベル部分の表示
	pDib->setFontColor(DIBMODULE_RGB(255, 255, 255));
	pDib->drawFont(300 * m_iScale,24 * m_iScale,"Envelope");
	// OP1の表示
	iPos += 32;
	DrawEnbelopleWave(pDib,300,iPos,"OP1",m_VoiceData.ar_op1,m_VoiceData.dr_op1,m_VoiceData.sr_op1,m_VoiceData.rr_op1,m_VoiceData.sl_op1);
	iPos += 75;
	// OP3の表示
	DrawEnbelopleWave(pDib,300,iPos,"OP2",m_VoiceData.ar_op2,m_VoiceData.dr_op2,m_VoiceData.sr_op2,m_VoiceData.rr_op2,m_VoiceData.sl_op2);
	iPos += 75;
	// OP3の表示
	DrawEnbelopleWave(pDib,300,iPos,"OP3",m_VoiceData.ar_op3,m_VoiceData.dr_op3,m_VoiceData.sr_op3,m_VoiceData.rr_op3,m_VoiceData.sl_op3);
	iPos += 75;
	// OP4の表示
	DrawEnbelopleWave(pDib,300,iPos,"OP4",m_VoiceData.ar_op4,m_VoiceData.dr_op4,m_VoiceData.sr_op4,m_VoiceData.rr_op4,m_VoiceData.sl_op4);
	// 枠線の表示
	pDib->line(292 * m_iScale,30 * m_iScale,292 * m_iScale,348 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	pDib->line(292 * m_iScale,348 * m_iScale,428 * m_iScale,348 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	pDib->line(428 * m_iScale,348 * m_iScale,428 * m_iScale,30 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	// 文字の両脇
	pDib->line(292 * m_iScale,30 * m_iScale,298 * m_iScale,30 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	pDib->line(348 * m_iScale,30 * m_iScale,428 * m_iScale,30 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
}

// エンベロープ波形表示
void	FmToneEditor::DrawEnbelopleWave(DibModule *pDib,int x,int y,LPCSTR name,int ar,int dr,int sr,int rr,int sl){
	int	xs,ys,xe,ye;
	// ARの描画
	xs = x;
	ys = y + 63;
	xe = xs + 1 + (31 - ar);
	ye = y;
	pDib->line(xs * m_iScale,ys * m_iScale,xe * m_iScale,ye * m_iScale, DIBMODULE_RGB(0,255,0));
	xs = xe;
	ys = ye;
	// DR/SL
	xe = xs + (31 - dr);
	ye = y + sl * 4;
	pDib->line(xs * m_iScale,ys * m_iScale,xe * m_iScale,ye * m_iScale, DIBMODULE_RGB(0,255,0));
	xs = xe;
	ys = ye;
	// SR
	xe = xs + 32;
	ye = ys + (sr / 2);
	if(ye > (63 + y)) ye = 63 + y;
	pDib->line(xs * m_iScale,ys * m_iScale,xe * m_iScale,ye * m_iScale, DIBMODULE_RGB(0,255,0));
	xs = xe;
	ys = ye;
	// RR
	xe = xs + (30 - (rr * 2));
	ye = y + 63;
	pDib->line(xs * m_iScale,ys * m_iScale,xe * m_iScale,ye * m_iScale, DIBMODULE_RGB(255,0,0));
	// 名前の表示
	pDib->setFontColor(DIBMODULE_RGB(192, 192, 160));
	pDib->drawFont((x + 108) * m_iScale, y * m_iScale,name);
}

// 直接入力時の表示
void FmToneEditor::DrawInputBox(DibModule *pDib){
	// 入力モードで無い場合は処理を行わない
	if(m_bInputParam == FALSE) return;
	// 入力用のボックスを表示する
	int x = 128 + 16 + m_SelX * 24;
	int y = 40 + m_SelY * 32; 

	// 背景
	pDib->fillBox((x - 1) * m_iScale,(y - 1) * m_iScale,(18 + 2) * m_iScale,(12 + 2)  * m_iScale, DIBMODULE_RGB(128,255,255));
	// テキスト表示部分
	pDib->fillBox(x * m_iScale,y * m_iScale,18 * m_iScale,12 * m_iScale, DIBMODULE_RGB(255,255,255));
	// 初回入力で何も入力されて無い場合はハイライトで表示h当時する
	if (m_bInputFarst) {
		pDib->fillBox((x + 1) * m_iScale, (y + 1) * m_iScale, (6 * (int)strlen(m_cDispParam) - 1) * m_iScale, 10 * m_iScale, DIBMODULE_RGB(72, 255, 255));
	}
	// フォント表示
	pDib->setFontColor(DIBMODULE_RGB(0, 0, 0));
	pDib->drawFont(x * m_iScale,y * m_iScale,"%s",m_cDispParam);

	// キャレット表示
	if (m_iBlink == 1) {
		pDib->fillBox((x + m_iInputPos * 6)  * m_iScale, (y + 1) * m_iScale, 1 * m_iScale, 10 * m_iScale, DIBMODULE_RGB(0, 0, 255));
	}
	// キャレット点滅
	if(--m_iBlinkTimer == 0){
		m_iBlink = 1 - m_iBlink;
		m_iBlinkTimer = 12;
	}
}


// 鍵盤表示
void FmToneEditor::DrawKeyboard(DibModule *pDib){
	static const int noteWTable[7] = {0,2,4,5,7,9,11};
	static const int noteBTable[7] = {1,3,-1,6,8,10,-1};
	pDib->fillBox(0 * m_iScale,400 * m_iScale,12 * m_iScale,80 * m_iScale - 1, DIBMODULE_RGB(80,80,80));
	for(int j = 0; j < 8; j++){
		// 白鍵を表示
		for(int i = 0; i < 7; i++){
			if(isPushKeyboard(noteWTable[i],j)){
				pDib->fillBox((12 + (j * 77) + i * 11) * m_iScale,400 * m_iScale,11 * m_iScale - 1,80 * m_iScale - 1, DIBMODULE_RGB(255,128,128));
			}else{
				pDib->fillBox((12 + (j * 77) + i * 11) * m_iScale,400 * m_iScale,11 * m_iScale - 1,80 * m_iScale - 1, DIBMODULE_RGB(255,255,255));
			}
		}
		// 黒鍵を表示
		for(int i = 0; i < 7; i++){
			if(i == 2 || i == 6) continue;
			if(isPushKeyboard(noteBTable[i],j)){
				pDib->fillBox((12 + (j * 77) + (i * 11 + 7)) * m_iScale,400 * m_iScale,8 * m_iScale,45 * m_iScale, DIBMODULE_RGB(255,128,128));
			}else{
				pDib->fillBox((12 + (j * 77) + (i * 11 + 7)) * m_iScale,400 * m_iScale,8 * m_iScale,45 * m_iScale, DIBMODULE_RGB(0,0,0));
			}
		}
	}
	pDib->fillBox(628 * m_iScale,400 * m_iScale,12 * m_iScale,80 * m_iScale - 1, DIBMODULE_RGB(80,80,80));
	return;
}

boolean FmToneEditor::isPushKeyboard(int note,int oct){
	boolean isPush = false;
	for(int i = 0; i < 6; i++){
		if(m_midiChannel[i].bKeyOn){
			int playOct = (m_midiChannel[i].note / 12) + m_octOffset;
			int playNote = m_midiChannel[i].note % 12;
			if(playOct == oct && playNote == note){
				isPush = true;
				break;
			}
		}
	}
	return isPush;
}

// インフォメーション表示
void FmToneEditor::DrawIntormation(DibModule *pDib){
	int ixOffset = 20;
	int iPos = 196;
	// パラメータ部分見出し
	pDib->setFontColor(DIBMODULE_RGB(255, 255, 255));
	pDib->drawFont((420 + ixOffset) * m_iScale,iPos * m_iScale,"Operation Information");
	iPos += 12;
	pDib->setFontColor(DIBMODULE_RGB(192, 192, 255));
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Cursor     : Select Item position");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Page Up    : Value Up");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Page Down  : Value Down");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Enter      : Direct input");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"ESC        : Exit Direct input");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Ctrl+C     : Copy FM Param text");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Ctrl+V     : Paste FM Param text");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Ctrl+R     : Restore Tone Params");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Ctrl+N     : Set Tone Name");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Ctrl+Up    : ToneNo Up");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Ctrl+Down  : ToneNo Down");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Shift+Right: Octave Up");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Shift+Left : Octave Down");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Shift+Up   : Target Track Up");
		iPos += 12;
	pDib->drawFont((418 + ixOffset) * m_iScale,iPos * m_iScale,"Shift+Down : Target Track Down");
		iPos += 12;

	// 枠線の表示
	pDib->line((412 + ixOffset) * m_iScale,202 * m_iScale,(412 + ixOffset) * m_iScale,400 * m_iScale - 1, DIBMODULE_RGB(0x20,0x20,0x40));
	// 文字の両脇
	pDib->line((412 + ixOffset) * m_iScale,202 * m_iScale,(416 + ixOffset) * m_iScale,202 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	pDib->line((552 + ixOffset) * m_iScale,202 * m_iScale,640 * m_iScale - 1,202 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
}

// プレビューのインフォメーション
void	FmToneEditor::DrawPreviewInformation(DibModule *pDib){
	// ガイドラインを引く
	pDib->line(0 * m_iScale,360 * m_iScale,432 * m_iScale,360 * m_iScale, DIBMODULE_RGB(0x20,0x20,0x40));
	// 今のオクターブを表示する
	pDib->setFontColor(DIBMODULE_RGB(255, 255, 255));
	pDib->drawFont(8 * m_iScale,368 * m_iScale,"Peview Information");
	pDib->drawFont(8 * m_iScale,(368 + 16) * m_iScale,"Midi in Track : %d",m_iTargetTrack + 1);
	pDib->drawFont(120 * m_iScale,(368 + 16) * m_iScale,"Oct : %d",m_Octave + 1);
	// 音色データにフォーカスが当たっている場合は背景色を出す
	if (m_bIsToneNoSel) {
		pDib->fillBox(172 * m_iScale, 368 * m_iScale, 78 * m_iScale, 12 * m_iScale, DIBMODULE_RGB(128, 128, 64));
	}
	// トーンの番号とコメントがあれば表示する
	pDib->drawFont(172 * m_iScale,368 * m_iScale,"ToneNo : %d",m_iToneNo);

	char cName[7];
	memset(cName,0x00,7);
	memcpy(cName,m_VoiceData.name,0x06);

	if (m_bIsToneNameSel) {
		pDib->fillBox(172 * m_iScale, (368 + 16) * m_iScale, 90 * m_iScale, 12 * m_iScale, DIBMODULE_RGB(128, 128, 64));
	}
	pDib->drawFont(172 * m_iScale, (368 + 16) * m_iScale,"Name   : %s",cName);

	// DATファイルが読み込まれている場合
	if(m_pVoiceData){
		pDib->drawFont(274 * m_iScale,368 * m_iScale,"FileName : %s",m_sDatFileName.c_str());
	}
	if (m_pOrgVoiceData != NULL && m_pVoiceData != NULL) {
		VOICEFORMAT org, edit;
		edit = m_pVoiceData[m_iToneNo];
		edit.am_op1 = 1;
		edit.am_op2 = 1;
		edit.am_op3 = 1;
		edit.am_op4 = 1;
		org = m_pOrgVoiceData[m_iToneNo];
		org.am_op1 = 1;
		org.am_op2 = 1;
		org.am_op3 = 1;
		org.am_op4 = 1;
		if (memcmp(&edit, &org, sizeof(VOICEFORMAT)) != 0) {
			pDib->setFontColor(DIBMODULE_RGB(255, 64, 64));
			pDib->drawFont(274 * m_iScale, (368 + 16) * m_iScale,"This tone is editing");
		}
	}
}

// キー入力系
BOOL FmToneEditor::KeyDown(WPARAM wParam,LPARAM lParam){
	static const int iVolTbl[16] = {0,89,93,95,98,101,103,106,109,110,114,117,119,122,125,127};
	BOOL bRet = TRUE;
	// CTRL+の処理
	if((GetKeyState(VK_LCONTROL) & 0x8000) == 0x8000 || (GetKeyState(VK_RCONTROL) & 0x8000) == 0x8000 ){
		// DATが読み込まれている場合
		if(m_pVoiceData){
			// Page Up/Downを動作させる
			if(wParam == VK_UP){
				if(m_iToneNo < 255) m_iToneNo++;
				m_VoiceData = m_pVoiceData[m_iToneNo];
				SetToneParam(m_VoiceData);
				return bRet;
				}
			else if(wParam == VK_DOWN) {
				if(m_iToneNo > 0) m_iToneNo--;
				m_VoiceData = m_pVoiceData[m_iToneNo];
				SetToneParam(m_VoiceData);
				return bRet;
			}
			else if (wParam == 'R') {
				// 音色のリストア処理
				if (m_pOrgVoiceData != NULL && m_pVoiceData != NULL) {
					VOICEFORMAT org, edit;
					edit = m_pVoiceData[m_iToneNo];
					edit.am_op1 = 1;
					edit.am_op2 = 1;
					edit.am_op3 = 1;
					edit.am_op4 = 1;
					org = m_pOrgVoiceData[m_iToneNo];
					org.am_op1 = 1;
					org.am_op2 = 1;
					org.am_op3 = 1;
					org.am_op4 = 1;
					if (memcmp(&edit, &org, sizeof(VOICEFORMAT)) != 0) {
						// メッセージを表示する
						char str[256];
						wsprintf(str, "音色番号[%d]を編集前に戻しますか？", m_iToneNo);
						long iResult = MessageBox(m_hOwnerWin, str, "確認", MB_YESNO | MB_DEFBUTTON2);
						if (iResult == IDYES) {
							memcpy(&m_pVoiceData[m_iToneNo], &m_pOrgVoiceData[m_iToneNo], sizeof(VOICEFORMAT));
							m_VoiceData = m_pOrgVoiceData[m_iToneNo];
							SetToneParam(m_VoiceData);
						}
					}
				}
			}
			else if (wParam == 'N') {
				// 音色名変更
				char cName[7];
				memset(cName, 0x00, sizeof(cName));
				strncpy_s(cName, m_VoiceData.name, 6);
				// 名前入力処理へ
				SendMessage(m_hOwnerWin, TONE_EDIT_MSG_SET_TONENAME, (WPARAM)cName, NULL);
			}
		}
	// SHIFT+の処理
	} else if(((GetKeyState(VK_LSHIFT) & 0x8000) == 0x8000 || (GetKeyState(VK_RSHIFT) & 0x8000) == 0x8000)){
/*
		// DATが読み込まれている場合
		if(m_pVoiceData){
			// Page Up/Downを動作させる
			if(wParam == VK_UP){
				for(size_t i = 0; i < m_vTones.size(); i++){
					if(m_vTones[i] > m_iToneNo){
						m_iToneNo = m_vTones[i];
						break;
					}
				}
				m_VoiceData = m_pVoiceData[m_iToneNo];
				return bRet;
			}else if(wParam == VK_DOWN){
				for(int i = m_vTones.size() - 1; i >= 0; i--){
					if(m_vTones[i] < m_iToneNo){
						m_iToneNo = m_vTones[i];
						break;
					}
				}
				m_VoiceData = m_pVoiceData[m_iToneNo];
				return bRet;
			}
		}
*/
		if(wParam == VK_RIGHT) {
			if(m_Octave < 7){
				m_Octave++;
			}
			return bRet;
		}
		else if(wParam == VK_LEFT) {
			if(m_Octave > 0){
				m_Octave--;
			}
			return bRet;
		}
		else if(wParam == VK_UP) {
			if(m_iTargetTrack < 15) m_iTargetTrack++;
			// キーの初期化
			keyInitialize();
			SetToneParam(m_VoiceData);
			return bRet;
		}
		else if(wParam == VK_DOWN) {
			if(m_iTargetTrack > 0) m_iTargetTrack--;
			// キーの初期化
			keyInitialize();
			SetToneParam(m_VoiceData);
			return bRet;
		}
	}
	switch(wParam){
		case VK_PRIOR:
			if((GetKeyState(VK_LCONTROL) & 0x8000) == 0x8000 || (GetKeyState(VK_RCONTROL) & 0x8000) == 0x8000 ){
				if(m_iVolume < 15){
					m_iVolume++;
					SetVolume(iVolTbl[m_iVolume]);
				}
			}else{
				ChangeParam(1,m_SelX,m_SelY);
				SetToneParam(m_VoiceData);
			}
			break;
		case VK_NEXT:
			if((GetKeyState(VK_LCONTROL) & 0x8000) == 0x8000 || (GetKeyState(VK_RCONTROL) & 0x8000) == 0x8000 ){
				if(m_iVolume > 0){
					m_iVolume--;
					SetVolume(iVolTbl[m_iVolume]);
				}
			}else{
				ChangeParam(-1, m_SelX, m_SelY);
				SetToneParam(m_VoiceData);
			}
			break;
		case VK_UP:
			InputKey(wParam,lParam);
			if(m_SelY > 0 && m_bInputParam == FALSE){
				m_SelY--;
			}
			break;
		case VK_DOWN:
			InputKey(wParam,lParam);
			if(m_SelY < 9 && m_bInputParam == FALSE){
				m_SelY++;
			}
			break;
		case VK_LEFT:
			InputKey(wParam,lParam);
			if(m_SelX > 0 && m_bInputParam == FALSE) m_SelX--;
			break;
		case VK_RIGHT:
			InputKey(wParam,lParam);
			if(m_SelX < 3 && m_bInputParam == FALSE) m_SelX++;
			break;
			// 鍵盤用
		case 'Z':
		case 'S':
		case 'X':
		case 'C':
		case 'F':
		case 'V':
		case 'G':
		case 'B':
		case 'N':
		case 'J':
		case 'M':
		case 'K':
		case VK_OEM_COMMA:
		case 'L':
		case VK_OEM_PERIOD:
		case VK_OEM_2:
		case VK_OEM_1:
		case VK_OEM_102:
			// 直接入力じゃない且つ、KeyDownが違う場合
			if(m_bInputParam == FALSE && (lParam & 0x40000000) == 0){
				KeyOn(wParam);
			}
			break;
		case VK_ESCAPE:
			if(m_bInputParam == FALSE) break;
		case VK_RETURN:
			if(m_bInputParam == FALSE){
				InputStart();
			}else{
				InputEnd(wParam,lParam);
				SetToneParam(m_VoiceData);
			}
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case VK_NUMPAD0:
		case VK_NUMPAD1:
		case VK_NUMPAD2:
		case VK_NUMPAD3:
		case VK_NUMPAD4:
		case VK_NUMPAD5:
		case VK_NUMPAD6:
		case VK_NUMPAD7:
		case VK_NUMPAD8:
		case VK_NUMPAD9:
		case VK_BACK:
		case VK_DELETE:
			InputKey(wParam,lParam);
			break;
		default:
			bRet = FALSE;
	}
	return bRet;
}

// キー入力系
BOOL FmToneEditor::KeyUp(WPARAM wParam,LPARAM lParam){
	BOOL bRet = TRUE;
	KeyOff(wParam);
	return bRet;
}

BOOL FmToneEditor::MouseEvent(UINT msg, WPARAM wParam, LPARAM lParam) {
	BOOL bRet = FALSE;
	// マウスイベント
	if (msg == WM_MOUSEMOVE) {
		// マウス移動で座標情報を取得
		m_MousePosX = LOWORD(lParam);
		m_MousePosY = HIWORD(lParam);
		bRet = MouseParamHitCheck();
	}
	else if (msg == WM_LBUTTONUP) {
		bRet = MouseHitCheck();
		// ポジションの移動
		if (m_MouseSelX >= 0 && m_MouseSelY >= 0) {
			// マウスで指定したポジションへ移動させる
			if (m_bInputParam == TRUE) {
				if (m_SelX != m_MouseSelX &&
					m_SelY != m_MouseSelY) {
					m_bInputParam = FALSE;
				}
				else
				{
					// 座標をとる
					m_SelX = m_MouseSelX;
					m_SelY = m_MouseSelY;
					m_bInputParam = FALSE;

				}
			}
		} else if(m_bInputParam == TRUE)
		{
			m_bInputParam = FALSE;
			m_MouseSelX = -1;
			m_MouseSelY = -1;

		} else if (m_bIsToneNameSel) {
			char cName[7];
			memset(cName, 0x00, sizeof(cName));
			strncpy_s(cName, m_VoiceData.name, 6);
			// 名前入力処理へ
			SendMessage(m_hOwnerWin, TONE_EDIT_MSG_SET_TONENAME, (WPARAM)cName, NULL);
		}
	}
	else if (msg == WM_LBUTTONDBLCLK) {
		// マウスカーソルが選択位置にある場合
		if (m_MouseSelX >= 0 && m_MouseSelY >= 0) {
			// 直接入力箇所確定
			m_SelX = m_MouseSelX;
			m_SelY = m_MouseSelY;
			// 直接入力モードにする
			InputStart();
			// 直接入力モードになった場合パラメータ選択を解除する
			m_MouseSelX = -1;
			m_MouseSelY = -1;
			// 次のマウスイベントのクリックを無視
			m_bInputMouseFarst = TRUE;
		}
	}
	else if (msg == WM_MOUSEWHEEL) {
		// マウス入力キャンセル
		if (m_bInputParam) return TRUE;
		if (m_MouseSelX >= 0 &&  m_MouseSelY >= 0) {
			int iDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (iDelta > 0) {
				ChangeParam(1, m_MouseSelX, m_MouseSelY);
				SetToneParam(m_VoiceData);
			}
			else if(iDelta < 0){
				ChangeParam(-1, m_MouseSelX, m_MouseSelY);
				SetToneParam(m_VoiceData);
			}
		}
		else if (m_bIsToneNoSel) {
			int iDelta = GET_WHEEL_DELTA_WPARAM(wParam);
			if (iDelta > 0) {
				// 音色番号++
				if (m_iToneNo < 255) m_iToneNo++;
				if (m_pVoiceData) {
					m_VoiceData = m_pVoiceData[m_iToneNo];
				}
			}
			else if (iDelta < 0) {
				// 音色番号--
				if (m_iToneNo > 0) m_iToneNo--;
				if (m_pVoiceData) {
					m_VoiceData = m_pVoiceData[m_iToneNo];
				}
			}
			SetToneParam(m_VoiceData);
		}
	}
	return bRet;
}

BOOL FmToneEditor::MouseParamHitCheck() {
	BOOL bRet = FALSE;
	//int x = 128 + 16 + m_SelX * 24;
	//int y = 40 + m_SelY * 32;
	int x = 128 + 16;
	int y = 40;
	RECT rc;

	// FMトーンのパラメータエリアのチェック
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 10; j++) {
			rc.left = x + i * 24;
			rc.right = rc.left + 18;
			rc.top = y + j * 32;
			rc.bottom = rc.top + 12;
			// 倍率設定
			rc.left *= m_iScale;
			rc.right *= m_iScale;
			rc.top *= m_iScale;
			rc.bottom *= m_iScale;
			// ヒット判定
			if (m_MousePosX >= rc.left && m_MousePosX < rc.right &&
				m_MousePosY >= rc.top && m_MousePosY < rc.bottom) {
				m_MouseSelX = i;
				m_MouseSelY = j;
				return TRUE;
			}
		}
	}
	m_MouseSelX = -1;
	m_MouseSelY = -1;
	// マウス入力キャンセル
	if (m_bInputParam) return TRUE;

	// 音色番号のヒットチェック180,368
	rc.left = 172;
	rc.right = rc.left + 78;
	rc.top = 368;
	rc.bottom = rc.top + 12;
	// 倍率設定
	rc.left *= m_iScale;
	rc.right *= m_iScale;
	rc.top *= m_iScale;
	rc.bottom *= m_iScale;
	// ヒット判定
	m_bIsToneNoSel = FALSE;
	if (m_MousePosX >= rc.left && m_MousePosX < rc.right &&
		m_MousePosY >= rc.top && m_MousePosY < rc.bottom) {
		m_bIsToneNoSel = TRUE;
		return TRUE;
	}

	// 音色名のヒットチェック
	rc.left = 172;
	rc.right = rc.left + 90;
	rc.top = 368 + 16;
	rc.bottom = rc.top + 12;
	// 倍率設定
	rc.left *= m_iScale;
	rc.right *= m_iScale;
	rc.top *= m_iScale;
	rc.bottom *= m_iScale;
	// ヒット判定
	m_bIsToneNameSel = FALSE;
	if (m_MousePosX >= rc.left && m_MousePosX < rc.right &&
		m_MousePosY >= rc.top && m_MousePosY < rc.bottom) {
		m_bIsToneNameSel = TRUE;
		return TRUE;
	}

	return bRet;
}

// マウスの当たり判定チェック処理
BOOL	FmToneEditor::MouseHitCheck() {
	RECT rc;
		// CH部分のヒットチェック
		for (int i = 0; i < 6; i++) {
			// 基本座標取得
			rc.left = 8;
			rc.right = rc.left + 114;
			rc.top = 401 + i * 12;
			rc.bottom = rc.top + 12;
			// 倍率設定
		rc.left *= m_iScale;
		rc.right *= m_iScale;
		rc.top *= m_iScale;
		rc.bottom *= m_iScale;
			// ヒット判定
			if (m_MousePosX >= rc.left && m_MousePosX < rc.right &&
				m_MousePosY >= rc.top && m_MousePosY < rc.bottom) {
				if (m_iChToneNum[i] >= 0) {
					m_iToneNo = m_iChToneNum[i];
					m_VoiceData = m_pVoiceData[m_iToneNo];
				}
				return TRUE;
			}
		}
		for (int i = 0; i < (int)m_vTones.size(); i++) {
			// 基本座標取得
			rc.left = 132 + (i / 6) * 80;
			rc.right = rc.left + 80;
			rc.top = 402 + (i % 6) * 12;
			rc.bottom = rc.top + 12;
			// 倍率設定
		rc.left *= m_iScale;
		rc.right *= m_iScale;
		rc.top *= m_iScale;
		rc.bottom *= m_iScale;
			if (m_MousePosX >= rc.left && m_MousePosX < rc.right &&
				m_MousePosY >= rc.top && m_MousePosY < rc.bottom) {
				m_iToneNo = m_vTones[i];
				m_VoiceData = m_pVoiceData[m_iToneNo];
				return TRUE;
			}
		}
	return FALSE;
}


// パラメータのクリッピング
int	FmToneEditor::ClipParam(int param,int min,int max,int add){
	param += add;
	if(param < min) param = min;
	if(param > max) param = max;
	return param;
}

// パラメーターの上下
BOOL FmToneEditor::ChangeParam(int param,int selX,int selY){
	// 直接入力状態の場合は即処理終了
//	if(m_bInputParam == TRUE) return TRUE;
	// 所定の場所処理
	if(selY == 0){
		// AR
		switch(selX){
			case 0: m_VoiceData.ar_op1 = ClipParam(m_VoiceData.ar_op1,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 1: m_VoiceData.ar_op2 = ClipParam(m_VoiceData.ar_op2,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.ar_op3 = ClipParam(m_VoiceData.ar_op3,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 3: m_VoiceData.ar_op4 = ClipParam(m_VoiceData.ar_op4,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}else if(selY == 1){
		// DR
		switch(selX){
			case 0: m_VoiceData.dr_op1 = ClipParam(m_VoiceData.dr_op1,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 1: m_VoiceData.dr_op2 = ClipParam(m_VoiceData.dr_op2,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.dr_op3 = ClipParam(m_VoiceData.dr_op3,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 3: m_VoiceData.dr_op4 = ClipParam(m_VoiceData.dr_op4,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}else if(selY == 2){
		// SR
		switch(selX){
			case 0: m_VoiceData.sr_op1 = ClipParam(m_VoiceData.sr_op1,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 1: m_VoiceData.sr_op2 = ClipParam(m_VoiceData.sr_op2,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.sr_op3 = ClipParam(m_VoiceData.sr_op3,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 3: m_VoiceData.sr_op4 = ClipParam(m_VoiceData.sr_op4,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}else if(selY == 3){
		// RR
		switch(selX){
			case 0: m_VoiceData.rr_op1 = ClipParam(m_VoiceData.rr_op1,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 1: m_VoiceData.rr_op2 = ClipParam(m_VoiceData.rr_op2,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.rr_op3 = ClipParam(m_VoiceData.rr_op3,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 3: m_VoiceData.rr_op4 = ClipParam(m_VoiceData.rr_op4,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}else if(selY == 4){
		// SL
		switch(selX){
			case 0: m_VoiceData.sl_op1 = ClipParam(m_VoiceData.sl_op1,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 1: m_VoiceData.sl_op2 = ClipParam(m_VoiceData.sl_op2,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.sl_op3 = ClipParam(m_VoiceData.sl_op3,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 3: m_VoiceData.sl_op4 = ClipParam(m_VoiceData.sl_op4,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}else if(selY == 5){
		// TL
		switch(selX){
			case 0: m_VoiceData.tl_op1 = ClipParam(m_VoiceData.tl_op1,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 1: m_VoiceData.tl_op2 = ClipParam(m_VoiceData.tl_op2,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.tl_op3 = ClipParam(m_VoiceData.tl_op3,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 3: m_VoiceData.tl_op4 = ClipParam(m_VoiceData.tl_op4,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}else if(selY == 6){
		// KS
		switch(selX){
			case 0: m_VoiceData.ks_op1 = ClipParam(m_VoiceData.ks_op1,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 1: m_VoiceData.ks_op2 = ClipParam(m_VoiceData.ks_op2,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.ks_op3 = ClipParam(m_VoiceData.ks_op3,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 3: m_VoiceData.ks_op4 = ClipParam(m_VoiceData.ks_op4,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}else if(selY == 7){
		// ML
		switch(selX){
			case 0: m_VoiceData.ml_op1 = ClipParam(m_VoiceData.ml_op1,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 1: m_VoiceData.ml_op2 = ClipParam(m_VoiceData.ml_op2,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.ml_op3 = ClipParam(m_VoiceData.ml_op3,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 3: m_VoiceData.ml_op4 = ClipParam(m_VoiceData.ml_op4,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}else if(selY == 8){
		// DT
		switch(selX){
			case 0: m_VoiceData.dt_op1 = ClipParam(m_VoiceData.dt_op1,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 1: m_VoiceData.dt_op2 = ClipParam(m_VoiceData.dt_op2,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.dt_op3 = ClipParam(m_VoiceData.dt_op3,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 3: m_VoiceData.dt_op4 = ClipParam(m_VoiceData.dt_op4,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}else if(selY == 9){
		// FB/AL
		switch(selX){
			case 0: m_VoiceData.fb = ClipParam(m_VoiceData.fb,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
			case 2: m_VoiceData.al = ClipParam(m_VoiceData.al,m_iRange[selY][selX][0],m_iRange[selY][selX][1],param); break;
		}
	}
	return TRUE;
}

// 入力開始
void FmToneEditor::InputStart(){
	// 選択したパラメータ初期化
	m_InputValue = -1;
	// 現在の項目をパラメータとして設定する
	if(m_SelY == 0){
		// AR
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.ar_op1; break;
			case 1: m_InputValue = m_VoiceData.ar_op2; break;
			case 2: m_InputValue = m_VoiceData.ar_op3; break;
			case 3: m_InputValue = m_VoiceData.ar_op4; break;
		 }
	}else if(m_SelY == 1){
		// DR
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.dr_op1; break;
			case 1: m_InputValue = m_VoiceData.dr_op2; break;
			case 2: m_InputValue = m_VoiceData.dr_op3; break;
			case 3: m_InputValue = m_VoiceData.dr_op4; break;
		}
	}else if(m_SelY == 2){
		// SR
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.sr_op1; break;
			case 1: m_InputValue = m_VoiceData.sr_op2; break;
			case 2: m_InputValue = m_VoiceData.sr_op3; break;
			case 3: m_InputValue = m_VoiceData.sr_op4; break;
		}
	}else if(m_SelY == 3){
		// RR
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.rr_op1; break;
			case 1: m_InputValue = m_VoiceData.rr_op2; break;
			case 2: m_InputValue = m_VoiceData.rr_op3; break;
			case 3: m_InputValue = m_VoiceData.rr_op4; break;
		}
	}else if(m_SelY == 4){
		// SL
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.sl_op1; break;
			case 1: m_InputValue = m_VoiceData.sl_op2; break;
			case 2: m_InputValue = m_VoiceData.sl_op3; break;
			case 3: m_InputValue = m_VoiceData.sl_op4; break;
		}
	}else if(m_SelY == 5){
		// TL
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.tl_op1; break;
			case 1: m_InputValue = m_VoiceData.tl_op2; break;
			case 2: m_InputValue = m_VoiceData.tl_op3; break;
			case 3: m_InputValue = m_VoiceData.tl_op4; break;
		}
	}else if(m_SelY == 6){
		// KS
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.ks_op1; break;
			case 1: m_InputValue = m_VoiceData.ks_op2; break;
			case 2: m_InputValue = m_VoiceData.ks_op3; break;
			case 3: m_InputValue = m_VoiceData.ks_op4; break;
		}
	}else if(m_SelY == 7){
		// ML
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.ml_op1; break;
			case 1: m_InputValue = m_VoiceData.ml_op2; break;
			case 2: m_InputValue = m_VoiceData.ml_op3; break;
			case 3: m_InputValue = m_VoiceData.ml_op4; break;
		}
	}else if(m_SelY == 8){
		// DT
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.dt_op1; break;
			case 1: m_InputValue = m_VoiceData.dt_op2; break;
			case 2: m_InputValue = m_VoiceData.dt_op3; break;
			case 3: m_InputValue = m_VoiceData.dt_op4; break;
		}
	}else if(m_SelY == 9){
		// FB/AL
		switch(m_SelX){
			case 0: m_InputValue = m_VoiceData.fb; break;
			case 2: m_InputValue = m_VoiceData.al; break;
		}
	}
	// パラメータが選択されない場合は処理を抜ける
	if(m_InputValue < 0) return;
	// 入力中にフラグを設定する
	m_bInputParam = TRUE;
	m_bInputFarst = TRUE;
	// 表示用文字列にパラメータを変換する
	ZeroMemory(m_cDispParam,sizeof(m_cDispParam));
	wsprintf(m_cDispParam,"%d",m_InputValue);
	// キャレット位置を最後にする
	m_iInputPos = lstrlen(m_cDispParam);

}

// 入力終了
void FmToneEditor::InputEnd(WPARAM wParam,LPARAM lParam){
	// ESCならパラメータの反映はしない
	if(wParam == VK_ESCAPE || strlen(m_cDispParam) == 0){
		// 入力中フラグを停止する
		m_bInputParam = FALSE;
		return;
	}
	// パラメータの範囲をチェックして問題が無ければ確定させる
	int iParam = atoi(m_cDispParam);
	BOOL bIsChanged = TRUE;
	if(iParam >= m_iRange[m_SelY][m_SelX][0] && iParam <= m_iRange[m_SelY][m_SelX][1]){
		// 確定させる
		// 現在の項目をパラメータとして設定する
		if(m_SelY == 0){
			// AR
			switch(m_SelX){
				case 0: m_VoiceData.ar_op1 = iParam; break;
				case 1: m_VoiceData.ar_op2 = iParam; break;
				case 2: m_VoiceData.ar_op3 = iParam; break;
				case 3: m_VoiceData.ar_op4 = iParam; break;
				default: bIsChanged = FALSE;
			 }
		}else if(m_SelY == 1){
			// DR
			switch(m_SelX){
				case 0: m_VoiceData.dr_op1 = iParam; break;
				case 1: m_VoiceData.dr_op2 = iParam; break;
				case 2: m_VoiceData.dr_op3 = iParam; break;
				case 3: m_VoiceData.dr_op4 = iParam; break;
				default: bIsChanged = FALSE;
			}
		}else if(m_SelY == 2){
			// SR
			switch(m_SelX){
				case 0: m_VoiceData.sr_op1 = iParam; break;
				case 1: m_VoiceData.sr_op2 = iParam; break;
				case 2: m_VoiceData.sr_op3 = iParam; break;
				case 3: m_VoiceData.sr_op4 = iParam; break;
				default: bIsChanged = FALSE;
			}
		}else if(m_SelY == 3){
			// RR
			switch(m_SelX){
				case 0: m_VoiceData.rr_op1 = iParam; break;
				case 1: m_VoiceData.rr_op2 = iParam; break;
				case 2: m_VoiceData.rr_op3 = iParam; break;
				case 3: m_VoiceData.rr_op4 = iParam; break;
				default: bIsChanged = FALSE;
			}
		}else if(m_SelY == 4){
			// SL
			switch(m_SelX){
				case 0: m_VoiceData.sl_op1 = iParam; break;
				case 1: m_VoiceData.sl_op2 = iParam; break;
				case 2: m_VoiceData.sl_op3 = iParam; break;
				case 3: m_VoiceData.sl_op4 = iParam; break;
				default: bIsChanged = FALSE;
			}
		}else if(m_SelY == 5){
			// TL
			switch(m_SelX){
				case 0: m_VoiceData.tl_op1 = iParam; break;
				case 1: m_VoiceData.tl_op2 = iParam; break;
				case 2: m_VoiceData.tl_op3 = iParam; break;
				case 3: m_VoiceData.tl_op4 = iParam; break;
				default: bIsChanged = FALSE;
			}
		}else if(m_SelY == 6){
			// KS
			switch(m_SelX){
				case 0: m_VoiceData.ks_op1 = iParam; break;
				case 1: m_VoiceData.ks_op2 = iParam; break;
				case 2: m_VoiceData.ks_op3 = iParam; break;
				case 3: m_VoiceData.ks_op4 = iParam; break;
				default: bIsChanged = FALSE;
			}
		}else if(m_SelY == 7){
			// ML
			switch(m_SelX){
				case 0: m_VoiceData.ml_op1 = iParam; break;
				case 1: m_VoiceData.ml_op2 = iParam; break;
				case 2: m_VoiceData.ml_op3 = iParam; break;
				case 3: m_VoiceData.ml_op4 = iParam; break;
				default: bIsChanged = FALSE;
			}
		}else if(m_SelY == 8){
			// DT
			switch(m_SelX){
				case 0: m_VoiceData.dt_op1 = iParam; break;
				case 1: m_VoiceData.dt_op2 = iParam; break;
				case 2: m_VoiceData.dt_op3 = iParam; break;
				case 3: m_VoiceData.dt_op4 = iParam; break;
				default: bIsChanged = FALSE;
			}
		}else if(m_SelY == 9){
			// FB/AL
			switch(m_SelX){
				case 0: m_VoiceData.fb = iParam; break;
				case 2: m_VoiceData.al = iParam; break;
				default: bIsChanged = FALSE;
			}
		}
		else {
			bIsChanged = FALSE;
		}
		m_bInputParam = FALSE;
		if (bIsChanged) {
			if (m_pVoiceData != NULL) {
				m_pVoiceData[m_iToneNo] = m_VoiceData;
			}
			SetToneParam(m_VoiceData);
		}
	}
	return;
}

// 編集中のキー操作
void FmToneEditor::InputKey(WPARAM wParam,LPARAM lParam){
	// 文字入力処理
	int iLen = lstrlen(m_cDispParam);
	// テンキーの入力だった場合
	if(wParam >= VK_NUMPAD0 && wParam <= VK_NUMPAD9){
		wParam -= 0x30;
	}
	// カーソルの移動
	if(wParam == VK_LEFT){
		// 左
		if(m_iInputPos > 0){
			m_iInputPos--;
		}
	}else if(wParam == VK_RIGHT){
		// 右
		if(m_iInputPos < lstrlen(m_cDispParam)){
			m_iInputPos++;
		}
	}else if(wParam == VK_BACK){
		// 削除処理
		if (m_bInputFarst == FALSE) {
		if(iLen > 0 && m_iInputPos > 0){
			int i;
			for(i = m_iInputPos - 1; i < 3; i++){
				m_cDispParam[i] = m_cDispParam[i + 1];
			}
			for(;i < 4; i++){
				m_cDispParam[i] = 0x00;
			}
			m_iInputPos--;
		}
		}
		else {
			for (int i = 0; i < 4; i++) {
				m_cDispParam[i] = 0x00;
			}
			m_iInputPos = 0;
		}
	}else if(wParam == VK_DELETE){
		// 削除処理
		if (m_bInputFarst == FALSE) {
		if(iLen > 0){
			int i;
			for(i = m_iInputPos; i < 4; i++){
				m_cDispParam[i] = m_cDispParam[i + 1];
			}
			for(;i < 4; i++){
				m_cDispParam[i] = 0x00;
			}
		}
		}
		else {
			for (int i = 0; i < 4; i++) {
				m_cDispParam[i] = 0x00;
			}
			m_iInputPos = 0;
		}
	}else if(wParam >= '0' && wParam <= '9'){
		if (m_bInputFarst == FALSE) {
		// 3文字未満なら入力可能
		if(iLen < 3){
			// 末尾か
			if(iLen == m_iInputPos){
				// 末尾の場合は文字列を追加する
				m_cDispParam[iLen] = (char)wParam;
				m_iInputPos++;
				}
				else {
				// 文字列を差し込む
				for(int i = 2; i > m_iInputPos; i--){
					m_cDispParam[i] = m_cDispParam[i - 1];
				}
				m_cDispParam[m_iInputPos] = (char)wParam;
			}
		}
	}
		else {
			// 初回入力で数値だった場合
			m_cDispParam[0] = (char)wParam;
			m_iInputPos = 1;
			for (int i = 1; i < 4; i++) {
				m_cDispParam[i] = 0x00;
			}
		}
	}
	// 初回入力フラグを落とす
	m_bInputFarst = FALSE;
}

// 音色設定
void	FmToneEditor::SetToneData(VOICEFORMAT param,int iToneNo){
	// パラメータを設定する
	m_VoiceData = param;
	if (iToneNo < 0) {
		// 現在の音色番号をしようするので何もしない
	}
	else {
		m_iToneNo = iToneNo;
	}
	SetToneParam(m_VoiceData);
}

// 音色取得
VOICEFORMAT FmToneEditor::GetToneData(){
	return m_VoiceData;
}

void	FmToneEditor::SetRegister(DWORD reg,DWORD data){
	m_pChip->SetRegister(reg,data);
}

void	FmToneEditor::SetToneParam(VOICEFORMAT param){
	// 音色設定
	unsigned char *pParam = reinterpret_cast<unsigned char*>(&param);

	// キー入力を初期化する
	keyInitialize();

	// 音色データを設定する
	for(int i = 0; i < 6; i++){
		SetToneDataChannel(pParam,i);
	}

	// 変更したパラメータを保存する
	if(m_pVoiceData) m_pVoiceData[m_iToneNo] = m_VoiceData;
	if(m_pVmCallback){
		m_pVmCallback(m_pVoiceData,m_iToneNo,m_pCallbackParam);
	}
}

void	FmToneEditor::SetToneDataChannel(unsigned char *pParam,int ch){
	DWORD	a2 = (ch >= 3) ? 0x100 : 0x000;
	DWORD	offset = a2 | (ch % 3);
	// TLOff
	SetRegister(0x40 + offset,127);
	SetRegister(0x44 + offset,127);
	SetRegister(0x48 + offset,127);
	SetRegister(0x4c + offset,127);
	SetRegister(0x80 + offset,0x0f);
	SetRegister(0x84 + offset,0x0f);
	SetRegister(0x88 + offset,0x0f);
	SetRegister(0x8c + offset,0x0f);
	// KeyOff
	SetRegister(0x28,(ch % 3) + ((ch / 3) * 4));
	pParam++;
	// ml/dt
	SetRegister(0x30 + offset,*pParam);
	pParam++;
	SetRegister(0x34 + offset,*pParam);
	pParam++;
	SetRegister(0x38 + offset,*pParam);
	pParam++;
	SetRegister(0x3c + offset,*pParam);
	pParam++;
	// tl
	SetRegister(0x40 + offset,*pParam);
	pParam++;
	SetRegister(0x44 + offset,*pParam);
	pParam++;
	SetRegister(0x48 + offset,*pParam);
	pParam++;
	SetRegister(0x4c + offset,*pParam);
	pParam++;
	// ar/ks
	SetRegister(0x50 + offset,*pParam);
	pParam++;
	SetRegister(0x54 + offset,*pParam);
	pParam++;
	SetRegister(0x58 + offset,*pParam);
	pParam++;
	SetRegister(0x5c + offset,*pParam);
	pParam++;
	// dr
	SetRegister(0x60 + offset,*pParam);
	pParam++;
	SetRegister(0x64 + offset,*pParam);
	pParam++;
	SetRegister(0x68 + offset,*pParam);
	pParam++;
	SetRegister(0x6c + offset,*pParam);
	pParam++;
	// sr
	SetRegister(0x70 + offset,*pParam);
	pParam++;
	SetRegister(0x74 + offset,*pParam);
	pParam++;
	SetRegister(0x78 + offset,*pParam);
	pParam++;
	SetRegister(0x7c + offset,*pParam);
	pParam++;
	// rr/sl
	SetRegister(0x80 + offset,*pParam);
	pParam++;
	SetRegister(0x84 + offset,*pParam);
	pParam++;
	SetRegister(0x88 + offset,*pParam);
	pParam++;
	SetRegister(0x8c + offset,*pParam);
	pParam++;
	// fb/al
	SetRegister(0xb0 + offset,*pParam);
}


int		FmToneEditor::keyToTone(WPARAM wParam){
	int iTone = 0;
	switch(wParam){
		case 'Z': iTone = -3; break;
		case 'S': iTone = -2; break;
		case 'X': iTone = -1; break;
		case 'C': iTone = 0; break;
		case 'F': iTone = 1; break;
		case 'V': iTone = 2; break;
		case 'G': iTone = 3; break;
		case 'B': iTone = 4; break;
		case 'N': iTone = 5; break;
		case 'J': iTone = 6; break;
		case 'M': iTone = 7; break;
		case 'K': iTone = 8; break;
		case VK_OEM_COMMA: iTone = 9; break;
		case 'L': iTone = 10; break;
		case VK_OEM_PERIOD: iTone = 11; break;
		case VK_OEM_2: iTone = 12; break;
		case VK_OEM_1: iTone = 13; break;
		case VK_OEM_102: iTone = 14; break;
		default: 
			return -1;
		break;
	}
	iTone = iTone + m_Octave * 12;
	if(iTone < 0) iTone = 0;
	return iTone;
}

// KeyOn
void	FmToneEditor::KeyOn(WPARAM wParam){
	KeyOnMidi(0x90 + (m_iTargetTrack & 0x0f),24 + keyToTone(wParam),127);
}

// KeyOff
void	FmToneEditor::KeyOff(WPARAM wParam){
	int iTone = keyToTone(wParam);
	if(iTone < 0) return;
	KeyOffMidi(0x80 + (m_iTargetTrack & 0x0f),24 + iTone,0);
}

// SetVolume
void	FmToneEditor::SetVolume(int iVolume){
	BYTE	bVolMask = 0;
	switch(m_VoiceData.al){
		case	0: bVolMask = 0x08 | 0x00 | 0x00 | 0x00;
			break;
		case	1: bVolMask = 0x08 | 0x00 | 0x00 | 0x00;
			break;
		case	2: bVolMask = 0x08 | 0x00 | 0x00 | 0x00;
			break;
		case	3: bVolMask = 0x08 | 0x00 | 0x00 | 0x00;
			break;
		case	4: bVolMask = 0x08 | 0x00 | 0x02 | 0x00;
			break;
		case	5: bVolMask = 0x08 | 0x04 | 0x02 | 0x00;
			break;
		case	6: bVolMask = 0x08 | 0x04 | 0x02 | 0x00;
			break;
		case	7: bVolMask = 0x08 | 0x04 | 0x02 | 0x01;
			break;
	}
	// ボリュームを設定する
	int iSetVolume;
	for(int i = 0; i < 6; i++){
		DWORD	a2 = (i >= 3) ? 0x100 : 0x000;
		DWORD	offset = a2 | (i % 3);
		if(bVolMask & 0x01){
			iSetVolume = (((127 - m_VoiceData.tl_op1) * iVolume) / 127);
			if(iSetVolume < 0) iSetVolume = 0;
			if(iSetVolume > 127) iSetVolume = 127;
			SetRegister(offset + 0x40,127 - iSetVolume);
		}
		if(bVolMask & 0x02){
			iSetVolume = (((127 - m_VoiceData.tl_op2) * iVolume) / 127);
			if(iSetVolume < 0) iSetVolume = 0;
			if(iSetVolume > 127) iSetVolume = 127;
			SetRegister(offset + 0x48,127 - iSetVolume);
		}
		if(bVolMask & 0x04){
			iSetVolume = (((127 - m_VoiceData.tl_op3) * iVolume) / 127);
			if(iSetVolume < 0) iSetVolume = 0;
			if(iSetVolume > 127) iSetVolume = 127;
			SetRegister(offset + 0x44,127 - iSetVolume);
		}
		if(bVolMask & 0x08){
			iSetVolume = (((127 - m_VoiceData.tl_op4) * iVolume) / 127);
			if(iSetVolume < 0) iSetVolume = 0;
			if(iSetVolume > 127) iSetVolume = 127;
			SetRegister(offset + 0x4c,127 - iSetVolume);
		}
	}
}

// カレントディレクトリ設定
void	FmToneEditor::SetCurrentDir(const char *pPath) {
	m_sCurrentPath = pPath;
}

// カレントディレクトリ取得
string	FmToneEditor::GetCurrentDir() {
	return m_sCurrentPath;
}

// Datファイル読込み
void	FmToneEditor::SetDatFile(string fileName,VOICEFORMAT *pData){
	BOOL IsIdentity = FALSE;
	// ファイル名の比較
	if(m_sDatFileName != fileName){
		IsIdentity = TRUE;
	}

	// ファイル名を保存する
	m_sDatFileName = fileName;
	// DATファイル領域があるか
	if(m_pVoiceData == NULL){
		m_pVoiceData = new VOICEFORMAT[256];
		memset(m_pVoiceData,0x00,sizeof(VOICEFORMAT) * 256);
	}
	if(m_pOrgVoiceData == NULL){
		m_pOrgVoiceData = new VOICEFORMAT[256];
		memset(m_pOrgVoiceData,0x00,sizeof(VOICEFORMAT) * 256);
	}
	// メモリをコピーする
	memcpy(m_pOrgVoiceData,pData,sizeof(VOICEFORMAT) * 256);
	// MML上で設定している音色リストが存在する場合
	for (size_t i = 0; i < m_vMmlToneList.size(); i++) {
		// 音色データを上書きする
		m_pOrgVoiceData[m_vMmlToneList[i].first] = m_vMmlToneList[i].second;
	}
	// 全音色ネームをチェックして不正文字をカットする
	for(int i = 0; i < 256; i++){
		for(int j = 0; j < 6; j++){
			// 不正文字は0x00に変換する
			if(m_pOrgVoiceData[i].name[j] < ' ' || m_pOrgVoiceData[i].name[j] > 0x7e) m_pOrgVoiceData[i].name[j] = 0x00;
		}
	}
	// 違うファイルなら
	if(IsIdentity){
		// オリジナルから編集中へコピーする
		memcpy(m_pVoiceData,m_pOrgVoiceData,sizeof(VOICEFORMAT) * 256);
		m_iToneNo = 0;
		m_VoiceData = m_pVoiceData[m_iToneNo];
		SetToneParam(m_VoiceData);
	}
	// MML上で設定している音色リストが存在する場合
	for (size_t i = 0; i < m_vMmlToneList.size(); i++) {
		// 音色データを上書きする
		m_pOrgVoiceData[m_vMmlToneList[i].first] = m_vMmlToneList[i].second;
	}
}

// mmlから音色データを設定
void	FmToneEditor::SetMmlTone(char *pMml) {
	// ベクターに保存している音色をクリアする
	m_vMmlToneList.clear();
	if (pMml) {
		ToneParam toneParam;
		toneParam.AnalizeToneParams(m_vMmlToneList, pMml);
	}
}

// 音色番号取得
int	FmToneEditor::GetToneNo(){
	return m_iToneNo;
}

// コールバック設定
void	FmToneEditor::SetToneChangeCallback(CHANGETONECALLBACK pFunc,LPVOID pParam){
	m_pVmCallback = pFunc;
	m_pCallbackParam = pParam;
}

// ファイル保存
void	FmToneEditor::SaveDatFiele(HWND hWnd) {
	char fileName[MAX_PATH];
	// メモリをクリアする
	ZeroMemory(fileName, MAX_PATH);

	// ファイル名が設定されているか
	if (m_sDatFileName.length() == 0) return;
	// フルパスのファイル名を生成する
	string fillPathName = m_sCurrentPath + "\\" + m_sDatFileName;
	// ファイル保存ダイアログを出す
	if (FileSaveDialog(hWnd, fileName, (LPSTR)fillPathName.c_str(), "音色データを保存する", "DATファイル(*.dat)\0 * .dat\0全てのファイル(*.*)\0 * .*","dat")) {
		if (m_pVoiceData) {
			MemSave(fileName, m_pVoiceData, sizeof(VOICEFORMAT) * 256);
		}
	}
}

// 中継
void	FmToneEditor::midiCallBack(UINT msg, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR drInstance) {
	FmToneEditor *pThis = (FmToneEditor*)drInstance;
	pThis->midiCallBackMain(msg, dwParam1, dwParam2);
}

void	FmToneEditor::midiCallBackMain(UINT msg, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
//	if (msg != MIM_DATA) {
//		return;
//	}
	switch(msg){
		case MIM_OPEN:
			return;
		case MIM_CLOSE:
			return;
		case MIM_LONGDATA:
			return;
		case MIM_ERROR:
			return;
		case MIM_LONGERROR:
			return;
		case MIM_DATA:
			break;
	}
	EnterCriticalSection(&cs);
	int cmd = (int)dwParam1 & 0xff;
	int param1 = (int)dwParam1 >> 8 & 0xff;
	int param2 = (int)dwParam1 >> 16 & 0xff;
	switch(cmd & 0xf0){
		case 0x90:	// KeyOn
			if(param2 != 0){
				KeyOnMidi(cmd,param1,param2);
			}else{
				KeyOffMidi(cmd,param1,param2);
			}
			break;
		case 0x80:	// KeyOff
			KeyOffMidi(cmd,param1,param2);
			break;
		case 0xb0:	// modulation
			if(param1 == 0x01){
				modulation(cmd,param1,param2);
			}
			break;
		case 0xe0:	// pitch
			pitch(cmd,param1,param2);
			break;
	}
	LeaveCriticalSection(&cs);
}

// キー情報を初期化する
void 	FmToneEditor::keyInitialize(){
	for(int i = 0; i < 6; i++){
		m_midiChannel[i].track = 0;
		m_midiChannel[i].bKeyOn = false;
		m_midiChannel[i].keyOnTime = 0x7fffffffffffffff;
		m_midiChannel[i].note = 0;
		DWORD	a2 = (i >= 3) ? 0x100 : 0x000;
		DWORD	offset = a2 | (i % 3);
		// TLOff
		SetRegister(0x40 + offset,127);
		SetRegister(0x44 + offset,127);
		SetRegister(0x48 + offset,127);
		SetRegister(0x4c + offset,127);
		SetRegister(0x80 + offset,0x0f);
		SetRegister(0x84 + offset,0x0f);
		SetRegister(0x88 + offset,0x0f);
		SetRegister(0x8c + offset,0x0f);
		// KeyOff
		SetRegister(0x28,(i % 3) + ((i / 3) * 4));
	}
	for(int i = 0; i < 16; i++){
		m_iPitch[i] = 0;
		m_iModulation[i]  = 0;
		m_midiModulation[i].iCount = 0;
		m_midiModulation[i].iPos = 0;
		m_midiModulation[i].iModulation = 0;
	}
}

// KeyOn
void  	FmToneEditor::KeyOnMidi(int cmd,int param1,int param2){
	// ボリューム用定義
	static const int iVolTbl[16] = {0,89,93,95,98,101,103,106,109,110,114,117,119,122,125,127};
	int track = cmd & 0x0f;
	if(track != m_iTargetTrack) return;
	if(track == 9) return;
	while(true){
		for(int i = 0; i < 6; i++){
			if(m_midiChannel[i].bKeyOn == false){
				DWORD	a2 = (i >= 3) ? 0x100 : 0x000;
				DWORD	offset = a2 | (i % 3);
				m_midiChannel[i].track = track;
				m_midiChannel[i].bKeyOn = true;
				m_midiChannel[i].note = param1;
				m_midiChannel[i].velocity = param2;
				SYSTEMTIME sysTime;
				GetSystemTime(&sysTime);
				SystemTimeToFileTime(&sysTime,(LPFILETIME)&m_midiChannel[i].keyOnTime);
				// 音量を設定する
				int velo = param2 > 80 ? 80 : param2;
				SetVolume(i,iVolTbl[15 * velo / 80]);
				// 音階を設定する
				setNote(i,param1,m_iPitch[track] + m_iModulation[track]);
				// KeyOnする
				SetRegister(0x28, 0xf0 + (i % 3) + ((i / 3) * 4));
				return;
			}
		}
		// 空きがないからループして止める
		LONGLONG time = 0x7fffffffffffffff;
		int iOldCh = -1;
		for(int i = 0; i < 6; i++){
			if(m_midiChannel[i].keyOnTime < time){
				time = m_midiChannel[i].keyOnTime;
				iOldCh = i;
			}
		}
		// 一番古いCHをKeyOffする
		KeyOffMidi(0x90 + m_midiChannel[iOldCh].track,m_midiChannel[iOldCh].note,m_midiChannel[iOldCh].velocity);
	}
}

// KeyOff
void  	FmToneEditor::KeyOffMidi(int cmd,int param1,int param2){
	int track = cmd & 0x0f;
	if(track != m_iTargetTrack) return;
	for(int i = 0; i < 6; i++){
		if(m_midiChannel[i].bKeyOn == TRUE && m_midiChannel[i].note == param1){
			// KeyOnする
			SetRegister(0x28, 0x00 + (i % 3) + ((i / 3) * 4));
			// クリアする
			m_midiChannel[i].bKeyOn = false;
			m_midiChannel[i].note = 0;
			m_midiChannel[i].velocity = 0;
			m_midiChannel[i].keyOnTime = 0;
			break;
		}
	}
}

// modulation
void FmToneEditor::modulation(int cmd,int param1,int param2){
	int track = cmd & 0x0f;
	if(track != m_iTargetTrack) return;
	if(track == 9) return;
	// モジュレーション記憶
	m_midiModulation[track].iModulation = param2;
	if(m_midiModulation[track].iModulation != 0){
		m_iModulation[track] = 0;
	} else {
		m_iModulation[track] = 0;
	}
}

// pitch
void FmToneEditor::pitch(int cmd,int param1,int param2){
	int track = cmd & 0x0f;
	if(track != m_iTargetTrack) return;
	if(track == 9) return;
	int pitch = (param1 | (param2 << 7)) - 8192;
	m_iPitch[track] = pitch / 6;
	for(int i = 0; i < 6; i++){
		if(m_midiChannel[i].bKeyOn == TRUE){
			setNote(i,m_midiChannel[i].note,m_iPitch[track] + m_iModulation[track]);
		}
	}
}

// ノート設定
void	FmToneEditor::setNote(int ch,int note,int pitch){
	// オクターブ単位の音階（１２度）
	int octNote = note % 12;
	int oct = (note / 12) - 2;
	// pitchの計算
	int octNoteAndPitch = ((octNote * 64) + pitch) % 768;
	oct += ((octNote * 64) + pitch) / 768;
	if(oct < 0) oct = 0;
	if(oct > 7) oct = 7;
	// １オクターブでの
	double freqRate = pow(2,(double)octNoteAndPitch / 768.0f) * 32.70319566257483;
	DWORD	a2 = (ch >= 3) ? 0x100 : 0x000;
	DWORD	offset = a2 | (ch % 3);
	// ＦＭ音源用音程計算
	double dFnum = (144 * freqRate * pow(2.0f, 20.0f) / (double)baseclock);
	DWORD fnum = (DWORD)dFnum;
	fnum |= (oct << 11);
	SetRegister(0xa4 + offset, (fnum >> 8) & 0xff);
	SetRegister(0xa0 + offset, fnum & 0xff);
}

// SetVolume
void	FmToneEditor::SetVolume(int ch,int iVolume){
	BYTE	bVolMask = 0;
	switch(m_VoiceData.al){
		case	0: bVolMask = 0x08 | 0x00 | 0x00 | 0x00;
			break;
		case	1: bVolMask = 0x08 | 0x00 | 0x00 | 0x00;
			break;
		case	2: bVolMask = 0x08 | 0x00 | 0x00 | 0x00;
			break;
		case	3: bVolMask = 0x08 | 0x00 | 0x00 | 0x00;
			break;
		case	4: bVolMask = 0x08 | 0x00 | 0x02 | 0x00;
			break;
		case	5: bVolMask = 0x08 | 0x04 | 0x02 | 0x00;
			break;
		case	6: bVolMask = 0x08 | 0x04 | 0x02 | 0x00;
			break;
		case	7: bVolMask = 0x08 | 0x04 | 0x02 | 0x01;
			break;
	}
	// ボリュームを設定する
	int iSetVolume;
	DWORD	a2 = (ch >= 3) ? 0x100 : 0x000;
	DWORD	offset = a2 | (ch % 3);
	if(bVolMask & 0x01){
		iSetVolume = (((127 - m_VoiceData.tl_op1) * iVolume) / 127);
		if(iSetVolume < 0) iSetVolume = 0;
		if(iSetVolume > 127) iSetVolume = 127;
		SetRegister(offset + 0x40,127 - iSetVolume);
	}
	if(bVolMask & 0x02){
		iSetVolume = (((127 - m_VoiceData.tl_op2) * iVolume) / 127);
		if(iSetVolume < 0) iSetVolume = 0;
		if(iSetVolume > 127) iSetVolume = 127;
		SetRegister(offset + 0x48,127 - iSetVolume);
	}
	if(bVolMask & 0x04){
		iSetVolume = (((127 - m_VoiceData.tl_op3) * iVolume) / 127);
		if(iSetVolume < 0) iSetVolume = 0;
		if(iSetVolume > 127) iSetVolume = 127;
		SetRegister(offset + 0x44,127 - iSetVolume);
	}
	if(bVolMask & 0x08){
		iSetVolume = (((127 - m_VoiceData.tl_op4) * iVolume) / 127);
		if(iSetVolume < 0) iSetVolume = 0;
		if(iSetVolume > 127) iSetVolume = 127;
		SetRegister(offset + 0x4c,127 - iSetVolume);
	}
}

// タイマー処理
void CALLBACK FmToneEditor::vTimerCallback(UINT uTimerID,UINT uMsg,LONG_PTR dwUser,DWORD dw1,DWORD dw2){
	FmToneEditor *pThis = (FmToneEditor*)dwUser;
	pThis->timerCallback(uTimerID,uMsg,dw1,dw2);
}

void FmToneEditor::timerCallback(UINT uTimerID,UINT uMsg,DWORD dw1,DWORD dw2){
	EnterCriticalSection(&cs);
	for(int i = 0; i < 16; i++){
		if (++m_midiModulation[i].iCount >= 10) {
			m_midiModulation[i].iCount = 0;
			if(m_midiModulation[i].iModulation != 0){
				m_iModulation[i] = (int)((double)m_midiModulation[i].iModulation * sin((3.141592535 * 2 / 360.f) * (double)m_midiModulation[i].iPos)) * 2;
				for(int  j = 0; j < 6; j++){
					if(m_midiChannel[j].bKeyOn == TRUE){
						setNote(j,m_midiChannel[j].note,m_iPitch[i] + m_iModulation[i]);
					}			
				}
				m_midiModulation[i].iPos += 18;
				m_midiModulation[i].iPos %= 360;
			}
		}
	}
	LeaveCriticalSection(&cs);
}

// MIDIデバイス設定
void	FmToneEditor::setMidiDevice(HWND hWnd){
	midiconfig mc(&m_midiIn,m_iMidiInDevNum);
	INT_PTR devNum = mc.Dialog(Application::app->GetInstance(),MAKEINTRESOURCE(IDD_DIALOG1),hWnd);
	// 何もしない
	if(devNum < 0) return;
	// デバイス番号を記憶する
	m_iMidiInDevNum = (int)devNum;
	// デバイスの再オープンを行う
	m_midiIn.close();
	// 再オープン
	m_midiIn.open(m_iMidiInDevNum, "", (DWORD_PTR)FmToneEditor::midiCallBack, (DWORD_PTR)this);	
	// プロファイルの記録
	Profile cProf;
	cProf.setModulePath(Application::app->GetInstance());	// モジュールのパスを設定する
	// 各種オプション項目取得
	cProf.setParam("MIDI","MIDI-IN",(DWORD)m_iMidiInDevNum);
}

