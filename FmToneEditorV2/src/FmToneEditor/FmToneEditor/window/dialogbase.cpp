/******************************************************************************
	Dialog base Unit
******************************************************************************/

#include	<windows.h>
#include	<commctrl.h>
#include	<string.h>
#include	"DialogBase.h"
#include	<crtdbg.h>
#ifdef _DEBUG
#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

/*******************************************************************************
	基本ダイアログクラス
*******************************************************************************/
//	----------Constructer----------
DialogBase::DialogBase(void){
	//	----------Values Initialize----------
	m_hInst = NULL;
	m_hWnd = NULL;
	m_hDlg = NULL;
}

//	----------Desctructer----------
DialogBase::~DialogBase(void){
}

//	----------DialogBox-----------
long	DialogBase::Dialog(HINSTANCE hinst,LPCTSTR tmp,HWND hwnd = NULL){
	m_hInst = hinst;
	m_hWnd = hwnd;
	long lRes = (long)DialogBoxParam(hinst,tmp,m_hWnd,(DLGPROC)DialogBoxFuncPass,(LPARAM)this);
	return lRes;
}

//	---------- Create DialogBox ---------
HWND	DialogBase::Create(HINSTANCE hinst,LPCTSTR tmp,HWND hwnd){
	m_hInst = hinst;
	m_hWnd = hwnd;
	m_hMyDlg = CreateDialogParam(m_hInst,tmp,m_hWnd,(DLGPROC)DialogBoxFuncPass,(LPARAM)this);
	return	m_hMyDlg;
}

//	--------- Destroy ---------
void	DialogBase::Destroy(void){
	DestroyWindow(m_hMyDlg);				//	自分を殺す
}

//	----------Dialog Callback Function Hooker----------
BOOL CALLBACK DialogBase::DialogBoxFuncPass(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	DialogBase	*pDlg = NULL;
	if(uMsg == WM_INITDIALOG){
		SetWindowLongPtr(hDlg,DWLP_USER,(LONG_PTR)lParam);
		((DialogBase*)lParam)->m_hDlg = hDlg;
	}
	pDlg = (DialogBase*)(::GetWindowLongPtr(hDlg,DWLP_USER));
	if(pDlg == NULL) return FALSE;											//	Skip Dialog Class Pointer Null
	return pDlg->Func(hDlg,uMsg,wParam,lParam);								//	Call Dialog Callback
}

//	----------Dialog Calback Funtion----------
BOOL	CALLBACK DialogBase::Func(HWND hdlg,UINT uMsg,WPARAM wParam,LPARAM lParam){
	switch(uMsg){
		case	WM_INITDIALOG:
			break;
		case	WM_COMMAND:
			//	---------Dialog Command Action----------
			switch(LOWORD(wParam)){
				case	IDCANCEL:
				case	IDOK:
					SendMessage(m_hDlg,WM_CLOSE,0,0);
					break;
			}
			break;
		case	WM_CLOSE:
			EndDialog(m_hDlg,NULL);
			break;
		default:
			return FALSE;
	}
	return	TRUE;
}

