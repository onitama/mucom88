#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <windows.h>
#include "voice.h"
#include "../util/DibModule.h"
#include "../fmgen/opna.h"

// データ変更用コールバック登録用
typedef void (*CHANGETONECALLBACK)(VOICEFORMAT *pTone,int iToneNum,LPVOID param);
typedef void(*SAVETONECALLBACK)(LPVOID param);

#define	TONE_EDIT_MSG_SET_TONENAME	(WM_APP + 102)
using namespace std;

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

	HWND						m_hOwnerWin;

	// DATファイル対応関連
	string		m_sCurrentPath;
	string		m_sDatFileName;		// ファイル名
	VOICEFORMAT	*m_pVoiceData;		// 音色データ
	VOICEFORMAT	*m_pOrgVoiceData;	// 現在編集中の音色データ

	// Debug Flag
	int m_iDebugCount;

	int m_iVolume;

	// 画面の倍率
	int		m_iScale;
	int		m_iBlink;
	int		m_iBlinkTimer;

	// 再生中のトーンナンバーを設定する
	int		m_iChToneNum[6];
	int		m_iBefChToneNum[6];
	int		m_iChAplha[6];
	// マネージャから受け取るトーン番号
	vector<int> m_vTones;

	// VM再度への通知用コールバック
	CHANGETONECALLBACK	m_pVmCallback;
	LPVOID				m_pCallbackParam;
	SAVETONECALLBACK	m_pSaveCallback;
	LPVOID				m_pSvaeCallbackParam;

public:
	// コンストラクタ
	FmToneEditor();
	// デストラクタ
	~FmToneEditor();
	// 初期化
	BOOL Initialize(HWND hWnd);
	// 開放
	BOOL UnInitialize();
private:
	// カーソル表示
	void	DrawCursor(DibModule *pDib);
	// パラメータ表示
	void	DrawParamaters(DibModule *pDib);
	// アルゴリズム表示
	void	DrawAlgorithm(DibModule *pDib);
	// エンベロープの表示
	void	DrawEnvelope(DibModule *pDib);
	// エンベロープ波形表示
	void	DrawEnbelopleWave(DibModule *pDib,int x,int y,LPSTR name,int ar,int dr,int sr,int rr,int sl);
	// 直接入力表示処理
	void	DrawInputBox(DibModule *pDib);
	// 鍵盤表示
	void	DrawKeyboard(DibModule *pDib);
	// インフォメーション表示
	void	DrawIntormation(DibModule *pDib);
	// プレビューのインフォメーション表示
	void	DrawPreviewInformation(DibModule *pDib);
	// プレイインフォメーションの表示
	void	DrawPlayInformation(DibModule *pDib);
	// SetRegister
	void	SetRegister(DWORD reg,DWORD data);
	// SetToneParam
	void	SetToneParam(VOICEFORMAT param);
public:
	// 描画処理
	void Paint(DibModule *pDib);
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
	//		音色設定
	void	SetToneData(VOICEFORMAT param,int iToneNo = 0);
	VOICEFORMAT GetToneData();
	// KeyOn
	void	KeyOn();
	// KeyOff
	void	KeyOff();
	// SetVolume
	void	SetVolume(int iVol);
	// カレント設定
	void	SetCurrentDir(const char *pParh);
	// カレント取得
	string	GetCurrentDir();
	// Datファイル設定
	void	SetDatFile(string fileName,VOICEFORMAT *pData);
	// オリジナル音色設定
	void	SetOrgToneData(VOICEFORMAT *pData);
		// 音色番号取得
	int		GetToneNo();
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
	// 音色名を設定する
	void SetToneName(string sName);
	// コールバック設定
	void	SetToneChangeCallback(CHANGETONECALLBACK pFunc,LPVOID pParam);
	// 音色保存コールバック設定
	void	SetSaveToneCallback(SAVETONECALLBACK pFunc, LPVOID pParam);
	// ファイル保存
	void	SaveDatFiele(HWND hWnd);
	// 倍率設定
	void	SetScale(int iScale) {
		m_iScale = iScale;
	}
	// タイマー処理
	void	Timer(void);
};

