// マネージャー
#pragma once
#include	<vector>
#include	<queue>
#include "FmToneEditorManager.h"
#include "window\mainwindow.h"

#define	TONE_EDIT_MSG_SET_TONEDATA	(WM_APP + 100)
#define	TONE_EDIT_MSG_SET_MMLDATA	(WM_APP + 101)

// レジスタ用データ
struct FMREGDATA {
	int	reg;
	int	data;
};

using namespace std;

// マネージャークラス
class FmToneEditorManagerInner : public FmToneEditorManager {
	// 公開関数
public:
	// 初期化
	virtual BOOL __stdcall Initialize();
	// 開放
	virtual BOOL __stdcall UnInitialize();
	// 表示非表示
	virtual BOOL __stdcall Show(BOOL bShow);
	// カレントディレクトリの設定
	virtual void __stdcall SetCurrentDir(const char *pPath);
	// MMLデータ転送
	virtual void __stdcall SetMmlData(const char *pMml);
	// 音色データ転送
	virtual BOOL __stdcall SetToneData(const char *pFileName,const char *pMem);
	// 音色番号取得
	virtual int __stdcall GetTargetToneNum();
	// 音色情報マッチング
	virtual void __stdcall SetAnalyzeToneNum(const char *pMem);
	// レジスタ転送コマンド数取得
	virtual int __stdcall GetRegisterCommandCount();
	// レジスタへ送信する情報を取得する
	virtual FmRegisterCommand __stdcall GetRegisterCommand();
	// VMのレジスタ書き込みを取得する
	virtual void __stdcall SetRegister(DWORD reg,DWORD data);
	// トーン変更用のレジスタデータを受け取る
	virtual bool __stdcall GetRegisterDatas(int &reg,int &data);
	// 内部処理用変数
private:

	char		*m_pToneMem;			// 仮想マシン上のメモリアドレスを記憶しておく
	Application	*m_pApp;				// FmToneEditorアプリケーションクラス
	HANDLE		m_hThread;				// スレッド用ハンドル
	DWORD		m_dThreadId;			// スレッドID
	HINSTANCE	m_hInst;				// DLLのインスタンス
	vector<int>	m_vToneMap;				// 実音色とのMUBデータのマッチングを行う
	queue<FMREGDATA> m_qFmRegDatas;		// レジスタデータ
	BYTE		m_bRegs[2][0x100];		// レジスタ
	FmToneEditor *m_pEdit;				// エディタクラス
	BOOL		m_bStartMsgLoop;		// メッセージループ開始
	// VM上のメモリ
	const char *m_pVmToneBuff;			// VM上のバッファ
	int			m_iChToneNum[6];		// チャンネル毎の音色番号
	CRITICAL_SECTION	m_cs;			// クリティカルセッション

	// 内部処理用
public:
	// コンストラクタ
	FmToneEditorManagerInner(HINSTANCE hInst);
	// デストラクタ
	~FmToneEditorManagerInner();
private:
	// Window用スレッド処理
	BOOL startThread();									// スレッド開始
	BOOL stopThread();									// スレッド終了
	static DWORD WINAPI vThreadFunc(LPVOID pParam);		// スレッド関数
	void threadFunc();
	// レジスタの解析
	void	AnalyzeRegs();
	// レジスタから音色を取得する
	VOICEFORMAT GetPlayVoiceData(int iCh);
	// VM上の音色を取得する
	VOICEFORMAT GetVoiceData(int iNum);
	static 	void vVmCallback(VOICEFORMAT *pVoiceParam, int iNum, LPVOID pParam);
	void VmCallBack(VOICEFORMAT *pVoiceParam, int iNum);

};

