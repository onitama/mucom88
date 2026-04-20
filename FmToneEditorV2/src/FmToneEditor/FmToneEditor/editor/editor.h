#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include <windows.h>
#include "voice.h"
#include "../util/DibModule.h"
#include "realchip.h"
#include "../util/midiin.h"
#include "../window/midiconfig.h"

// データ変更用コールバック登録用
typedef void (*CHANGETONECALLBACK)(VOICEFORMAT *pTone,int iToneNum,LPVOID param);
#define	TONE_EDIT_MSG_SET_TONENAME	(WM_APP + 102)

typedef struct {
	int			track;			// トラック番号
	boolean		bKeyOn;			// チャンネルの状態
	LONGLONG	keyOnTime;		// KeyOn時の時間
	int			note;			// 再生トーン
	int			velocity;		// ベロシティー
} MidiChannel;

typedef struct {
	int	iCount;
	int iPos;
	int iModulation;
} MidiModulation;

// エディタークラス
class FmToneEditor{
private:
	// 固定リソース
	static LPCSTR m_cAlList[8];
	// 固定範囲データ
	static int m_iRange[10][4][2];

	// フォントデータ
	GraphicData *m_pFont;
	GraphicData *m_pAlImage[8];
	// 表示情報
	VOICEFORMAT m_VoiceData;
	int			m_iToneNo;
	// 選択中の項目
	int	m_SelX;
	int	m_SelY;

	int m_MouseSelX;
	int m_MouseSelY;
	// 押されているキー
	int	m_DownKey;
	int m_Octave;
	// パラメータ入力中
	BOOL m_bInputParam;
	BOOL m_bInputMouseFarst;
	BOOL m_bInputFarst;
	// 入力中のパラメータ値
	int	m_InputValue;
	// 編集中の表示文字
	char m_cDispParam[4];
	// カーソルのポジション
	int m_iInputPos;

	// マウスカーソルの位置
	int	m_MousePosX;
	int m_MousePosY;

	// Mouseの選択位置
	BOOL m_bIsToneNoSel;		// 音色
	BOOL m_bIsToneNameSel;		// 音色名

	// FM音源関連
	HWND						m_hOwnerWin;

	LONG sending;
	bool playflag;

	// 実チップ対応
	BOOL		m_bUseScci;
	realchip	*m_pChip;

	// DATファイル対応関連
	string		m_sCurrentPath;
	string		m_sDatFileName;		// ファイル名
	VOICEFORMAT	*m_pVoiceData;
	VOICEFORMAT	*m_pOrgVoiceData;
	vector<pair<int,VOICEFORMAT>> m_vMmlToneList;

	// Debug Flag
	int m_iDebugCount;
	int m_iVolume;

	// 画面の倍率
	int		m_iScale;
	int		m_iBlink;
	int		m_iBlinkTimer;

	// MIDI情報
	MidiChannel m_midiChannel[6];
	MidiModulation	m_midiModulation[16];
	int			m_iPitch[16];
	int			m_iModulation[16];
	int			m_iMidiInDevNum;
	midiIn		m_midiIn;
	int			m_iTargetTrack;

	// 再生中のトーンナンバーを設定する
	int		m_iChToneNum[6];
	// マネージャから受け取るトーン番号
	vector<int> m_vTones;

	// VM再度への通知用コールバック
	CHANGETONECALLBACK	 m_pVmCallback;
	LPVOID				m_pCallbackParam;
	UINT				m_TimerId;
	CRITICAL_SECTION	cs;

	// oct offset
	int					m_octOffset = 0;

public:
	// コンストラクタ
	FmToneEditor();
	// デストラクタ
	~FmToneEditor();
	// SCCI設定
	void	SetUseSCCI(BOOL bScci);
	// 初期化
	BOOL Initialize(HWND hWnd);
	// 開放
	BOOL UnInitialize();
private:
	// フォント描画
	void	DrawFont(DibModule *pDib,int x,int y,DWORD dColor,char *pFormat,...);
	// カーソル表示
	void	DrawCursor(DibModule *pDib);
	// パラメータ表示
	void	DrawParamaters(DibModule *pDib);
	// アルゴリズム表示
	void	DrawAlgorithm(DibModule *pDib);
	// エンベロープの表示
	void	DrawEnvelope(DibModule *pDib);
	// エンベロープ波形表示
	void	DrawEnbelopleWave(DibModule *pDib,int x,int y,LPCSTR name,int ar,int dr,int sr,int rr,int sl);
	// 直接入力表示処理
	void	DrawInputBox(DibModule *pDib);
	// 鍵盤表示
	void	DrawKeyboard(DibModule *pDib);
	// 鍵盤が押されてるかチェック
	boolean isPushKeyboard(int note,int oct);
	// インフォメーション表示
	void	DrawIntormation(DibModule *pDib);
	// プレビューのインフォメーション表示
	void	DrawPreviewInformation(DibModule *pDib);
	// SetRegister
	void	SetRegister(DWORD reg,DWORD data);
	// SetToneParam
	void	SetToneParam(VOICEFORMAT param);
	// MIDIEvent
	static void	midiCallBack(UINT msg, DWORD_PTR dwParam1, DWORD_PTR dwParam2, DWORD_PTR drInstance);
	void	midiCallBackMain(UINT msg, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	// 音量を設定する
	void SetVolume(int ch,int iVolume);
	// 音程設定
	void setNote(int ch,int note,int pitch);
	// 初期化
	void keyInitialize();
	// KeyOn
	void KeyOnMidi(int cmd,int param1,int param2);
	// KeyOff
	void KeyOffMidi(int cmd,int param1,int param2);
	// modulation
	void modulation(int cmd,int param1,int param2);
	// pitch
	void pitch(int cmd,int param1,int param2);
	// タイマー
	static void CALLBACK vTimerCallback(UINT uTimerID,UINT uMsg,LONG_PTR dwUser,DWORD dw1,DWORD dw2);
	void timerCallback(UINT uTimerID,UINT uMsg,DWORD dw1,DWORD dw2);
public:
	// 描画処理
	void	Paint(DibModule *pDib);
	// 入力の開始
	void	InputStart();
	// 入力の終了
	void	InputEnd(WPARAM wParam,LPARAM lParam);
	// キー入力処理
	void	InputKey(WPARAM wParam,LPARAM lParam);
	// キー入力系
	BOOL	KeyDown(WPARAM wParam,LPARAM lParam);
	// キー入力系
	BOOL	KeyUp(WPARAM wParam,LPARAM lParam);
	// マウス系の処理
	BOOL	MouseEvent(UINT msg, WPARAM wParam, LPARAM lParam);
	// マウスの当たり判定チェック処理
	BOOL	MouseParamHitCheck();
	// マウスの当たり判定チェック処理
	BOOL	MouseHitCheck();
	// パラメータのクリッピング
	int		ClipParam(int param,int min,int max,int add);
	// パラメータチェンジ
	BOOL	ChangeParam(int param,int selX,int selY);
	//		タイマー
	//		音色設定
	void	SetToneData(VOICEFORMAT param,int iToneNo = 0);
	void	SetToneDataChannel(unsigned char *pParam,int ch);
	VOICEFORMAT GetToneData();
	// KeyChange
	int		keyToTone(WPARAM wParam);
	// KeyOn
	void	KeyOn(WPARAM wParam);
	// KeyOff
	void	KeyOff(WPARAM wParam);
	// SetVolume
	void	SetVolume(int iVol);
	// カレント設定
	void	SetCurrentDir(const char *pParh);
	// カレント取得
	string	GetCurrentDir();
	// Datファイル設定
	void	SetDatFile(string fileName,VOICEFORMAT *pData);
	// mml上で定義している音色を設定する
	void	SetMmlTone(char *pMml);
	// 音色番号取得
	int		GetToneNo();
	// パラメータデータの取得
	VOICEFORMAT * GetLoadVoiceParams(){
		return m_pOrgVoiceData;
	};
	// パラメータデータの取得
	VOICEFORMAT * GetEditVoiceParams(){
		return m_pVoiceData;
	};
	// 再生中の番号を設定する
	void	SetPlayToneNum(int iCh,int iNum){
		m_iChToneNum[iCh] = iNum;
	}
	// 音色を設定する
	void	SetTones(vector<int> tones){
		m_vTones = tones;
		// 昇順でソートする
		std::sort(m_vTones.begin(),m_vTones.end());
	}
	// コールバック設定
	void	SetToneChangeCallback(CHANGETONECALLBACK pFunc,LPVOID pParam);
	// ファイル保存
	void	SaveDatFiele(HWND hWnd);
	// 倍率設定
	void	SetScale(int iScale) {
		m_iScale = iScale;
	}
	// MIDIデバイス設定
	void	setMidiDevice(HWND hWnd);
};

