#include "midiin.h"

#include	<crtdbg.h>
#ifdef _DEBUG
#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// コンストラクタ
midiIn::midiIn() {
	m_hMidiIn = NULL;
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

// midiデバイス破棄
void midiIn::destroyMidiInDevice() {
	for (size_t i = 0; i < m_midiDevs.size(); i++) {
		// 解放
		delete m_midiDevs[i];
	}
}

// デバイスオープン
void midiIn::open(UINT devNum, LPSTR devName){
	int res = midiInOpen(&m_hMidiIn, devNum, (DWORD_PTR)&midiIn::midiInCb, (DWORD_PTR)this, CALLBACK_FUNCTION);
}

// コールバック
void midiIn::midiInCb(HMIDIIN hMidiIn, UINT msg, DWORD_PTR drInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
	midiIn *pThis = (midiIn*)drInstance;
	pThis->midiInCbMain(hMidiIn,msg,dwParam1,dwParam2);
}

// コールバックメイン
void midiIn::midiInCbMain(HMIDIIN hMidiIn, UINT msg, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
}
