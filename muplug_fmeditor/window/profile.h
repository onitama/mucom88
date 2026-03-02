//----------------------------------------------------------------------
// プロファイル関連
//----------------------------------------------------------------------
#pragma once
#include <Windows.h>

// iniファイル用
class Profile{
private:
	char	m_cProfileName[MAX_PATH];
public:
	// コンストラクタ
	Profile();
	// デストラクタ
	~Profile();
	// モジュールパス設定
	BOOL	setModulePath(HINSTANCE hDll);
	// 数値パラメータ取得
	DWORD getParam(char *pSessionName,char *pKeyName,DWORD dDefaultValue = 0);
	// 数値パラメータ設定
	BOOL setParam(char *pSessionName,char *pKeyName,DWORD param);
	BOOL getParam(char *pValue,char *pSessionName,char *pKeyName,char *pDefault = "");
	BOOL setParam(char *pSessionName,char *pKeyName,char *pValue);
};

