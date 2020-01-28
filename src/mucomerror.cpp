
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
	// 
	"",

	// ﾌﾞﾝﾎﾟｳ ﾆ ｱﾔﾏﾘ ｶﾞ ｱﾘﾏｽ
	"\xcc\xde\xdd\xce\xdf\xb3\x20\xc6\x20\xb1\xd4\xcf\xd8\x20\xb6\xde\x20\xb1\xd8\xcf\xbd",

	// ﾊﾟﾗﾒｰﾀﾉ ｱﾀｲ ｶﾞ ｲｼﾞｮｳﾃﾞｽ
	"\xca\xdf\xd7\xd2\xb0\xc0\xc9\x20\xb1\xc0\xb2\x20\xb6\xde\x20\xb2\xbc\xde\xae\xb3\xc3\xde\xbd",

	//  ]  ﾉ ｶｽﾞ ｶﾞ ｵｵｽｷﾞﾏｽ
	"\x20\x5d\x20\x20\xc9\x20\xb6\xbd\xde\x20\xb6\xde\x20\xb5\xb5\xbd\xb7\xde\xcf\xbd",

	//  [  ﾉ ｶｽﾞ ｶﾞ ｵｵｽｷﾞﾏｽ
	"\x20\x5b\x20\x20\xc9\x20\xb6\xbd\xde\x20\xb6\xde\x20\xb5\xb5\xbd\xb7\xde\xcf\xbd",

	// ｵﾝｼｮｸ ﾉ ｶｽﾞ ｶﾞ ｵｵｽｷﾞﾏｽ
	"\xb5\xdd\xbc\xae\xb8\x20\xc9\x20\xb6\xbd\xde\x20\xb6\xde\x20\xb5\xb5\xbd\xb7\xde\xcf\xbd",

	// ｵｸﾀｰﾌﾞ ｶﾞ ｷﾃｲﾊﾝｲ ｦ ｺｴﾃﾏｽ
	"\xb5\xb8\xc0\xb0\xcc\xde\x20\xb6\xde\x20\xb7\xc3\xb2\xca\xdd\xb2\x20\xa6\x20\xba\xb4\xc3\xcf\xbd",

	// ﾘｽﾞﾑ ｶﾞ ｸﾛｯｸ ﾉ ﾁｦ ｺｴﾃﾏｽ
	"\xd8\xbd\xde\xd1\x20\xb6\xde\x20\xb8\xdb\xaf\xb8\x20\xc9\x20\xc1\xa6\x20\xba\xb4\xc3\xcf\xbd",

	// [ ] ﾅｲ ﾆ / ﾊ ﾋﾄﾂﾀﾞｹﾃﾞｽ
	"\x5b\x20\x5d\x20\xc5\xb2\x20\xc6\x20\x2f\x20\xca\x20\xcb\xc4\xc2\xc0\xde\xb9\xc3\xde\xbd",

	// ﾊﾟﾗﾒｰﾀ ｶﾞ ﾀﾘﾏｾﾝ
	"\xca\xdf\xd7\xd2\xb0\xc0\x20\xb6\xde\x20\xc0\xd8\xcf\xbe\xdd",

	// ｺﾉﾁｬﾝﾈﾙ ﾃﾞﾊ ﾂｶｴﾅｲ ｺﾏﾝﾄﾞｶﾞｱﾘﾏｽ
	"\xba\xc9\xc1\xac\xdd\xc8\xd9\x20\xc3\xde\xca\x20\xc2\xb6\xb4\xc5\xb2\x20\xba\xcf\xdd\xc4\xde\xb6\xde\xb1\xd8\xcf\xbd",

	// [ ] ﾉ ﾈｽﾄﾊ 16ｶｲ ﾏﾃﾞﾃﾞｽ
	"\x5b\x20\x5d\x20\xc9\x20\xc8\xbd\xc4\xca\x20\x31\x36\xb6\xb2\x20\xcf\xc3\xde\xc3\xde\xbd",

	// ｵﾝｼｮｸ ﾃﾞｰﾀ ｶﾞ ﾗｲﾌﾞﾗﾘ ﾆ ｿﾝｻﾞｲｼﾏｾﾝ
	"\xb5\xdd\xbc\xae\xb8\x20\xc3\xde\xb0\xc0\x20\xb6\xde\x20\xd7\xb2\xcc\xde\xd7\xd8\x20\xc6\x20\xbf\xdd\xbb\xde\xb2\xbc\xcf\xbe\xdd",

	// ｶｳﾝﾀｰ ｵｰﾊﾞｰﾌﾛｰ
	"\xb6\xb3\xdd\xc0\xb0\x20\xb5\xb0\xca\xde\xb0\xcc\xdb\xb0",

	// ﾓｰﾄﾞ ｴﾗｰ
	"\xd3\xb0\xc4\xde\x20\xb4\xd7\xb0",

	// ｵﾌﾞｼﾞｪｸﾄ ﾘｮｳｲｷ ｦ ｺｴﾏｼﾀ
	"\xb5\xcc\xde\xbc\xde\xaa\xb8\xc4\x20\xd8\xae\xb3\xb2\xb7\x20\xa6\x20\xba\xb4\xcf\xbc\xc0",

	// ﾃｲｷﾞｼﾃﾅｲ ﾏｸﾛﾅﾝﾊﾞｰｶﾞｱﾘﾏｽ
	"\xc3\xb2\xb7\xde\xbc\xc3\xc5\xb2\x20\xcf\xb8\xdb\xc5\xdd\xca\xde\xb0\xb6\xde\xb1\xd8\xcf\xbd",

	// ﾏｸﾛｴﾝﾄﾞｺｰﾄﾞ ｶﾞ ｱﾘﾏｾﾝ
	"\xcf\xb8\xdb\xb4\xdd\xc4\xde\xba\xb0\xc4\xde\x20\xb6\xde\x20\xb1\xd8\xcf\xbe\xdd"
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

