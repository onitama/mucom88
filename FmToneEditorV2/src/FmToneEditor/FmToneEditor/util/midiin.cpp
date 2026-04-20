#include "midiin.h"

#include	<crtdbg.h>
#ifdef _DEBUG
#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// コンストラクタ
midiIn::midiIn() {
	m_hMidiIn = NULL;
	m_pMidiCb = NULL;
	enumMidiInDevice();
}

// デストラクタ
midiIn::~midiIn() {
	destroyMidiInDevice();
}

// midiinデバイスを列挙する
void midiIn::enumMidiInDevice(){
	destroyMidiInDevice();
	UINT devCnt = midiInGetNumDevs();
	for (UINT i = 0; i < devCnt; i++) {
		LPMIDIINCAPS pMidiInCaps = new MIDIINCAPS();
		ZeroMemory(pMidiInCaps,sizeof(MIDIINCAPS));
		MMRESULT res = midiInGetDevCaps(i, pMidiInCaps, sizeof(MIDIINCAPS));
		if (MMSYSERR_NOERROR == res) {
			m_midiDevs.push_back(pMidiInCaps);
		}
	}
}

// MIDI情報を返却
vector<LPMIDIINCAPS> midiIn::getMidiInDeviceList(){
	return m_midiDevs;
}

// midiデバイス破棄
void midiIn::destroyMidiInDevice() {
	for (size_t i = 0; i < m_midiDevs.size(); i++) {
		// 解放
		delete m_midiDevs[i];
	}
	m_midiDevs.clear();
}

// デバイスオープン
void midiIn::open(UINT devNum, LPCSTR devName,DWORD_PTR eventCallback, DWORD_PTR drInstance){
	m_midiDevNum = devNum;
	m_pMidiCb = (MIDICB)eventCallback;
	m_hdrInstance = drInstance;
	int res = midiInOpen(&m_hMidiIn, devNum, (DWORD_PTR)&midiIn::midiInCb, (DWORD_PTR)this, CALLBACK_FUNCTION);
	midiInStart(m_hMidiIn);
}

// デバイスクローズ
void midiIn::close() {
	midiInStop(m_hMidiIn);
	midiInReset(m_hMidiIn);
	midiInClose(m_hMidiIn);
	m_hMidiIn = NULL;
}
// コールバック
void midiIn::midiInCb(HMIDIIN hMidiIn, UINT msg, DWORD_PTR drInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	midiIn *pThis = (midiIn*)drInstance;
	pThis->midiInCbMain(hMidiIn,msg,dwParam1,dwParam2);
}

// コールバックメイン
void midiIn::midiInCbMain(HMIDIIN hMidiIn, UINT msg, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	m_pMidiCb(msg,dwParam1,dwParam2, m_hdrInstance);
}
