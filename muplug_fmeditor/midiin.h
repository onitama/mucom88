#pragma once
#include	<windows.h>
#include	<vector>

using namespace std;

class midiIn {
private:
	vector<LPMIDIINCAPS>	m_midiDevs;
	DWORD_PTR				m_midiInCb;
	HMIDIIN					m_hMidiIn;
public:
	// コンストラクタ
	midiIn();
	// デストラクタ
	~midiIn();
	// midiinデバイスを列挙する
	void enumMidiInDevice();
	// midiデバイス破棄
	void destroyMidiInDevice();
	// デバイスオープン
	void open(UINT devNum,LPSTR devName);
	// MIDIinコールバック
	static void CALLBACK midiInCb(HMIDIIN hMidiIn, UINT msg, DWORD_PTR drInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2);
	// MIDIinコールバック
	void CALLBACK midiInCbMain(HMIDIIN hMidiIn, UINT msg,  DWORD_PTR dwParam1, DWORD_PTR dwParam2);

};
