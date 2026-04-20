// マネージャー
#pragma once
#include	<vector>
#include	<queue>
#include "FmToneEditorManager.h"
#include "window\mainwindow.h"

#define	TONE_EDIT_MSG_SET_TONEDATA	(WM_APP + 100)
#define	TONE_EDIT_MSG_SET_ORGTONE	(WM_APP + 101)

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
	// プラグイン情報の設定
	virtual void __stdcall SetPlugin(Mucom88Plugin *mucom88plg);
	// 演奏後の割り込み通知
	virtual void __stdcall StartAnalyze(void);
	// 演奏前の通知
	virtual void __stdcall StartPlayback(void);

	// 内部処理用変数
private:

	char		*m_pToneMem;			// 仮想マシン上のメモリアドレスを記憶しておく
	Application	*m_pApp;				// FmToneEditorアプリケーションクラス
	HANDLE		m_hThread;				// スレッド用ハンドル
	DWORD		m_dThreadId;			// スレッドID
	HINSTANCE	m_hInst;				// DLLのインスタンス
	vector<int>	m_vToneMap;				// 実音色とのMUBデータのマッチングを行う
	queue<FMREGDATA> m_qFmRegDatas;		// レジスタデータ
	FmToneEditor *m_pEdit;				// エディタクラス
	BOOL		m_bStartMsgLoop;		// メッセージループ開始
	// VM上のメモリ
	const char *m_pVmToneBuff;			// VM上のバッファ
	int			m_iChToneNum[6];		// チャンネル毎の音色番号
	int			m_iChVolume[6];			// チャンネル毎のボリューム
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
	// チャンネルの音色番号解析設定
	void	AnalyzeUseToneNo();
	// VMへフィードバックするためのコールバック
	static 	void vVmCallback(VOICEFORMAT *pVoiceParam, int iNum, LPVOID pParam);
	void VmCallBack(VOICEFORMAT *pVoiceParam, int iNum);

	// 音色データ保存を行うためのコールバック
	static 	void vSaveToneCallback(LPVOID pParam);
	void SaveToneCallback();

	// 音色データ設定
	BOOL SetToneData(const char *pFileName, const char *pMem);
	BOOL SetOrgToneData(const char *pMem);
	void SetUseToneNo();

	// 音色変更の許可フラグ
	BOOL allowChangeTone;

	// MUCOM88プラグイン情報
	Mucom88Plugin *m_mucom88plg;
};

