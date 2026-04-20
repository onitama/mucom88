//----------------------------------------------------------------------
// プロファイル関連
//----------------------------------------------------------------------
#include "Profile.h"

//----------------------------------------
// コンストラクタ
//----------------------------------------
Profile::Profile(){
	// ファイルパスクリア
	memset(m_cProfileName,0x00,sizeof(m_cProfileName));
}

//----------------------------------------
// デストラクタ
//----------------------------------------
Profile::~Profile(){
}

//----------------------------------------
// 数値パラメータ取得
//----------------------------------------
BOOL	Profile::setModulePath(HINSTANCE hDll){
	size_t	iPos;
	// ここでモジュールのパスを取得して保持する
	GetModuleFileName((HMODULE)hDll,m_cProfileName,MAX_PATH);
	iPos = strlen(m_cProfileName) - 1;
	while(m_cProfileName[iPos] != '.') iPos--;
	memcpy(&m_cProfileName[iPos + 1],"ini\0",4);
	return TRUE;
}

//----------------------------------------
// 数値パラメータ取得
//----------------------------------------
DWORD Profile::getParam(const char *pSessionName,const char *pKeyName,DWORD dDefaultValue){
	return	GetPrivateProfileInt(pSessionName,pKeyName,(INT)dDefaultValue,m_cProfileName);
}

//----------------------------------------
// 数値パラメータ設定
//----------------------------------------
BOOL  Profile::setParam(const char *pSessionName,const char *pKeyName,DWORD param){
	char cValue[256];
	wsprintf(cValue,"%d",param);
	WritePrivateProfileString(pSessionName,pKeyName,cValue,m_cProfileName);
	return	TRUE;
}

//----------------------------------------
// 文字パラメータ取得
//----------------------------------------
BOOL Profile::getParam(char *pValue,const char *pSessionName,const char *pKeyName,const char *pDefault){
	GetPrivateProfileString(pSessionName,pKeyName,pDefault,pValue,64,m_cProfileName);
	return	TRUE;
}

//----------------------------------------
// 文字パラメータ設定
//----------------------------------------
BOOL  Profile::setParam(const char *pSessionName,const char *pKeyName,const char *pValue){
	WritePrivateProfileString(pSessionName,pKeyName,pValue,m_cProfileName);
	return	TRUE;
}

