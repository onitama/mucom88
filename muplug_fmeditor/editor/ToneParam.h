#pragma once
#include <vector>
#include <algorithm>
#include <string>
#include "voice.h"

using namespace std;

enum {
	TYPE_NONE,
	TYPE_MUCOM,
	TYPE_MMLDRV
};

class ToneParam
{
public:
	// コンストラクタ
	ToneParam();
	// デストラクタ
	~ToneParam();
	// コピー用（MUCOMパラメータ生成)
	string CopyMucomParams(VOICEFORMAT param,int iToneNo);
	// コピー用（mmldrvパラメータ生成)
	string CopyMmldrvParams(VOICEFORMAT param, int iToneNo);

	// コメントスキップ
	BOOL	SkipComma(const char *pText, int &iPos, int iLen);
	// パラメータチェック
	int	CheckParam(std::string param);
	// 数値取得
	BOOL	getValue(const char *pText, int &iPos, int iLen, int &iValue);
	// スペースをスキップする
	void	SkipSpace(const char *pText, int &iPos, int iLen);
	// 改行コードをスキップする
	void	SkipCrLf(const char *pText, int &iPos, int iLen);
	// 行をスキップする
	void	SkipLine(const char *pText, int &iPos, int iLen);
	// mmldrvコメントスキップ
	void	SkipMmldrvComment(const char *pText, int &iPos, int iLen);

	// Mucomパラメータの取得
	BOOL GetMucomParameter(std::string str, VOICEFORMAT &param, int &iToneNo);
	// Mmldrvパラメータの取得
	BOOL GetMmldrvParameter(std::string str, VOICEFORMAT &param, int &iToneNo);

	// 解析MMLデータからの
	void	AnalizeToneParams(vector<pair<int, VOICEFORMAT>> &vTone, string sMml);
	// Mucomパラメータを取得する
	BOOL GetMucomToneParam(const char *pStr,int &iPos, int iLen, VOICEFORMAT &param, int &iToneNo);
};