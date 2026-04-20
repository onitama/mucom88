#include "FmToneEditorManagerInner.h"

// 初期化
BOOL FmToneEditorManagerInner::Initialize(){
	InitializeCriticalSection(&m_cs);
	// FmToneEditroの起動を行う
	startThread();
	// メッセージループ開始麻痺
	while(1){
		if(m_bStartMsgLoop == TRUE) break;
		Sleep(0);
	}
	// ダミーメッセージをWindowへ送信して初期化まで確認する
	MainWindow* pWin = reinterpret_cast<MainWindow*>(m_pApp->GetWindowInstance());
	// ウィンドウ初期化終了待ち
	while(1){
		if(pWin->IsInitialized()) break;
		Sleep(0);
	}
	return TRUE;
}

// 開放
BOOL FmToneEditorManagerInner::UnInitialize(){
	// FmToneEditorの終了を行う
	stopThread();
	// クリティカルセッションの開放
	DeleteCriticalSection(&m_cs);
	return TRUE;
}

// 表示非表示
BOOL FmToneEditorManagerInner::Show(BOOL bShow){
	if(m_pApp){
		if(bShow){
			ShowWindow(m_pApp->getWindoHandle(),SW_SHOW);
		}else{
			ShowWindow(m_pApp->getWindoHandle(),SW_HIDE);
		}
	}
	return TRUE;
}

// カレントディレクトリの設定
void FmToneEditorManagerInner::SetCurrentDir(const char *pPath) {
	if (m_pEdit) m_pEdit->SetCurrentDir(pPath);
}

// MMLデータ転送
void FmToneEditorManagerInner::SetMmlData(const char *pMml) {
	// ウィンドウメッセージを送信する
	SendMessage(m_pApp->getWindoHandle(), TONE_EDIT_MSG_SET_MMLDATA, (WPARAM)NULL, (LPARAM)pMml);
}

// 音色データ転送
BOOL FmToneEditorManagerInner::SetToneData(const char *pFileName,const char *pMem){
	// ウィンドウメッセージを送信する
	SendMessage(m_pApp->getWindoHandle(),TONE_EDIT_MSG_SET_TONEDATA,(WPARAM)pFileName,(LPARAM)pMem);
	return TRUE;
}

// 音色番号取得
int FmToneEditorManagerInner::GetTargetToneNum(){
	return 0;
}

// チェック用のダンプルーチン
void TestDump(char *pText,char *pData,size_t size){
	char strBuf[1024];
	char str[1024];
	wsprintf(strBuf,pText);
	for(size_t sz = 0; sz < size; sz++){
		wsprintf(str,"%02x ",(DWORD)(pData[sz]) & 0x000000ff);
		lstrcat(strBuf,str);
	}
	lstrcat(strBuf,"\r\n");
	OutputDebugString(strBuf);
}

// 音色番号をメモリから解析する
void FmToneEditorManagerInner::SetAnalyzeToneNum(const char *pMem){
	// ベースになる場所
	unsigned short *pOffset = (unsigned short*)(pMem + 1);
	DWORD offset = *pOffset;
	unsigned char *pTop = (unsigned char*)((DWORD_PTR)pMem + offset);

	// VMのトップアドレスを指定しておく
	m_pVmToneBuff = pMem;

	// テーブルへ移動する
	int iNum = *pTop++;
	m_pVmToneBuff = (char*)pTop;		// VMのアドレスを記憶しておく
	// ベクターをクリアする
	m_vToneMap.clear();
	// エディタークラスを記憶しておく
	m_pEdit = reinterpret_cast<FmToneEditor*>(m_pApp->GetWindowInstance()->GetEditorInstance());
	// 音色分ループして音色テーブルを生成する
	VOICEFORMAT *pEditorVoice = m_pEdit->GetLoadVoiceParams();
//	VOICEFORMAT *pEditorVoice = m_pEdit->GetEditVoiceParams();
	for(int i = 0; i < iNum; i++){
		// 音色データを総当たり戦で取得する
		for(int j = 0; j < 256; j++){
			VOICEFORMAT tone = pEditorVoice[j];
			tone.am_op1 = 1;
			tone.am_op2 = 1;
			tone.am_op3 = 1;
			tone.am_op4 = 1;
			char *pEditorTone = (char*)(((DWORD_PTR)&tone) + 1);
			if(memcmp(pTop + (i * 25),pEditorTone,25) == 0){
				// ベクターに追加する
				m_vToneMap.push_back(j);
				break;
			}
		}
	}
	// エディターにトーン番号を設定する
	m_pEdit->SetTones(m_vToneMap);
	// 編集中の音色をVMに書き込む
	VOICEFORMAT *pEditingVoice = m_pEdit->GetEditVoiceParams();
	for(size_t i = 0; i < m_vToneMap.size(); i++){
		VOICEFORMAT tone = pEditingVoice[m_vToneMap[i]];
		tone.am_op1 = 1;
		tone.am_op2 = 1;
		tone.am_op3 = 1;
		tone.am_op4 = 1;
		char *pTone = (char*)(((DWORD_PTR)&tone + 1));
		// VMのメモリへコピーする
		memcpy(pTop + (i * 25),pTone ,25);
	}
	// コールバックを設定する
	m_pEdit->SetToneChangeCallback(	vVmCallback, this);
}

// レジスタ転送コマンド数取得
int FmToneEditorManagerInner::GetRegisterCommandCount(){
	return 0;
}

// レジスタへ送信する情報を取得する
FmRegisterCommand FmToneEditorManagerInner::GetRegisterCommand(){
	FmRegisterCommand cmd = {0,0};
	return cmd;
}

// レジスタマップにレジスタ情報を書き込む
void	FmToneEditorManagerInner::SetRegister(DWORD reg,DWORD data){
	// レジスタに書き込みする
	m_bRegs[(reg >> 8) & 1][reg & 0xff] = (BYTE)data;
	// レジスタ解析へ処理をまわす
	if((reg & 0xff) >= 0xb0 && (reg & 0xff) <= 0xb2){
		AnalyzeRegs();
	}
}

// コンストラクタ
FmToneEditorManagerInner::FmToneEditorManagerInner(HINSTANCE hInst){
	// 変数の初期化
	m_pToneMem = NULL;
	m_pApp = NULL;
	m_hThread = NULL;
	m_dThreadId = NULL;
	m_hInst = hInst;
	memset(&m_bRegs,0x00,sizeof(m_bRegs));		// レジスタマップをクリアする
	m_pEdit = NULL;
	m_bStartMsgLoop = FALSE;
	m_pVmToneBuff = NULL;
	for(int i = 0; i < 6; i++){
		m_iChToneNum[i] = -1;
	}
}

// デストラクタ
FmToneEditorManagerInner::~FmToneEditorManagerInner(){
}

// スレッド起動
BOOL FmToneEditorManagerInner::startThread(){
	// スレッドを生成する
	m_hThread = CreateThread(NULL,1024 * 1024 * 4,(LPTHREAD_START_ROUTINE)FmToneEditorManagerInner::vThreadFunc,(LPVOID)this,NULL,&m_dThreadId);
	if(m_hThread == NULL){
		return FALSE;
	}
	// スレッドの優先順位を変更する
	SetThreadPriority(m_hThread,THREAD_PRIORITY_TIME_CRITICAL);
	// タイマー精度を1msecに設定
	timeBeginPeriod(1);
	return	TRUE;
}

// スレッド停止
BOOL FmToneEditorManagerInner::stopThread(){
	DWORD	dActive = 0;
	// アプリケーションクラスが起動中の場合
	if(m_pApp != NULL){
		// Winndouを消しに行く
		DestroyWindow(m_pApp->getWindoHandle());
	}
	// スレッド停止を待つ
	GetExitCodeThread(m_hThread,&dActive);
	if(dActive == STILL_ACTIVE){
		::WaitForSingleObject(m_hThread,500);
	}
	// スレッド破棄
	CloseHandle(m_hThread);
	m_hThread = NULL;
	m_dThreadId = 0;
	return	TRUE;
}

// スレッド関数
DWORD WINAPI FmToneEditorManagerInner::vThreadFunc(LPVOID pParam){
	FmToneEditorManagerInner *pThis = reinterpret_cast<FmToneEditorManagerInner*>(pParam);
	pThis->threadFunc();
	return 0;
}

// クラス内部スレッド関数
void FmToneEditorManagerInner::threadFunc(){
	// アプリケーションクラスを起動する
	LONG_PTR ret;
	//	----------Appクラス発生----------
	m_pApp = new Application(m_hInst,NULL,NULL,FALSE);
	//	----------ウィンドウ作成----------
	m_pApp->Init();
	//	----------同期モードで設定する----------
	m_pApp->SetSyncMode(TRUE);
	//	----------メッセージループ開始----------
	m_bStartMsgLoop = TRUE;
	//	----------メッセージループ----------
	ret = m_pApp->Msg();	//	メッセージループ
	//	----------開放処理----------
	delete	m_pApp;
	m_pApp = NULL;
}

// レジスタの解析
void	FmToneEditorManagerInner::AnalyzeRegs(){
	if(m_pEdit == NULL) return;
	// 各CHをチェック
	for(int i = 0; i < 6; i++){
		m_iChToneNum[i] = -1;
		m_pEdit->SetPlayToneNum(i,-1);
		VOICEFORMAT ch = GetPlayVoiceData(i);			// 各CHのトーンデータを取得するS
		for(size_t j = 0; j < m_vToneMap.size(); j++){
			VOICEFORMAT voice = GetVoiceData((int)j);
			// メモリ上を比較する
			if(memcmp(&ch,&voice,26) == 0){
				m_pEdit->SetPlayToneNum(i,m_vToneMap[j]);
				m_iChToneNum[i] = m_vToneMap[j];
				break;
			}
		}
	}
}

// レジスタから音色を取得する
VOICEFORMAT FmToneEditorManagerInner::GetPlayVoiceData(int iCh){
	VOICEFORMAT voice;
	memset(&voice,0x00,sizeof(VOICEFORMAT));
	// 指定されたレジスタからデータを取得する
	char  *pData = reinterpret_cast<char*>(&voice);
	pData++;
	// バンクとチャンネルを取得
	DWORD dBank = iCh / 3;
	DWORD dCh = iCh % 3;
	// レジスタ情報を取得書き込み
	// Dt/Ml
	*pData++ = m_bRegs[dBank][0x30 + dCh];
	*pData++ = m_bRegs[dBank][0x34 + dCh];
	*pData++ = m_bRegs[dBank][0x38 + dCh];
	*pData++ = m_bRegs[dBank][0x3c + dCh];
	// Tl
	*pData++ = m_bRegs[dBank][0x40 + dCh];
	*pData++ = m_bRegs[dBank][0x44 + dCh];
	*pData++ = m_bRegs[dBank][0x48 + dCh];
	*pData++ = m_bRegs[dBank][0x4c + dCh];
	// Ks/Ar
	*pData++ = m_bRegs[dBank][0x50 + dCh];
	*pData++ = m_bRegs[dBank][0x54 + dCh];
	*pData++ = m_bRegs[dBank][0x58 + dCh];
	*pData++ = m_bRegs[dBank][0x5c + dCh];
	// Dr
	*pData++ = m_bRegs[dBank][0x60 + dCh];
	*pData++ = m_bRegs[dBank][0x64 + dCh];
	*pData++ = m_bRegs[dBank][0x68 + dCh];
	*pData++ = m_bRegs[dBank][0x6c + dCh];
	// Sr
	*pData++ = m_bRegs[dBank][0x70 + dCh];
	*pData++ = m_bRegs[dBank][0x74 + dCh];
	*pData++ = m_bRegs[dBank][0x78 + dCh];
	*pData++ = m_bRegs[dBank][0x7c + dCh];
	// Sl/Rr
	*pData++ = m_bRegs[dBank][0x80 + dCh];
	*pData++ = m_bRegs[dBank][0x84 + dCh];
	*pData++ = m_bRegs[dBank][0x88 + dCh];
	*pData++ = m_bRegs[dBank][0x8c + dCh];
	// FB/AL
	*pData++ = m_bRegs[dBank][0xb0 + dCh];

	// ALでTLをマスク（比較用）
	switch(voice.al){
		case	0:
		case	1:
		case	2:
		case	3:
			voice.tl_op4 = 0;
			break;
		case	4:
			voice.tl_op2 = 0;
			voice.tl_op4 = 0;
			break;
		case	5:
		case	6:
			voice.tl_op2 = 0;
			voice.tl_op3 = 0;
			voice.tl_op4 = 0;
			break;
		case	7:
			voice.tl_op1 = 0;
			voice.tl_op2 = 0;
			voice.tl_op3 = 0;
			voice.tl_op4 = 0;
			break;
	}
	return voice;
}

// 音色データを取得する
VOICEFORMAT FmToneEditorManagerInner::GetVoiceData(int iNum){
	VOICEFORMAT voice;
	// 音色分ループして音色テーブルを生成する
	VOICEFORMAT *pEditorVoice = m_pEdit->GetEditVoiceParams();
	// ボイスデータを取得する
	voice = pEditorVoice[m_vToneMap[iNum]];
	voice.hed = 0;
	// AMをONに設定する
	voice.am_op1 = 1;
	voice.am_op2 = 1;
	voice.am_op3 = 1;
	voice.am_op4 = 1;
	// ALでTLをマスク（比較用）
	switch(voice.al){
		case	0:
		case	1:
		case	2:
		case	3:
			voice.tl_op4 = 0;
			break;
		case	4:
			voice.tl_op2 = 0;
			voice.tl_op4 = 0;
			break;
		case	5:
		case	6:
			voice.tl_op2 = 0;
			voice.tl_op3 = 0;
			voice.tl_op4 = 0;
			break;
		case	7:
			voice.tl_op1 = 0;
			voice.tl_op2 = 0;
			voice.tl_op3 = 0;
			voice.tl_op4 = 0;
			break;
	}
	return voice;
}

// コールバック関数
void FmToneEditorManagerInner::vVmCallback(VOICEFORMAT *pVoiceParam,int iNum,LPVOID pParam){
	FmToneEditorManagerInner *pThis = reinterpret_cast<FmToneEditorManagerInner*>(pParam);
	pThis->VmCallBack(pVoiceParam,iNum);
}

// コールバック関数本体
void FmToneEditorManagerInner::VmCallBack(VOICEFORMAT *pVoiceParam,int iNum){
	// 編集しているトーンをチェックしてVMに書き込みしてあげる
	for(size_t i = 0; i < m_vToneMap.size(); i++){
		if(m_vToneMap[i] == iNum){
			VOICEFORMAT tone = pVoiceParam[iNum];
			tone.am_op1 = 1;
			tone.am_op2 = 1;
			tone.am_op3 = 1;
			tone.am_op4 = 1;
			char *pTone = (char*)(((DWORD_PTR)&tone + 1));
			// VMのメモリへコピーする
			memcpy((void*)((DWORD_PTR)m_pVmToneBuff + (i * 25)),pTone ,25);
			break;
		}
	}
	// チャンネルで対象の音色が使われているチャンネルがある場合
	VOICEFORMAT tone = pVoiceParam[iNum];
	tone.am_op1 = 1;
	tone.am_op2 = 1;
	tone.am_op3 = 1;
	tone.am_op4 = 1;
	char *pTone = reinterpret_cast<char*>((DWORD_PTR)&tone + 1);
	int iOffset = 0;
	int iBank = 0;
	int iAl = pVoiceParam[iNum].al;
	FMREGDATA data;
	EnterCriticalSection(&m_cs);
	for(int i = 0; i < 6; i++){
		iBank = (i / 3) * 0x100;
		iOffset = i % 3;
		char *pSrc = pTone;
		if(m_iChToneNum[i] == iNum){
			for(int j = 0; j < 24; j++){
				data.reg = 0x30 + (j << 2) + iOffset + iBank;
				data.data = pTone[j];
				// TLはALに従い設定する
				if(j >= 4 && j < 8){
					if(j == 4){
						// OP1
						m_qFmRegDatas.push(data);
					} else if(j == 5){
						// OP3
						switch (iAl) {
							case 0:
							case 1:
							case 2:
							case 3:
							case 4:
							case 7:
								m_qFmRegDatas.push(data);
								break;
						}
					} else if(j == 6){
						// OP2
						switch(iAl){
							case 0:
							case 1:
							case 2:
							case 3:
							case 7:
								m_qFmRegDatas.push(data);
								break;
						}
					} else if(j == 7 && iAl == 7){
						// OP4
						m_qFmRegDatas.push(data);
					}
				}else{
					m_qFmRegDatas.push(data);
				}
			}
			// AL/FB書き込み
			data.reg = 0xb0 + iOffset + iBank;
			data.data = pTone[24];
			m_qFmRegDatas.push(data);
		}
	}
	LeaveCriticalSection(&m_cs);
}

// VMで転送してもらうレジスタデータを返却する
bool FmToneEditorManagerInner::GetRegisterDatas(int &reg,int &data){
	bool bRet = false;
	EnterCriticalSection(&m_cs);
	if(m_qFmRegDatas.size() > 0){
		// レジスタデータを取得する
		FMREGDATA fmdata = m_qFmRegDatas.front();
		// 削除する
		m_qFmRegDatas.pop();
		reg = fmdata.reg;
		data = fmdata.data;
		bRet = true;
		//// データをダンプする
		//char str[256];
		//wsprintf(str, "%02x:%02x\r\n", (BYTE)reg, (BYTE)data);
		//OutputDebugString(str);
	}
	LeaveCriticalSection(&m_cs);
	return bRet;
}
