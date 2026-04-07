//--------------------------------------------------
// SPFM設定
//--------------------------------------------------
#pragma once
#include "midiconfig.h"
#include "../resource.h"
#include <regex>

// コンストラクタ
midiconfig::midiconfig(midiIn *mi,int devNum){
	m_pManager = NULL;
	m_pIf = NULL;
	m_midiIn = mi;
	m_iMidiDevNum = devNum;
}

// デストラクタ
midiconfig::~midiconfig(){
}

// メッセージループ
BOOL CALLBACK midiconfig::Func(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		case WM_INITDIALOG:
			setDlgItems();
			break;
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDOK:
					m_iMidiDevNum = (int)SendDlgItemMessage(m_hDlg,IDC_COMBO1,CB_GETCURSEL,0,0);
					EndDialog(hDlg,m_iMidiDevNum);
					break;
				case IDCANCEL:
					EndDialog(hDlg,-1);
					break;
			}
			break;
		case WM_CLOSE:
			EndDialog(hDlg,NULL);
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

BOOL midiconfig::setDlgItems(){
	// コンボボックスを設定
	SendDlgItemMessage(m_hDlg,IDC_COMBO1,CB_DELETESTRING,0,0);
	SendDlgItemMessage(m_hDlg,IDC_COMBO1,CB_SETITEMHEIGHT,0,16);

	// コンボにMIDIデバイスを設定する
	m_midiIn->enumMidiInDevice();
	vector<LPMIDIINCAPS> devList = m_midiIn->getMidiInDeviceList();
	if(devList.size() > 0){
		for(size_t i = 0; i < devList.size(); i++){
			SendDlgItemMessage(m_hDlg,IDC_COMBO1,CB_ADDSTRING,0,(LPARAM)devList[i]->szPname);
		}
		SendDlgItemMessage(m_hDlg,IDC_COMBO1,CB_SETCURSEL,(WPARAM)((m_iMidiDevNum < (int)devList.size()) ? m_iMidiDevNum : 0),0);	
	}
	return TRUE;
}

// 設定デバイス番号を取得
int midiconfig::getDeviceNumber(){
	return m_iMidiDevNum;
}