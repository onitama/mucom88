#include "realchip.h"

#ifdef _DEBUG
#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// コンストラクタ
realchip::realchip()
{
	// フラグ初期化
	m_IsRealChip = false;
}

// デストラクタ
realchip::~realchip()
{
}

// 初期化
void realchip::Initialize()
{
	// SCCIを読み込む
	m_hScci = ::LoadLibrary((LPCSTR)"scci2.dll");
	if (m_hScci == NULL) {
		return;
	}
	// サウンドインターフェースマネージャー取得用関数アドレス取得
	SCCIFUNC getSoundInterfaceManager = (SCCIFUNC)(::GetProcAddress(m_hScci, "getSoundInterfaceManager"));
	if (getSoundInterfaceManager == NULL) {
		::FreeLibrary(m_hScci);
		m_hScci = NULL;
		return;
	}
	// サウンドインターフェースマネージャー取得
	m_pManager = getSoundInterfaceManager();

	// 初期化
	m_pManager->initializeInstance();

	// リセット処理
	m_pManager->reset();

	// サウンドチップ取得
	m_pSoundChip = m_pManager->getSoundChip(SC2_TYPE_YM2608, SC2_CLOCK_7987200);

	// サウンドチップの取得が出来ない場合
	if (m_pSoundChip == NULL)
	{
		// サウンドマネージャーを解放して終了
		m_pManager->releaseInstance();
		::FreeLibrary(m_hScci);
		m_hScci = NULL;
		return;
	}
	// チップが取得できたのでリアルチップで動作させる
	m_IsRealChip = true;
}

// 開放
void realchip::UnInitialize()
{
	// リアルチップ無しなら即終了
	if (m_IsRealChip == false) return;
	// サウンドチップを開放する
	m_pManager->releaseSoundChip(m_pSoundChip);
	m_pSoundChip = NULL;
	// サウンドマネージャー開放
	m_pManager->releaseInstance();
	m_pManager = NULL;
	// DLL開放
	::FreeLibrary(m_hScci);
	m_hScci = NULL;
}

// リセット処理
void realchip::Reset()
{
	// 実チップが存在しない場合
	if (m_IsRealChip == false) {
		return;
	}
	// リセットする
	if (m_pSoundChip) {
		m_pSoundChip->init();
	}
}

// リアルチップチェック
bool realchip::IsRealChip(){
	// リアルチップの有無を返却する
	return m_IsRealChip;
}

// レジスタ設定
void realchip::SetRegister(DWORD reg, DWORD data) {
	if (m_pSoundChip) {
		m_pSoundChip->setRegister(reg, data);
	}
}

