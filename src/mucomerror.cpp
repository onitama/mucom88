
//
//	MUCOM88 debug support
//	(エラー処理)
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mucomerror.h"

/*------------------------------------------------------------*/
/*
		system data
*/
/*------------------------------------------------------------*/


/*------------------------------------------------------------*/
/*
		interface
*/
/*------------------------------------------------------------*/

static const char *orgmsg[]={
	"",
	"ﾌﾞﾝﾎﾟｳ ﾆ ｱﾔﾏﾘ ｶﾞ ｱﾘﾏｽ",
	"ﾊﾟﾗﾒｰﾀﾉ ｱﾀｲ ｶﾞ ｲｼﾞｮｳﾃﾞｽ",
	" ]  ﾉ ｶｽﾞ ｶﾞ ｵｵｽｷﾞﾏｽ",
	" [  ﾉ ｶｽﾞ ｶﾞ ｵｵｽｷﾞﾏｽ",
	"ｵﾝｼｮｸ ﾉ ｶｽﾞ ｶﾞ ｵｵｽｷﾞﾏｽ",
	"ｵｸﾀｰﾌﾞ ｶﾞ ｷﾃｲﾊﾝｲ ｦ ｺｴﾃﾏｽ",
	"ﾘｽﾞﾑ ｶﾞ ｸﾛｯｸ ﾉ ﾁｦ ｺｴﾃﾏｽ",
	"[ ] ﾅｲ ﾆ / ﾊ ﾋﾄﾂﾀﾞｹﾃﾞｽ",
	"ﾊﾟﾗﾒｰﾀ ｶﾞ ﾀﾘﾏｾﾝ",
	"ｺﾉﾁｬﾝﾈﾙ ﾃﾞﾊ ﾂｶｴﾅｲ ｺﾏﾝﾄﾞｶﾞｱﾘﾏｽ",
	"[ ] ﾉ ﾈｽﾄﾊ 16ｶｲ ﾏﾃﾞﾃﾞｽ",
	"ｵﾝｼｮｸ ﾃﾞｰﾀ ｶﾞ ﾗｲﾌﾞﾗﾘ ﾆ ｿﾝｻﾞｲｼﾏｾﾝ",
	"ｶｳﾝﾀｰ ｵｰﾊﾞｰﾌﾛｰ",
	"ﾓｰﾄﾞ ｴﾗｰ",
	"ｵﾌﾞｼﾞｪｸﾄ ﾘｮｳｲｷ ｦ ｺｴﾏｼﾀ",
	"ﾃｲｷﾞｼﾃﾅｲ ﾏｸﾛﾅﾝﾊﾞｰｶﾞｱﾘﾏｽ",
	"ﾏｸﾛｴﾝﾄﾞｺｰﾄﾞ ｶﾞ ｱﾘﾏｾﾝ",
};

static const char *err[]={
	"",												// 0
	"Syntax error",									// 1 'ﾌﾞﾝﾎﾟｳ ﾆ ｱﾔﾏﾘ ｶﾞ ｱﾘﾏｽ',0
	"Illegal parameter",							// 2 'ﾊﾟﾗﾒｰﾀﾉ ｱﾀｲ ｶﾞ ｲｼﾞｮｳﾃﾞｽ',0
	"Too many loop end ']'",						// 3 ' ]  ﾉ ｶｽﾞ ｶﾞ ｵｵｽｷﾞﾏｽ',0
	"Too many loops '['",							// 4 ' [  ﾉ ｶｽﾞ ｶﾞ ｵｵｽｷﾞﾏｽ',0
	"Too many FM voice '@'",						// 5 'ｵﾝｼｮｸ ﾉ ｶｽﾞ ｶﾞ ｵｵｽｷﾞﾏｽ',0
	"Octave out of range",							// 6 'ｵｸﾀｰﾌﾞ ｶﾞ ｷﾃｲﾊﾝｲ ｦ ｺｴﾃﾏｽ',0
	"Rhythm too fast, Clock over",					// 7 'ﾘｽﾞﾑ ｶﾞ ｸﾛｯｸ ﾉ ﾁｦ ｺｴﾃﾏｽ',0
	"Unexpected loop escape '/'",					// 8 '[ ] ﾅｲ ﾆ / ﾊ ﾋﾄﾂﾀﾞｹﾃﾞｽ',0
	"Need more parameter",							// 9 'ﾊﾟﾗﾒｰﾀ ｶﾞ ﾀﾘﾏｾﾝ',0
	"Command not supported at this CH",				// 10 'ｺﾉﾁｬﾝﾈﾙ ﾃﾞﾊ ﾂｶｴﾅｲ ｺﾏﾝﾄﾞｶﾞｱﾘﾏｽ',0
	"Too many loop nest '['-']'",					// 11 '[ ] ﾉ ﾈｽﾄﾊ 16ｶｲ ﾏﾃﾞﾃﾞｽ',0
	"FM Voice not found",							// 12 'ｵﾝｼｮｸ ﾃﾞｰﾀ ｶﾞ ﾗｲﾌﾞﾗﾘ ﾆ ｿﾝｻﾞｲｼﾏｾﾝ',0
	"Counter overflow",								// 13 'ｶｳﾝﾀｰ ｵｰﾊﾞｰﾌﾛｰ',0
	"Mode error",									// 14 'ﾓｰﾄﾞ ｴﾗｰ',0
	"Music data overflow",							// 15 'ｵﾌﾞｼﾞｪｸﾄ ﾘｮｳｲｷ ｦ ｺｴﾏｼﾀ',0
	"Undefined macro number",						// 16 'ﾃｲｷﾞｼﾃﾅｲ ﾏｸﾛﾅﾝﾊﾞｰｶﾞｱﾘﾏｽ',0
	"Macro end code not found",						// 17 'ﾏｸﾛｴﾝﾄﾞｺｰﾄﾞ ｶﾞ ｱﾘﾏｾﾝ',0
	"*"
};

static const char *err_jpn[] = {
	"",
	"文法に誤りがあります",
	"パラメーターの値が異常です",
	" ]の数が多すぎます",
	" [の数が多すぎます",
	"音色の数が多すぎます",
	"オクターブが規定範囲を超えてます",
	"リズムがクロックの値を超えてます",
	"[ ] 内に / は1つだけです",
	"パラメーターがたりません",
	"このチャンネルでは使えないコマンドがあります",
	"[ ] のネストは16回までです",
	"音色のデータがライブラリに存在しません",
	"カウンターオーバーフロー",
	"モードエラー",
	"オブジェクト領域を超えました",
	"定義していないマクロナンバーがあります",
	"マクロエンドコードがありません",
};

const char *mucom_geterror(int error)
{
	if ((error<0)||(error>=MUCOMERR_MAX)) return err[0];
	return err[error];
}

const char *mucom_geterror_j(int error)
{
	if ((error<0) || (error >= MUCOMERR_MAX)) return err[0];
	return err_jpn[error];
}

int mucom_geterror(const char *orgerror)
{
	int i;
	for (i = 1; i < MUCOMERR_MAX; i++) {
		if (strcmp(orgmsg[i], orgerror) == 0) return i;
	}
	return 0;
}

