#pragma once
#include <Windows.h>
#include "scci2.h"

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

private:
	// SCCI関連
	HMODULE						m_hScci;
	Scci2SoundInterfaceManager	*m_pManager;
	Scci2SoundChip				*m_pSoundChip;
	// 実チップフラグ
	bool						m_IsRealChip;
};
