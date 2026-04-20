/*******************************************************************************
	Dialog Base Unit Header File
*******************************************************************************/

#pragma once
#include	<windows.h>
#include	<commctrl.h>

class	DialogBase {
public:
	HINSTANCE	m_hInst;				//	App Instance Handle
	HWND		m_hWnd;					//	Owner Window Handle
	HWND		m_hDlg;					//	Dialog Window Handle
	HWND		m_hMyDlg;				//	外部からの消去様
	//	----------Constructer----------
	DialogBase(void);
	//	----------Desctructer----------
	~DialogBase(void);
	//	----------Dialog Box Function-----------
	long	Dialog(HINSTANCE hinst,LPCTSTR tmp,HWND hWnd);
	HWND	Create(HINSTANCE hinst,LPCTSTR tmp,HWND hwnd = NULL);
	void	Destroy(void);
	//	----------Dialog Callback Funtion-----------
	static BOOL CALLBACK DialogBoxFuncPass(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
	virtual BOOL CALLBACK Func(HWND hdlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
};

//	----------エディット＆スピンボタンコントロール----------
class	EDSPCTRL{
public:
	HWND	hDlg;					//	ダイアログハンドル
	DWORD	spin;					//	スピンボタン識別
	DWORD	edit;					//	ＥＤＩＴボックス識別
	long	value;					//	数値
	long	mini,max;				//	最大値最小値
//	----------コンストラクター----------
	EDSPCTRL(HWND hd,DWORD ed,DWORD sp,long vl,long mi = 0,long ma = 99){
		hDlg = hd;
		edit = ed;
		spin = sp;
		value = vl;
		mini = mi;
		max = ma;
	};
	//	----------テキスト設定----------
	void	Setup(void){
		char	str[32];
		wsprintf(str,"%d",value);
		SetDlgItemText(hDlg,edit,str);
	};
//	----------値変更----------
	void	SetValue(long vl){
		char	str[32];
		value = vl;
		//	----------editboxに指示を出す----------
		wsprintf(str,"%d",value);
		SetDlgItemText(hDlg,edit,str);
	};
//	----------値取得----------
	long	GetValue(void){
		return	value;
	}
//	----------スピン側コントロール----------
	BOOL	SpinCheck(WPARAM wParam,LPARAM lParam){
		char	str[32];
		LPNM_UPDOWN	sp;
		if(wParam != spin) return FALSE;
		//	----------数値加算減算処理----------
		sp = (LPNM_UPDOWN)lParam;
		if(sp->iDelta < 0){
			//	----------加算処理----------
			if(value < max){
				value++;
			}
		}else if(sp->iDelta > 0){
			//	----------減算処理----------
			if(value > mini){
				value--;
			}
		}else{
			return FALSE;
		}
		//	----------ＥＤＩＴＢＯＸに値を反映させる----------
		wsprintf(str,"%d",value);
		SetDlgItemText(hDlg,edit,str);
		return	TRUE;
	};
//	----------ＥＤＩＴコントロール----------
	BOOL	EditCheck(WPARAM wParam){
	if(LOWORD(wParam) != edit) return	TRUE;		//	自分では無い戻り
		char	str[32];							//	一応３２桁まで
		long	flag = 0;
		unsigned long	ct = 0;
		//	----------ＥＤＩＴの文字を認識する----------
		GetDlgItemText(hDlg,edit,str,32);
		//	----------文字が無い場合----------
		if(str[0] == 0) return FALSE;
		//	----------数値のみか？----------
		for(ct = 0; ct < strlen(str); ct++){
			if(isdigit(str[ct]) == 0) flag = 1;
		}
		if(flag == 1) return FALSE;
		//	----------問題なし----------
		value = atoi(str);
		return	TRUE;
	}
};


