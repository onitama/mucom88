#pragma once
#include	<windows.h>
#include	<vector>

using namespace std;

typedef void(*MIDICB)(UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);

class midiIn {
private:
	vector<LPMIDIINCAPS>	m_midiDevs;
	UINT					m_midiDevNum;
	HMIDIIN					m_hMidiIn;
	DWORD_PTR				m_hdrInstance;
	MIDICB					m_pMidiCb;
public:
	// コンストラクタ
	midiIn();
	// デストラクタ
	~midiIn();
	// midiinデバイスを列挙する
	void enumMidiInDevice();
	// デバイスを取得する
	vector<LPMIDIINCAPS> getMidiInDeviceList();
	// midiデバイス破棄
	void destroyMidiInDevice();
	// デバイスオープン
	void open(UINT devNum,LPCSTR devName, DWORD_PTR eventCallback, DWORD_PTR drInstance);
	// デバイスクローズ
	void close();
	// MIDIinコールバック
	static void CALLBACK midiInCb(HMIDIIN hMidiIn, UINT msg, DWORD_PTR drInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	// MIDIinコールバック
	void CALLBACK midiInCbMain(HMIDIIN hMidiIn, UINT msg,  DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	// MIDIデバイス選択
	void selectMidiDevice();

};
