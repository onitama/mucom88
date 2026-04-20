#include "ToneParam.h"

// コンストラクタ
ToneParam::ToneParam() {

}

// デストラクタ
ToneParam::~ToneParam() {

}

// コピー用（MUCOMパラメータ生成)
string ToneParam::CopyMucomParams(VOICEFORMAT param, int iToneNo) {
	char	sText[1024];
	char	voiceName[7];
	char	strBuff[256];
	// クリア
	ZeroMemory(sText, 1024);
	// Mucom形式でクリップボードへデータを設定する
	// 名前をコピーする
	memset(voiceName, 0x00, sizeof(voiceName));
	memcpy(voiceName, param.name, 6);
	// 名前の後ろがスペースなら0にする
	for (int j = 5; j >= 0; j--) {
		if (voiceName[j] == 0x20 || (unsigned char)voiceName[j] >= 0x80) {
			voiceName[j] = 0x00;
		}
		else {
			break;
		}
	}
	// 一行目　音色番号
	wsprintf(strBuff, "  @%d:{\r\n", iToneNo);
	lstrcat(sText, strBuff);

	// 2行目　FB AL
	wsprintf(strBuff, " %3d,%3d\r\n", param.fb, param.al);
	lstrcat(sText, strBuff);

	// 2行目 AR DR SR RR SL TL KS ML DT
	wsprintf(strBuff, " %3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d\r\n",
		param.ar_op1,
		param.dr_op1,
		param.sr_op1,
		param.rr_op1,
		param.sl_op1,
		param.tl_op1,
		param.ks_op1,
		param.ml_op1,
		param.dt_op1);
	lstrcat(sText, strBuff);

	// 3行目 AR DR SR RR SL TL KS ML DT
	wsprintf(strBuff, " %3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d\r\n",
		param.ar_op2,
		param.dr_op2,
		param.sr_op2,
		param.rr_op2,
		param.sl_op2,
		param.tl_op2,
		param.ks_op2,
		param.ml_op2,
		param.dt_op2);
	lstrcat(sText, strBuff);

	// 4行目 AR DR SR RR SL TL KS ML DT
	wsprintf(strBuff, " %3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d\r\n",
		param.ar_op3,
		param.dr_op3,
		param.sr_op3,
		param.rr_op3,
		param.sl_op3,
		param.tl_op3,
		param.ks_op3,
		param.ml_op3,
		param.dt_op3);
	lstrcat(sText, strBuff);

	// 5行目 AR DR SR RR SL TL KS ML DT
	wsprintf(strBuff, " %3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,%3d,\"%s\"}\r\n\r\n",
		param.ar_op4,
		param.dr_op4,
		param.sr_op4,
		param.rr_op4,
		param.sl_op4,
		param.tl_op4,
		param.ks_op4,
		param.ml_op4,
		param.dt_op4,
		&voiceName);
	lstrcat(sText, strBuff);
	return sText;
}

// コメントスキップ
BOOL	ToneParam::SkipComma(const char *pText, int &iPos, int iLen) {
	SkipSpace(pText, iPos, iLen);
	if (pText[iPos] == ',') iPos++;
	SkipSpace(pText, iPos, iLen);
	return TRUE;
}
// パラメータチェック
int	ToneParam::CheckParam(std::string param) {
	int	iType = TYPE_NONE;

	// @まで検索する
	const char *pStr = param.c_str();
	int i;
	for (i = 0; i < (int)param.length(); i++) {
		if (pStr[i] == '@') break;
	}
	// @が存在しない場合
	if (param.length() == i) {
		return iType;
	}
	// 音色番号を取得する
	i++;
	int iToneNum = 0;
	if (getValue(pStr, i, (int)param.length(), iToneNum) == FALSE) {
		return iType;
	}
	// スペースがあるかも知れないのでスキップ
	SkipSpace(pStr, i, (int)param.length());
	// MUCOM形式かチェック
	if (pStr[i] == ':' && pStr[i + 1] == '{') {
		// Mucomのパラメータとして認識する
		iType = TYPE_MUCOM;
	}
	else {
		// Mmldrvのパラメータとして認識する
		iType = TYPE_MMLDRV;
	}
	// 次の行から情報を取得する
	return iType;
}

// 数値を取得する
BOOL	ToneParam::getValue(const char *pText, int &iPos, int iLen, int &iValue) {
	iValue = 0;
	if (pText[iPos] < '0' || pText[iPos] > '9') {
		iValue = 0;
		return FALSE;
	}
	while (iPos < iLen) {
		if (pText[iPos] >= '0' && pText[iPos] <= '9') {
			iValue *= 10;
			iValue += pText[iPos] - '0';
			iPos++;
			continue;
		}
		break;
	}
	return TRUE;
}

// スペースをスキップする
void	ToneParam::SkipSpace(const char *pText, int &iPos, int iLen) {
	while (iPos < iLen) {
		if (pText[iPos] == 0x20) {
			iPos++;
			continue;
		}
		break;
	}
}

// 改行コードをスキップする
void	ToneParam::SkipCrLf(const char *pText, int &iPos, int iLen) {
	// CRだったらポジションを移動する
	if (pText[iPos] == 0x0d) {
		iPos++;
	}
	// LFだったらポジションを移動する
	if (pText[iPos] == 0x0a) {
		iPos++;
	}
}

// 行をスキップする
void	ToneParam::SkipLine(const char *pText, int &iPos, int iLen) {
	// 基本は文字数でループする
	while (iPos < iLen) {
		if (pText[iPos] == 0x0d) {
			iPos++;
			if (iPos < iLen) {
				if (pText[iPos] == 0x0a) {
					iPos++;
					break;
				}
			}
			break;
		}
		iPos++;
	}
}

// mmldrvコメント
void	ToneParam::SkipMmldrvComment(const char *pText, int &iPos, int iLen) {
	while (iPos < iLen) {
		if (pText[iPos] == 0x0d || pText[iPos] == 0x0a) {
			SkipCrLf(pText, iPos, iLen);
			break;
		}
		iPos++;
	}
}


// コピー用（mmldrvパラメータ生成)
string ToneParam::CopyMmldrvParams(VOICEFORMAT param, int iToneNo) {
	char	sText[1024];
	char	voiceName[7];
	char	strBuff[256];
	// クリア
	ZeroMemory(sText, 1024);
	// 名前をコピーする
	memset(voiceName, 0x00, sizeof(voiceName));
	memcpy(voiceName, param.name, 6);
	// 名前の後ろがスペースなら0にする
	for (int j = 5; j >= 0; j--) {
		if (voiceName[j] == 0x20 || (unsigned char)voiceName[j] >= 0x80) {
			voiceName[j] = 0x00;
		}
		else {
			break;
		}
	}
	// 2行目　音色番号
	wsprintf(strBuff, "' %s\r\n", voiceName);
	lstrcat(sText, strBuff);

	// 2行目　音色番号
	wsprintf(strBuff, "@%d\r\n", iToneNo);
	lstrcat(sText, strBuff);

	// 3行目　コメント
	wsprintf(strBuff, "' AL   FB\r\n");
	lstrcat(sText, strBuff);

	// 4行目 AL　FB
	wsprintf(strBuff, "  %2d   %2d\r\n", param.al, param.fb);
	lstrcat(sText, strBuff);

	// 5行目　コメント
	wsprintf(strBuff, "' AR   DR  SR  RR  SL  TL  KS  ML DT1 DT2 AMS\r\n");
	lstrcat(sText, strBuff);

	// 6行目 AR DR SR RR SL TL KS ML DT
	wsprintf(strBuff, "   %2d  %2d  %2d  %2d %3d  %2d  %2d  %2d  %2d  %2d  %2d\r\n",
		param.ar_op1,
		param.dr_op1,
		param.sr_op1,
		param.rr_op1,
		param.sl_op1,
		param.tl_op1,
		param.ks_op1,
		param.ml_op1,
		param.dt_op1,
		0,
		param.am_op1);
	lstrcat(sText, strBuff);

	// 7行目 AR DR SR RR SL TL KS ML DT
	wsprintf(strBuff, "   %2d  %2d  %2d  %2d %3d  %2d  %2d  %2d  %2d  %2d  %2d\r\n",
		param.ar_op2,
		param.dr_op2,
		param.sr_op2,
		param.rr_op2,
		param.sl_op2,
		param.tl_op2,
		param.ks_op2,
		param.ml_op2,
		param.dt_op2,
		0,
		param.am_op2);
	lstrcat(sText, strBuff);

	// 8行目 AR DR SR RR SL TL KS ML DT
	wsprintf(strBuff, "   %2d  %2d  %2d  %2d %3d  %2d  %2d  %2d  %2d  %2d  %2d\r\n",
		param.ar_op3,
		param.dr_op3,
		param.sr_op3,
		param.rr_op3,
		param.sl_op3,
		param.tl_op3,
		param.ks_op3,
		param.ml_op3,
		param.dt_op3,
		0,
		param.am_op1);
	lstrcat(sText, strBuff);

	// 9行目 AR DR SR RR SL TL KS ML DT
	wsprintf(strBuff, "   %2d  %2d  %2d  %2d %3d  %2d  %2d  %2d  %2d  %2d  %2d\r\n",
		param.ar_op4,
		param.dr_op4,
		param.sr_op4,
		param.rr_op4,
		param.sl_op4,
		param.tl_op4,
		param.ks_op4,
		param.ml_op4,
		param.dt_op4,
		0,
		param.am_op4);
	lstrcat(sText, strBuff);

	wsprintf(strBuff, "\r\n");
	lstrcat(sText, strBuff);

	return sText;
}

// Mucomパラメータの取得
BOOL ToneParam::GetMucomParameter(std::string str, VOICEFORMAT &param,int &iToneNo) {
	const char *pStr = str.c_str();
	int	iPos = 0;
	int iLen = (int)str.length();
	VOICEFORMAT inParam;
	ZeroMemory(&inParam, sizeof(inParam));

	// @音色コードが始まるまでパースする
	while (iPos < iLen) {
		if (pStr[iPos] == '@') break;
		iPos++;
	}
	// 最後まで@を検索できない場合はエラーで終了
	if (iPos == iLen) return FALSE;
	iPos++;
	// 次が数値パラメータかチェック
	int iValue = 0;
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	// 文字列の最後に到達していたらエラーにする
	if (iPos == iLen) return FALSE;
	iToneNo = iValue;
	// 数値の次をが問題ないかチェックする
	if (pStr[iPos] != ':' && pStr[iPos + 1] != '{') {
		// MUCOMの音色データではないのでエラー
		return FALSE;
	}
	iPos += 2;
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);

	// フィードバックを取得する
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.fb = iValue;
	SkipSpace(pStr, iPos, iLen);
	// カンマがあるか
	if (pStr[iPos] != ',') {
		// エラーにする
		return FALSE;
	}
	iPos++;
	SkipSpace(pStr, iPos, iLen);
	// アルゴリズムを取得する
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.al = iValue;
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);

	// OP1のパラメータ取得する
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ar_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dr_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sr_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.rr_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sl_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.tl_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ks_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ml_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dt_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// 改行がくるはずなのでスキップする
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);

	// OP2のパラメータ取得する
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ar_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dr_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sr_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.rr_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sl_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.tl_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ks_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ml_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dt_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// 改行がくるはずなのでスキップする
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);

	// OP3のパラメータ取得する
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ar_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dr_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sr_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.rr_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sl_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.tl_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ks_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ml_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dt_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// 改行がくるはずなのでスキップする
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);

	// OP4のパラメータ取得する
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ar_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dr_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sr_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.rr_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sl_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.tl_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ks_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ml_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dt_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// コメントがくるはずなので"チェック
	if (pStr[iPos] != '\"') {
		// エラーにする
	}
	iPos++;
	// コメントを取得する
	BOOL bComment = FALSE;
	string comment = "";
	while (iPos < iLen) {
		if (pStr[iPos] == '\"') {
			bComment = TRUE;
			break;
		}
		comment += pStr[iPos];
		iPos++;
	}
	// コメント領域があるか
	if (bComment == FALSE) {
		// エラーにする
		return FALSE;
	}
	// コメントを設定する
	strcpy_s(inParam.name,6,comment.c_str());
	// 一通りパースで着たのでパラメータを設定する
	param = inParam;

	return TRUE;
}

// Mmldrvパラメータの取得
BOOL ToneParam::GetMmldrvParameter(std::string str, VOICEFORMAT &param, int &iToneNo) {
	const char *pStr = str.c_str();
	int	iPos = 0;
	int iLen = (int)str.length();
	VOICEFORMAT inParam;
	ZeroMemory(&inParam, sizeof(inParam));

	// スペースとか改行をスキップする
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);

	// 先頭にコメントがある場合はコメントとして処理する
	string comment = "";
	// コメントスキップ
	if (pStr[iPos] == '\'' || pStr[iPos] == '/') {
		if (pStr[iPos] == '/' && pStr[iPos + 1] == '/') {
			iPos++;
		}
		iPos++;
		SkipSpace(pStr, iPos, iLen);
		// とりあえずコピーする
		while (iPos < iLen) {
			if (pStr[iPos] < 0x20 || pStr[iPos] > 0x7e) break;
			comment += pStr[iPos];
			iPos++;
		}
	}
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);
	// 音色があるかチェック
	if (pStr[iPos] != '@') {
		// エラー
		return FALSE;
	}
	iPos++;
	int iValue = 0;
	// al
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	// 音色番号を取得
	iToneNo = iValue;
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);

	// 以降コメント行は無視
	if (pStr[iPos] == '\'' || pStr[iPos] == '/') {
		SkipMmldrvComment(pStr, iPos, iLen);
	}
	SkipSpace(pStr, iPos, iLen);

	// al
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.al = iValue;
	SkipSpace(pStr, iPos, iLen);
	// fb
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.fb = iValue;
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);
	if (pStr[iPos] == '\'' || pStr[iPos] == '/') {
		SkipMmldrvComment(pStr, iPos, iLen);
		SkipSpace(pStr, iPos, iLen);
	}

	// OP1パラメータ取得
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ar_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dr_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sr_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.rr_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sl_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.tl_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ks_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ml_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dt_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dt2
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	SkipSpace(pStr, iPos, iLen);
	// AMD
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.am_op1 = iValue;
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);
	if (pStr[iPos] == '\'' || pStr[iPos] == '/') {
		SkipMmldrvComment(pStr, iPos, iLen);
		SkipSpace(pStr, iPos, iLen);
	}

	// OP2パラメータ取得
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ar_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dr_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sr_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.rr_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sl_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.tl_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ks_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ml_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dt_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dt2
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	SkipSpace(pStr, iPos, iLen);
	// AMD
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.am_op2 = iValue;
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);
	if (pStr[iPos] == '\'' || pStr[iPos] == '/') {
		SkipMmldrvComment(pStr, iPos, iLen);
		SkipSpace(pStr, iPos, iLen);
	}

	// OP3パラメータ取得
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ar_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dr_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sr_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.rr_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sl_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.tl_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ks_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ml_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dt_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dt2
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	SkipSpace(pStr, iPos, iLen);
	// AMD
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.am_op3 = iValue;
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);
	if (pStr[iPos] == '\'' || pStr[iPos] == '/') {
		SkipMmldrvComment(pStr, iPos, iLen);
		SkipSpace(pStr, iPos, iLen);
	}

	// OP4パラメータ取得
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ar_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dr_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sr_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.rr_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.sl_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.tl_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ks_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.ml_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.dt_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// dt2
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	SkipSpace(pStr, iPos, iLen);
	// AMD
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	inParam.am_op4 = iValue;
	SkipSpace(pStr, iPos, iLen);
	// コメントを設定しておく
	strcpy_s(inParam.name,6,comment.c_str());
	// すべて正常だったのでパラメータを返却する
	param = inParam;
	
	return TRUE;
}

// 音色データの解析
void ToneParam::AnalizeToneParams(vector<pair<int, VOICEFORMAT>> &vTone, string sMml) {
	// ここでデータをパースしてゆく
	const char *pStr = sMml.c_str();
	int	iPos = 0;
	int iLen = (int)sMml.length();
	int iToneNo = 0;
	VOICEFORMAT param;
	pair<int, VOICEFORMAT> paramSet;
	ZeroMemory(&param, sizeof(VOICEFORMAT));

	// 以下でパース処理を行う
	while (iPos < iLen) {
		// 行先頭のスペースはスキップする
		SkipSpace(pStr, iPos, iLen);
		// 先頭が@の行の場合
		if (pStr[iPos] == '@') {
			// ここでパース処理を実行する
			ZeroMemory(&param, sizeof(VOICEFORMAT));
			if (GetMucomToneParam(pStr, iPos, iLen, param, iToneNo) == FALSE) {
				return;
			}
			// 音色情報を追加する
			paramSet = make_pair(iToneNo, param);
			vTone.push_back(paramSet);
		}
		else
		{
			// 改行まで進める
			SkipLine(pStr, iPos, iLen);
		}
	}
}

// Mucomパラメータを取得する
BOOL ToneParam::GetMucomToneParam(const char *pStr, int &iPos, int iLen, VOICEFORMAT &param, int &iToneNo) {
	ZeroMemory(&param, sizeof(param));

	// @音色コードが始まるまでパースする
	while (iPos < iLen) {
		if (pStr[iPos] == '@') break;
		iPos++;
	}
	// 最後まで@を検索できない場合はエラーで終了
	if (iPos >= iLen) return FALSE;
	iPos++;
	// 次が数値パラメータかチェック
	int iValue = 0;
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	// 文字列の最後に到達していたらエラーにする
	if (iPos >= iLen) return FALSE;
	iToneNo = iValue;
	// 数値の次をが問題ないかチェックする
	if (pStr[iPos] != ':' && pStr[iPos + 1] != '{') {
		// MUCOMの音色データではないのでエラー
		return FALSE;
	}
	iPos += 2;
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);
	if (iPos >= iLen) return FALSE;

	// フィードバックを取得する
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.fb = iValue;
	SkipSpace(pStr, iPos, iLen);
	if (iPos >= iLen) return FALSE;
	// カンマがあるか
	if (pStr[iPos] != ',') {
		// エラーにする
		return FALSE;
	}
	iPos++;
	SkipSpace(pStr, iPos, iLen);
	// アルゴリズムを取得する
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.al = iValue;
	SkipSpace(pStr, iPos, iLen);
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);
	if (iPos >= iLen) return FALSE;

	// OP1のパラメータ取得する
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ar_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.dr_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.sr_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.rr_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.sl_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.tl_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ks_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ml_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.dt_op1 = iValue;
	SkipComma(pStr, iPos, iLen);
	// 改行がくるはずなのでスキップする
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);
	if (iPos >= iLen) return FALSE;

	// OP2のパラメータ取得する
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ar_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.dr_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.sr_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.rr_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.sl_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.tl_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ks_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ml_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.dt_op2 = iValue;
	SkipComma(pStr, iPos, iLen);
	// 改行がくるはずなのでスキップする
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);
	if (iPos >= iLen) return FALSE;

	// OP3のパラメータ取得する
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ar_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.dr_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.sr_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.rr_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.sl_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.tl_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ks_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ml_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.dt_op3 = iValue;
	SkipComma(pStr, iPos, iLen);
	// 改行がくるはずなのでスキップする
	SkipCrLf(pStr, iPos, iLen);
	SkipSpace(pStr, iPos, iLen);

	// OP4のパラメータ取得する
	// ar
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ar_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.dr_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.sr_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// rr
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.rr_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// sl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.sl_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// tl
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.tl_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ks
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ks_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// ml
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.ml_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// dt
	if (getValue(pStr, iPos, iLen, iValue) == FALSE) {
		// エラーにする
		return FALSE;
	}
	param.dt_op4 = iValue;
	SkipComma(pStr, iPos, iLen);
	// コメントがくるはずなので"チェック
	if (pStr[iPos] != '\"') {
		// エラーにする
	}
	iPos++;
	// コメントを取得する
	BOOL bComment = FALSE;
	string comment = "";
	while (iPos < iLen) {
		if (pStr[iPos] == '\"') {
			bComment = TRUE;
			break;
		}
		comment += pStr[iPos];
		iPos++;
	}
	// コメント領域があるか
	if (bComment == FALSE) {
		// エラーにする
		return FALSE;
	}
	// コメントを設定する
	strcpy_s(param.name,6,comment.c_str());
	// 改行コードはスキップする
	SkipCrLf(pStr, iPos, iLen);

	return TRUE;

}

