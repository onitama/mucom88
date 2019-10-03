#include "realchip.h"

// コンストラクタ
realchip::realchip()
{
	// フラグ初期化
	m_IsRealChip = false;
	// ADPCM用バッファクリア
	memset(m_bADPCMBuff, 0x00, sizeof(m_bADPCMBuff));
}

// デストラクタ
realchip::~realchip()
{
}

// 初期化
void realchip::Initialize()
{
	// SCCIを読み込む
	m_hScci = ::LoadLibrary((LPCSTR)"scci.dll");
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
	m_chiptype = SC_TYPE_YM2608;
	m_pSoundChip = m_pManager->getSoundChip(m_chiptype, SC_CLOCK_7987200);

	// サウンドチップの取得が出来ない場合
	if (m_pSoundChip == NULL)
	{
		m_chiptype = SC_TYPE_YM2203;
		m_pSoundChip = m_pManager->getSoundChip(m_chiptype, SC_CLOCK_3993600);	// 2203も探す
		if (m_pSoundChip == NULL) {
			// サウンドマネージャーを解放して終了
			m_pManager->releaseInstance();
			::FreeLibrary(m_hScci);
			m_hScci = NULL;
			m_chiptype = SC_TYPE_NONE;
			return;
		}
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
bool realchip::IsRealChip() {
	// リアルチップの有無を返却する
	return m_IsRealChip;
}

// SB2チップチェック
bool realchip::IsRealChipSB2() {
	// SB2(YM2608)の有無を返却する
	return (m_chiptype == SC_TYPE_YM2608);
}

// レジスタ設定
void realchip::SetRegister(DWORD reg, DWORD data) {
	if (m_pSoundChip) {
		if (reg & 0x100) {
			if (m_chiptype != SC_TYPE_YM2608) return;
		}
		m_pSoundChip->setRegister(reg, data);
	}
}

// ADPCMの転送
void realchip::SendAdpcmData(void *pData, DWORD size) {

	if (m_chiptype != SC_TYPE_YM2608) return;

	// 差分があるかチェック
	if (memcmp(m_bADPCMBuff, pData, size) == 0) {
		// 差分が無い場合は転送しない
		return;
	}
	// メモリからコピーする
	memcpy(m_bADPCMBuff, pData, size);
	// 半端分？はゼロ生めする
	memset(&m_bADPCMBuff[size], 0x00, 0x40000 - size);
	// 転送サイズを計算する（パディングを考慮）
	DWORD transSize = size + ((0x20 - (size & 0x1f)) & 0x1f);

	m_pSoundChip->setRegister(0x100, 0x20);
	m_pSoundChip->setRegister(0x100, 0x21);
	m_pSoundChip->setRegister(0x100, 0x00);
	m_pSoundChip->setRegister(0x110, 0x00);
	m_pSoundChip->setRegister(0x110, 0x80);

	m_pSoundChip->setRegister(0x100, 0x61);
	m_pSoundChip->setRegister(0x100, 0x68);
	m_pSoundChip->setRegister(0x101, 0x00);

	// アドレス
	m_pSoundChip->setRegister(0x102, 0x00);
	m_pSoundChip->setRegister(0x103, 0x00);
	m_pSoundChip->setRegister(0x104, 0xff);
	m_pSoundChip->setRegister(0x105, 0xff);
	m_pSoundChip->setRegister(0x10c, 0xff);
	m_pSoundChip->setRegister(0x10d, 0xff);
	// PCM転送
	for (DWORD dCnt = 0; dCnt < transSize; dCnt++) {
		m_pSoundChip->setRegister(0x108, m_bADPCMBuff[dCnt]);
	}
	// 終了
	m_pSoundChip->setRegister(0x100, 0x00);
	m_pSoundChip->setRegister(SC_WAIT_REG, 16);
	m_pSoundChip->setRegister(0x110, 0x80);
	m_pSoundChip->setRegister(SC_WAIT_REG, 16);

	// バッファが空になるまで待たせる
	while (!m_pSoundChip->isBufferEmpty()) {
		Sleep(0);
	}
	
}
