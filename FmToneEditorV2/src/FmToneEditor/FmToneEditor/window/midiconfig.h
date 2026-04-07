//--------------------------------------------------
// SPFMê›íË
//--------------------------------------------------
#pragma once
#include "../window/dialogbase.h"
#include "../window/midiconfig.h"
#include "../util/midiin.h"

class SoundInterfaceManagerInner;
class SoundInterfaceMIDI;

class midiconfig:public DialogBase{
private:
	SoundInterfaceManagerInner	*m_pManager;
	SoundInterfaceMIDI			*m_pIf;
	midiIn						*m_midiIn;
	int							m_iMidiDevNum;
public:
	midiconfig(midiIn* mi,int devNum);
	~midiconfig();
	virtual BOOL CALLBACK Func(HWND hdlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
	BOOL setDlgItems();
	int getDeviceNumber();
};
