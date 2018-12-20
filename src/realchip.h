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
	// レジスタの設定
	void SetRegister(DWORD reg, DWORD data);
	// ADPCMの転送
	void SendAdpcmData(void *pData, DWORD size);

private:
	// SCCI関連
	HMODULE					m_hScci;
	SoundInterfaceManager	*m_pManager;
	SoundChip				*m_pSoundChip;
	// 実チップフラグ
	bool					m_IsRealChip;
	// ADPCM用
	BYTE					m_bADPCMBuff[0x40000];
};
