#include <windows.h>
#include "FmToneEditorManagerInner.h"
#pragma comment(lib,"winmm.lib")

// 初期化
BOOL FmToneEditorManagerInner::Initialize(){
	InitializeCriticalSection(&m_cs);

	allowChangeTone = false;

	// FmToneEditroの起動を行う
	startThread();
	// メッセージループ開始待ち
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

//	プラグイン情報を設定
void FmToneEditorManagerInner::SetPlugin(Mucom88Plugin *mucom88plg)
{
	m_mucom88plg = mucom88plg;
}

// 演奏後の割り込み通知
void FmToneEditorManagerInner::StartAnalyze(void)
{
	// 音色情報を解析する
	AnalyzeUseToneNo();
	// FM音源のレジスタへ送信する情報がある場合はここで送る

}

// 演奏前の通知
void FmToneEditorManagerInner::StartPlayback(void)
{
	//		MUCOM88演奏前に呼ばれます
	//		この中で各種タグ情報やメモリの状態などを参照することができます
	//

	//// おにたま追加: (プラグインからVM情報を取得する)
	const char *voicename;
	MUCOM88_VOICEFORMAT *pFmVoice;
	MUCOM88_VOICEFORMAT *pFmVoiceOriginal;

	// おにたま追加:初期化しておきます
	for (int i = 0; i < 6; i++) {
		m_iChToneNum[i] = -1;
		m_iChVolume[i] = 0;
	}

	m_mucom88plg->if_mucomvm(m_mucom88plg, MUCOM88IF_MUCOMVM_CMD_GETVOICENAME, 0, 0, &voicename, NULL);				// 音色ファイル名を取得
	m_mucom88plg->if_mucomvm(m_mucom88plg, MUCOM88IF_MUCOMVM_CMD_GETVOICEDATA, 0, 0, &pFmVoiceOriginal, &pFmVoice);	// 音色データを取得

	// 音色変更を許可しない(最初の設定時に音色変更してしまうため)
	allowChangeTone = false;

	// 音色情報を音色エディタに設定する
	SetToneData(voicename, (char *)pFmVoice);
	SetOrgToneData((char*)pFmVoiceOriginal);

	// 使用中運音色を設定
	SetUseToneNo();

	// 音色変更を許可する
	allowChangeTone = true;

	// 音色変更コールバック設定
	m_pEdit->SetToneChangeCallback(vVmCallback, this);

	// 音色保存用コールバック設定
	m_pEdit->SetSaveToneCallback(vSaveToneCallback, this);

}

// コンストラクタ
FmToneEditorManagerInner::FmToneEditorManagerInner(HINSTANCE hInst){
	// 変数の初期化
	m_pToneMem = NULL;
	m_pApp = NULL;
	m_hThread = NULL;
	m_dThreadId = NULL;
	m_hInst = hInst;
	m_pEdit = NULL;
	m_bStartMsgLoop = FALSE;
	m_pVmToneBuff = NULL;
	for(int i = 0; i < 6; i++){
		m_iChToneNum[i] = -1;
		m_iChVolume[i] = 0;
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
	int				ret;
	//	----------Appクラス発生----------
	m_pApp = new Application(m_hInst,NULL,NULL,FALSE);
	//	----------ウィンドウ作成----------
	m_pApp->Init();
	//	----------メッセージループ開始----------
	m_bStartMsgLoop = TRUE;
	//	----------メッセージループ----------
	ret = m_pApp->Msg();	//	メッセージループ
	//	----------開放処理----------
	delete	m_pApp;
	m_pApp = NULL;
}

// レジスタの解析
void	FmToneEditorManagerInner::AnalyzeUseToneNo(){
	if(m_pEdit == NULL) return;		// エディタが定義されていない場合スルー
	PCHDATA pchdata;				// チャンネル情報
	int iChTbl[] = { 0,1,2,7,8,9 };	// チャンネル変換テーブル
	// VMからの音色番号を反映する
	for (int i = 0; i < 6; i++) {
		int iCh = iChTbl[i];
		m_mucom88plg->if_mucomvm(m_mucom88plg, MUCOM88IF_MUCOMVM_CMD_GETCHDATA, iCh, 0, &pchdata, NULL);	// 演奏情報を取得
		if (m_iChToneNum[i] != pchdata.vnum_org) {
			m_iChToneNum[i] = pchdata.vnum_org;
			m_pEdit->SetPlayToneNum(i, m_iChToneNum[i]);
		} else if (m_iChToneNum[i] < 0) {
			// 定義されていない音色の場合
			m_pEdit->SetPlayToneNum(i,-1);
		}
		// ボリュームを設定する
		m_iChVolume[i] = pchdata.vol_org;
	}
	EnterCriticalSection(&m_cs);
	while(m_qFmRegDatas.size() > 0){
		// レジスタデータを取得する
		FMREGDATA fmdata = m_qFmRegDatas.front();
		// 削除する
		m_qFmRegDatas.pop();
		int reg = fmdata.reg;
		int data = fmdata.data;
		// VM側にデータを送信する
		m_mucom88plg->if_mucomvm(m_mucom88plg, MUCOM88IF_MUCOMVM_CMD_FMWRITE, reg, data, NULL, NULL);
	}
	LeaveCriticalSection(&m_cs);

}

// コールバック関数
void FmToneEditorManagerInner::vVmCallback(VOICEFORMAT *pVoiceParam,int iNum,LPVOID pParam){
	FmToneEditorManagerInner *pThis = reinterpret_cast<FmToneEditorManagerInner*>(pParam);
	pThis->VmCallBack(pVoiceParam,iNum);
}

// コールバック関数本体
void FmToneEditorManagerInner::VmCallBack(VOICEFORMAT *pVoiceParam,int iNum){
	static const int iVolTbl[] = {
		0x2a,0x28,0x25,0x22,
		0x20,0x1d,0x1a,0x18,
		0x15,0x12,0x10,0x0d,
		0x0a,0x08,0x05,0x02
	};

	// 編集しているトーンをチェックしてVMに書き込みしてあげる
	for(size_t i = 0; i < m_vToneMap.size(); i++){
		if(m_vToneMap[i] == iNum){
			VOICEFORMAT tone = pVoiceParam[iNum];
			tone.am_op1 = 1;
			tone.am_op2 = 1;
			tone.am_op3 = 1;
			tone.am_op4 = 1;
			char *pTone = (char*)(&tone);
			// VMのメモリへコピーする
			memcpy((void*)(m_pVmToneBuff + (i * 25)),pTone+1 ,25);
			break;
		}
	}

	// MUCOMへデータ音色データを設定する
	if (allowChangeTone) {
		m_mucom88plg->if_mucomvm(m_mucom88plg, MUCOM88IF_MUCOMVM_CMD_VOICEUPDATE, iNum, 0, &pVoiceParam[iNum], NULL);

		// チャンネルで対象の音色が使われているチャンネルがある場合
		VOICEFORMAT tone = pVoiceParam[iNum];
		tone.am_op1 = 1;
		tone.am_op2 = 1;
		tone.am_op3 = 1;
		tone.am_op4 = 1;
		char *pTone = reinterpret_cast<char*>((DWORD)&tone + 1);
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
				int iVolume = iVolTbl[m_iChVolume[i]];
				for(int j = 0; j < 24; j++){
					data.reg = 0x30 + (j << 2) + iOffset + iBank;
					data.data = pTone[j];
					// TLはALに従い設定する
					if(j >= 4 && j < 8){
						if(j == 4){
							// OP1
							if (iAl == 7) {
								data.data = iVolume;			// ボリュームを設定する
							}
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
									break;
								default:
									data.data = iVolume;			// ボリュームを設定する
							}
							m_qFmRegDatas.push(data);
						} else if(j == 6){
							// OP2
							switch(iAl){
								case 0:
								case 1:
								case 2:
								case 3:
								case 7:
									break;
								default:
									data.data = iVolume;			// ボリュームを設定する
							}
							m_qFmRegDatas.push(data);
						} else if(j == 7 && iAl == 7){
							// OP4
							data.data = iVolume;			// ボリュームを設定する
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
}

// 音色データ保存コールバック呼び出し
void FmToneEditorManagerInner::vSaveToneCallback(LPVOID pParam) {
	FmToneEditorManagerInner *pThis = reinterpret_cast<FmToneEditorManagerInner*>(pParam);
	pThis->SaveToneCallback();
}

// 音色データ保存コールバック
void FmToneEditorManagerInner::SaveToneCallback() {
	// MUCOMへデータ更新を指示する
	m_mucom88plg->if_mucomvm(m_mucom88plg, MUCOM88IF_MUCOMVM_CMD_VOICESAVE, 0, 0, NULL, NULL);
}

// 音色データ転送
BOOL FmToneEditorManagerInner::SetToneData(const char *pFileName, const char *pMem) {
	// ウィンドウメッセージを送信する
	SendMessage(m_pApp->getWindoHandle(), TONE_EDIT_MSG_SET_TONEDATA, (WPARAM)pFileName, (LPARAM)pMem);
	return TRUE;
}

// オリジナル音色データ転送
BOOL FmToneEditorManagerInner::SetOrgToneData(const char *pMem) {
	SendMessage(m_pApp->getWindoHandle(), TONE_EDIT_MSG_SET_ORGTONE, NULL, (LPARAM)pMem);
	return TRUE;
}
// チェック用のダンプルーチン
void TestDump(char *pText, char *pData, size_t size) {
	char strBuf[1024];
	char str[1024];
	wsprintf(strBuf, pText);
	for (size_t sz = 0; sz < size; sz++) {
		wsprintf(str, "%02x ", (DWORD)(pData[sz]) & 0x000000ff);
		lstrcat(strBuf, str);
	}
	lstrcat(strBuf, "\r\n");
	OutputDebugString(strBuf);
}

// 音色エディタ側へ使用中の音色を設定する
void FmToneEditorManagerInner::SetUseToneNo() {

	char *pMem;
	m_mucom88plg->if_mucomvm(m_mucom88plg, MUCOM88IF_MUCOMVM_CMD_GETVMMEMMAP, 0, 0, &pMem, NULL);	// vmからメモリを取得する

	// ベースになる場所
	unsigned short *pOffset = (unsigned short*)(pMem + 0xc201);
	DWORD offset = *pOffset;
	unsigned char *pTop = (unsigned char*)((DWORD)pMem + 0xc200 + offset);

	// テーブルへ移動する
	int iNum = *pTop++;
	m_pVmToneBuff = (char*)pTop;		// VMのアドレスを記憶しておく
	// ベクターをクリアする
	m_vToneMap.clear();
	// エディタークラスを記憶しておく
	m_pEdit = reinterpret_cast<FmToneEditor*>(m_pApp->GetWindowInstance()->GetEditorInstance());


	int voicenum[MUCOM_FMVOICE_MAX];
	iNum = m_mucom88plg->if_mucomvm(m_mucom88plg, MUCOM88IF_MUCOMVM_CMD_GETVOICENUM, 0, 0, &voicenum, NULL);	// 使用する音色番号テーブルを取得
	for (int i = 0; i < iNum; i++) {
		// ベクターに追加する
		m_vToneMap.push_back(voicenum[i]);
	}

	// エディターにトーン番号を設定する
	m_pEdit->SetTones(m_vToneMap);
}



