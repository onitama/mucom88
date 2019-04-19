
//
//	hspvar.cpp header
//
#ifndef __hspvar_core_h
#define __hspvar_core_h

#define HSPVAR_FLAG_NONE 0
#define HSPVAR_FLAG_LABEL 1
#define HSPVAR_FLAG_STR 2
#define HSPVAR_FLAG_DOUBLE 3
#define HSPVAR_FLAG_INT 4
#define HSPVAR_FLAG_STRUCT 5
#define HSPVAR_FLAG_COMSTRUCT 6

//	7はVARIANTで予約済み

#define HSPVAR_FLAG_USERDEF 8
#define HSPVAR_FLAG_MAX 8

#define HSPVAR_MODE_NONE -1
#define HSPVAR_MODE_MALLOC 1
#define HSPVAR_MODE_CLONE 2

#define HSPVAR_ERROR_INVALID HSPERR_WRONG_EXPRESSION
#define HSPVAR_ERROR_DIVZERO HSPERR_DIVIDED_BY_ZERO
#define HSPVAR_ERROR_TYPEMISS HSPERR_TYPE_MISMATCH
#define HSPVAR_ERROR_ARRAYOVER HSPERR_ARRAY_OVERFLOW
#define HSPVAR_ERROR_ILLEGALPRM HSPERR_ILLEGAL_FUNCTION

#define HSPVAR_SUPPORT_STORAGE 1				// 固定長ストレージサポート
#define HSPVAR_SUPPORT_FLEXSTORAGE 2			// 可変長ストレージサポート
#define HSPVAR_SUPPORT_FIXEDARRAY 4				// 配列サポート
#define HSPVAR_SUPPORT_FLEXARRAY 8				// 可変長配列サポート
#define HSPVAR_SUPPORT_ARRAYOBJ 16				// 連想配列サポート
#define HSPVAR_SUPPORT_FLEXSIZE 32				// 要素ごとのデータが可変長
#define HSPVAR_SUPPORT_NOCONVERT 64				// 代入時の型変換を無効にする
#define HSPVAR_SUPPORT_VARUSE 128				// varuse関数のチェックを有効にする
#define HSPVAR_SUPPORT_TEMPVAR 256				// テンポラリ変数として使用する
#define HSPVAR_SUPPORT_USER1 0x4000				// ユーザーフラグ1
#define HSPVAR_SUPPORT_USER2 0x8000				// ユーザーフラグ2

#define HSPVAR_SUPPORT_MISCTYPE (HSPVAR_SUPPORT_ARRAYOBJ)

typedef void * PDAT;							// データの実態へのポインタ
typedef int APTR;								// 配列データへのオフセット値

enum
{
CALCCODE_ADD = 0,
CALCCODE_SUB,
CALCCODE_MUL,
CALCCODE_DIV,
CALCCODE_MOD,
CALCCODE_AND,
CALCCODE_OR,
CALCCODE_XOR,
CALCCODE_EQ,
CALCCODE_NE,
CALCCODE_GT,
CALCCODE_LT,
CALCCODE_GTEQ,
CALCCODE_LTEQ,
CALCCODE_RR,
CALCCODE_LR,
CALCCODE_MAX
};

//	PVAL structure
//
typedef struct
{
	//	Memory Data structure (2.5 compatible)
	//
	short	flag;		// type of val
	short	mode;		// mode (0=normal/1=clone/2=alloced)
	int		len[5];		// length of array 4byte align (dim)
	int		size;		// size of Val
	char	*pt;		// ptr to array

	//	Memory Data structure (3.0 compatible)
	//
	void	*master;			// Master pointer for data
	unsigned short	support;	// Support Flag
	short	arraycnt;			// Array Set Count
	int		offset;				// Array Data Offset
	int		arraymul;			// Array Multiple Value 

} PVal;


//		command execute core function
//
typedef struct
{
	//		データフィールド
	//
	short flag;							// 型タイプ値 (親アプリケーションで設定されます)
	short aftertype;					// 演算後のタイプ値
	short version;						// 型タイプランタイムバージョン(0x100 = 1.0)
	unsigned short support;				// サポート状況フラグ(HSPVAR_SUPPORT_*)
	short basesize;						// １つのデータが使用するサイズ(byte) / 可変長の時は-1
	short opt;							// (未使用)

	char *vartype_name;					// 型タイプ名文字列へのポインタ
	char *user;							// ユーザーデータ(未使用)

	//		システム参照・型変換用
	//
	void *(*Cnv)( const void *buffer, int flag );
	void *(*CnvCustom)( const void *buffer, int flag );
	PDAT *(*GetPtr)( PVal *pval );

	void *(*ArrayObjectRead)( PVal *pval, int *mptype );// 配列要素の指定 (連想配列/読み出し)
	void (*ArrayObject)( PVal *pval );							// 配列要素の指定 (連想配列/書き込み準備)
	void (*ObjectWrite)( PVal *pval, void *data, int type );		// HSPVAR_SUPPORT_NOCONVERT指定時の代入
	void (*ObjectMethod)( PVal *pval );							// 変数に対するメソッドの指定

	void (*Alloc)( PVal *pval, const PVal *pval2 );		// 変数メモリを確保する
	void (*Free)( PVal *pval );						// 変数メモリを解放する

	int (*GetSize)( const PDAT *pdat );			// 要素が使用するメモリサイズを返す(可変長のため)
	int (*GetUsing)( const PDAT *pdat );			// 要素が使用中であるかを返す(varuse関数用)

	//		変数バッファ(バイナリ)のポインタとサイズを返す
	//		(要素が可変長(str)の場合は該当する１配列バイナリのみ)
	//		(要素が固定長(int,double)の場合は全配列バイナリ)
	//		(サイズはメモリ確保サイズを返す)
	void *(*GetBlockSize)( PVal *pval, PDAT *pdat, int *size );

	//		バイナリデータ用にメモリブロックを確保する
	//		(要素が可変長(str)の場合にブロックサイズを強制的に確保する)
	//		(固定長の場合は何もしない)
	void (*AllocBlock)( PVal *pval, PDAT *pdat, int size );

	//		代入用関数(型の一致が保障されます)
	//
	void (*Set)( PVal *pval, PDAT *pdat, const void *in );

	//		演算用関数(型の一致が保障されます)
	//
	void (*AddI)( PDAT *pval, const void *val );
	void (*SubI)( PDAT *pval, const void *val );
	void (*MulI)( PDAT *pval, const void *val );
	void (*DivI)( PDAT *pval, const void *val );
	void (*ModI)( PDAT *pval, const void *val );

	void (*AndI)( PDAT *pval, const void *val );
	void (*OrI)( PDAT *pval, const void *val );
	void (*XorI)( PDAT *pval, const void *val );

	void (*EqI)( PDAT *pval, const void *val );
	void (*NeI)( PDAT *pval, const void *val );
	void (*GtI)( PDAT *pval, const void *val );
	void (*LtI)( PDAT *pval, const void *val );
	void (*GtEqI)( PDAT *pval, const void *val );
	void (*LtEqI)( PDAT *pval, const void *val );

	void (*RrI)( PDAT *pval, const void *val );
	void (*LrI)( PDAT *pval, const void *val );
} HspVarProc;

extern HspVarProc *hspvarproc;
extern int hspvartype_max;

typedef void (* HSPVAR_COREFUNC) (HspVarProc *);

extern PVal *mem_pval;
#define HspVarCoreGetPVal( flag ) (&mem_pval[flag])


//	flex value define
//
#define FLEXVAL_TYPE_NONE 0
#define FLEXVAL_TYPE_ALLOC 1
#define FLEXVAL_TYPE_CLONE 2
typedef struct
{
	short type;			// typeID
	short myid;			// 固有ID(未使用)
	short customid;		// structure ID
	short clonetype;	// typeID for clone
	int size;			// data size
	void *ptr;			// data ptr
} FlexValue;


/*
	typefunc

	基本タイプ HSPVAR_FLAG_STR ～ HSPVAR_FLAG_DOUBLE
	拡張タイプ HSPVAR_FLAG_USERDEF 以降

	式の評価でpval->ptを参照するため、常に配列0のポイントはpval->ptが指し示す必要がある。
*/


//		Core System Main Function
//
void HspVarCoreInit( void );
void HspVarCoreBye( void );
void HspVarCoreResetVartype( int expand );
int HspVarCoreAddType();
void HspVarCoreRegisterType( int flag, HSPVAR_COREFUNC func );
HspVarProc *HspVarCoreSeekProc( const char *name );

//		low level support functions
//
void HspVarCoreDup( PVal *pval, PVal *arg, APTR aptr );
void HspVarCoreDupPtr( PVal *pval, int flag, void *ptr, int size );
void HspVarCoreClear( PVal *pval, int flag );
void HspVarCoreClearTemp( PVal *pval, int flag );
void HspVarCoreDim( PVal *pval, int flag, int len1, int len2, int len3, int len4 );
void HspVarCoreDimFlex( PVal *pval, int flag, int len0, int len1, int len2, int len3, int len4 );
void HspVarCoreReDim( PVal *pval, int lenid, int len );
void *HspVarCoreCnvPtr( PVal *pval, int flag );
PDAT *HspVarCorePtrAPTR( PVal *pv, APTR ofs );
void HspVarCoreArray( PVal *pval, int offset );
int  HspVarCoreCountElems( PVal *pval );
void HspVarCoreAllocPODArray( PVal *pval, const PVal *pval2, int basesize );

//		macro for PVal
//
#define HspVarCoreGetProc( flag ) (&hspvarproc[flag])
#define HspVarCoreDispose( pv ) hspvarproc[(pv)->flag].Free(pv)
#define HspVarCoreReset( pv ) ((pv)->offset=0,(pv)->arraycnt=0)
#define HspVarCorePtr( pv ) (hspvarproc[(pv)->flag].GetPtr(pv))
#define HspVarCoreArrayObject( pv,in ) (hspvarproc[(pv)->flag].ArrayObject(pv,in))	//	配列の要素を指定する(最初にResetを呼んでおくこと)

#define HspVarCoreSet( pv,in ) hspvarproc[(pv)->flag].Set( pv, in )
#define HspVarCoreCnv( in1,in2,in3 ) hspvarproc[in2].Cnv( in3,in1 )		// in1->in2の型にin3ポインタを変換する

#define HspVarCoreGetBlockSize( pv,in1,out ) hspvarproc[(pv)->flag].GetBlockSize( pv,in1,out )
#define HspVarCoreAllocBlock( pv,in1,in2 ) hspvarproc[(pv)->flag].AllocBlock( pv,in1,in2 )
#define HspVarCoreGetUsing( pv,in1 ) hspvarproc[(pv)->flag].GetUsing( in1 )

#define HspVarCoreGetAPTR( pv ) ((pv)->offset)

#define HspVarCoreCopyArrayInfo( pv, src ) (pv)->arraycnt = (src)->arraycnt;(pv)->offset = (src)->offset;(pv)->arraymul = (src)->arraymul;


inline PDAT *HspVarCorePtrAPTR( PVal *pv, APTR ofs )
{
	//		変数データの実態ポインタを得る
	//		(APTRとpvalから実態を求める)
	//
	pv->offset=ofs;
	return hspvarproc[(pv)->flag].GetPtr(pv);
}




#endif
