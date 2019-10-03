#pragma once
#include <Windows.h>
#include "scci.h"
#include "SCCIDefines.h"

// リアルチップ制御用
class realchip
{
public:
	// コンストラクタ
	realchip();
	// デストラクタ
	~realchip();
	// 初期化
	void Initialize();
	// 開放
	void UnInitialize();
	// リセット
	void Reset();
	// リアルチップの使用チェック
	bool IsRealChip();
	// SB2(YM2608)の使用チェック
	bool IsRealChipSB2();
	// レジスタの設定
	void SetRegister(DWORD reg, DWORD data);
	// ADPCMの転送
	void SendAdpcmData(void *pData, DWORD size);
	// 認識しているチップのタイプ
	SC_CHIP_TYPE GetChipType(void) { return m_chiptype; }

private:
	// SCCI関連
	HMODULE					m_hScci;
	SoundInterfaceManager	*m_pManager;
	SoundChip				*m_pSoundChip;
	// 実チップフラグ
	bool					m_IsRealChip;
	// ADPCM用
	BYTE					m_bADPCMBuff[0x40000];
	// type情報
	SC_CHIP_TYPE			m_chiptype;
};
