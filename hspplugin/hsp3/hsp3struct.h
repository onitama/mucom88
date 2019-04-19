
//
//		Structure for HSP
//
#ifndef __hsp3struct_h
#define __hsp3struct_h

#include "hspvar_core.h"

/*
	rev 43
	mingw : error : HSPERROR が未定義
	に対処
*/
#include "hsp3debug.h"

#ifdef _WIN64
#define PTR64BIT        //  ポインタは64bit
#else
#define PTR32BIT        //  ポインタは32bit
#endif

// command type
#define TYPE_MARK 0
#define TYPE_VAR 1
#define TYPE_STRING 2
#define TYPE_DNUM 3
#define TYPE_INUM 4
#define TYPE_STRUCT 5
#define TYPE_XLABEL 6
#define TYPE_LABEL 7
#define TYPE_INTCMD 8
#define TYPE_EXTCMD 9
#define TYPE_EXTSYSVAR 10
#define TYPE_CMPCMD 11
#define TYPE_MODCMD 12
#define TYPE_INTFUNC 13
#define TYPE_SYSVAR 14
#define TYPE_PROGCMD 15
#define TYPE_DLLFUNC 16
#define TYPE_DLLCTRL 17
#define TYPE_USERDEF 18

#define TYPE_ERROR -1
#define TYPE_CALCERROR -2

#define PARAM_OK 0
#define PARAM_SPLIT -1
#define PARAM_END -2
#define PARAM_DEFAULT -3
#define PARAM_ENDSPLIT -4

#define HSP3_FUNC_MAX 18
#define HSP3_TYPE_USER 18

#define EXFLG_0 0x1000
#define EXFLG_1 0x2000
#define EXFLG_2 0x4000
#define EXFLG_3 0x8000
#define CSTYPE 0x0fff

typedef struct HSPHED
{
	//		HSP3.0 header structure
	//

	char	h1;					// magic code1
	char	h2;					// magic code2
	char	h3;					// magic code3
	char	h4;					// magic code4
	int		version;			// version number info
	int		max_val;			// max count of VAL Object
	int		allsize;			// total file size

	int		pt_cs;				// ptr to Code Segment
	int		max_cs;				// size of CS
	int		pt_ds;				// ptr to Data Segment
	int		max_ds;				// size of DS

	int		pt_ot;				// ptr to Object Temp
	int		max_ot;				// size of OT
	int		pt_dinfo;			// ptr to Debug Info
	int		max_dinfo;			// size of DI

	int		pt_linfo;			// ptr to LibInfo(2.3)
	int		max_linfo;			// size of LibInfo(2.3)
	int		pt_finfo;			// ptr to FuncInfo(2.3)
	int		max_finfo;			// size of FuncInfo(2.3)

	int		pt_minfo;			// ptr to ModInfo(2.5)
	int		max_minfo;			// size of ModInfo(2.5)
	int		pt_finfo2;			// ptr to FuncInfo2(2.5)
	int		max_finfo2;			// size of FuncInfo2(2.5)

	int		pt_hpidat;			// ptr to HPIDAT(3.0)
	short		max_hpi;			// size of HPIDAT(3.0)
	short		max_varhpi;			// Num of Vartype Plugins(3.0)
	int		bootoption;			// bootup options
	int		runtime;			// ptr to runtime name

	//		HSP3.5 extra header structure
	//
	int		pt_sr;				// ptr to Option Segment
	int		max_sr;				// size of Option Segment
	int		opt1;				// option (reserved)
	int		opt2;				// option (reserved)

} HSPHED;

#define HSPHED_BOOTOPT_DEBUGWIN 1			// 起動時デバッグウインドゥ表示
#define HSPHED_BOOTOPT_WINHIDE 2			// 起動時ウインドゥ非表示
#define HSPHED_BOOTOPT_DIRSAVE 4			// 起動時カレントディレクトリ変更なし
#define HSPHED_BOOTOPT_SAVER 0x100			// スクリーンセーバー

#define HSPHED_BOOTOPT_RUNTIME 0x1000		// 動的ランタイムを有効にする
#define HSPHED_BOOTOPT_NOMMTIMER 0x2000		// マルチメディアタイマーを無効にする
#define HSPHED_BOOTOPT_NOGDIP 0x4000		// GDI+による描画を無効にする
#define HSPHED_BOOTOPT_FLOAT32 0x8000		// 実数を32bit floatとして処理する
#define HSPHED_BOOTOPT_ORGRND 0x10000		// 標準の乱数発生を使用する
#define HSPHED_BOOTOPT_UTF8 0x20000			// UTF8ランタイムを使用する(コード識別用)
#define HSPHED_BOOTOPT_HSP64 0x40000		// 64bitランタイムを使用する(コード識別用)
#define HSPHED_BOOTOPT_IORESUME 0x80000		// ファイルI/Oエラーを無視して処理を続行する


#define HPIDAT_FLAG_TYPEFUNC 0
#define HPIDAT_FLAG_SELFFUNC -1
#define HPIDAT_FLAG_VARFUNC 1
#define HPIDAT_FLAG_DLLFUNC 2

typedef struct MEM_HPIDAT {		// native HPIDAT

	short	flag;				// flag info
	short	option;
	int		libname;			// lib name index (DS)
	int		funcname;			// function name index (DS)
	void	*libptr;			// lib handle

} MEM_HPIDAT;

#ifdef PTR64BIT
typedef struct HPIDAT {

	short	flag;				// flag info
	short	option;
	int		libname;			// lib name index (DS)
	int		funcname;			// function name index (DS)
	int		p_libptr;			// lib handle

} HPIDAT;
#else
typedef MEM_HPIDAT HPIDAT;
#endif


#define LIBDAT_FLAG_NONE 0
#define LIBDAT_FLAG_DLL 1
#define LIBDAT_FLAG_DLLINIT 2
#define LIBDAT_FLAG_MODULE 3
#define LIBDAT_FLAG_COMOBJ 4

typedef struct LIBDAT {

	int		flag;				// initalize flag
	int		nameidx;			// function name index (DS)
								// Interface IID ( Com Object )
	void	*hlib;				// Lib handle
	int		clsid;				// CLSID (DS) ( Com Object )

} LIBDAT;

#ifdef PTR64BIT
typedef struct HED_LIBDAT {

	int		flag;				// initalize flag
	int		nameidx;			// function name index (DS)
								// Interface IID ( Com Object )
	int		p_hlib;				// Lib handle
	int		clsid;				// CLSID (DS) ( Com Object )

} HED_LIBDAT;
#else
typedef LIBDAT HED_LIBDAT;
#endif


// multi parameter type
#define MPTYPE_NONE 0
#define MPTYPE_VAR 1
#define MPTYPE_STRING 2
#define MPTYPE_DNUM 3
#define MPTYPE_INUM 4
#define MPTYPE_STRUCT 5
#define MPTYPE_LABEL 7

#define MPTYPE_LOCALVAR -1
#define MPTYPE_ARRAYVAR -2
#define MPTYPE_SINGLEVAR -3
#define MPTYPE_FLOAT -4
#define MPTYPE_STRUCTTAG -5
#define MPTYPE_LOCALSTRING -6
#define MPTYPE_MODULEVAR -7
#define MPTYPE_PPVAL -8
#define MPTYPE_PBMSCR -9
#define MPTYPE_PVARPTR -10
#define MPTYPE_IMODULEVAR -11

#define MPTYPE_IOBJECTVAR -12
#define MPTYPE_LOCALWSTR -13
#define MPTYPE_FLEXSPTR -14
#define MPTYPE_FLEXWPTR -15
#define MPTYPE_PTR_REFSTR -16
#define MPTYPE_PTR_EXINFO -17
#define MPTYPE_PTR_DPMINFO -18
#define MPTYPE_NULLPTR -19
#define MPTYPE_TMODULEVAR -20

//#define MPTYPE_PTR_HWND -14
//#define MPTYPE_PTR_HDC -15
//#define MPTYPE_PTR_HINST -16

#define STRUCTPRM_SUBID_STACK -1
#define STRUCTPRM_SUBID_STID -2
#define STRUCTPRM_SUBID_DLL -3
#define STRUCTPRM_SUBID_DLLINIT -4
#define STRUCTPRM_SUBID_OLDDLL -5
#define STRUCTPRM_SUBID_OLDDLLINIT -6
#define STRUCTPRM_SUBID_COMOBJ -7

#define STRUCTCODE_THISMOD -1

#define TYPE_OFFSET_COMOBJ 0x1000

typedef struct STRUCTPRM {
	short	mptype;				// Parameter type
	short	subid;				// struct index
	int		offset;				// offset from top
} STRUCTPRM;

//	DLL function flags
#define STRUCTDAT_OT_NONE 0
#define STRUCTDAT_OT_CLEANUP 1
#define STRUCTDAT_OT_STATEMENT 2
#define STRUCTDAT_OT_FUNCTION 4

//	Module function flags
#define STRUCTDAT_INDEX_FUNC -1
#define STRUCTDAT_INDEX_CFUNC -2
#define STRUCTDAT_INDEX_STRUCT -3
#define STRUCTDAT_FUNCFLAG_CLEANUP 0x10000

// function,module specific data

#ifdef PTR64BIT
typedef struct STRUCTDAT {
	short	index;				// base LIBDAT index
	short	subid;				// struct index
	int		prmindex;			// STRUCTPRM index(MINFO)
	int		prmmax;				// number of STRUCTPRM
	int		nameidx;			// name index (DS)
	int		size;				// struct size (stack)
	int		otindex;			// OT index(Module) / cleanup flag(Dll)
	void	*proc;				// proc address
	int		funcflag;			// function flags(Module)
} STRUCTDAT;

typedef struct HED_STRUCTDAT {
	short	index;				// base LIBDAT index
	short	subid;				// struct index
	int		prmindex;			// STRUCTPRM index(MINFO)
	int		prmmax;				// number of STRUCTPRM
	int		nameidx;			// name index (DS)
	int		size;				// struct size (stack)
	int		otindex;			// OT index(Module) / cleanup flag(Dll)
	int		funcflag;			// function flags(Module)
} HED_STRUCTDAT;

#else
typedef struct STRUCTDAT {
	short	index;				// base LIBDAT index
	short	subid;				// struct index
	int		prmindex;			// STRUCTPRM index(MINFO)
	int		prmmax;				// number of STRUCTPRM
	int		nameidx;			// name index (DS)
	int		size;				// struct size (stack)
	int		otindex;			// OT index(Module) / cleanup flag(Dll)
	union {
		void	*proc;				// proc address
		int		funcflag;			// function flags(Module)
	};
} STRUCTDAT;
typedef STRUCTDAT HED_STRUCTDAT;
#endif


//	Var Data for Multi Parameter
typedef struct MPVarData {
	PVal	*pval;
	APTR	aptr;
} MPVarData;

//	Var Data for Module Function
typedef struct MPModVarData {
	short subid;
	short magic;
	PVal	*pval;
	APTR	aptr;
} MPModVarData;
#define MODVAR_MAGICCODE 0x55aa

#define IRQ_FLAG_NONE 0
#define IRQ_FLAG_DISABLE 1
#define IRQ_FLAG_ENABLE 2
#define IRQ_OPT_GOTO 0
#define IRQ_OPT_GOSUB 1
#define IRQ_OPT_CALLBACK 2


typedef struct IRQDAT {
	short	flag;								// flag
	short	opt;								// option value
	int		custom;								// custom message value
	int		custom2;							// custom message value2
	int		iparam;								// iparam option
	unsigned short *ptr;						// jump ptr
	void	(*callback)(struct IRQDAT *,int,int);		// IRQ callback function
} IRQDAT;

typedef struct HSPCTX HSPCTX;

//	Plugin info data (3.0 compatible)
typedef struct HSPEXINFO30
{
	//		HSP internal info data (2.6)
	//
	short ver;		// Version Code
	short min;		// Minor Version
	//
	int *er;		// Not Use
	char *pstr;		// String Buffer (master)
	char *stmp;		// String Buffer (sub)
	PVal **mpval;		// Master PVAL
	//
	int *actscr;		// Active Window ID
	int *nptype;		// Next Parameter Type
	int *npval;			// Next Parameter Value
	int *strsize;		// StrSize Buffer
	char *refstr;		// RefStr Buffer
	//
	void *(*HspFunc_prm_getv)( void );
	int (*HspFunc_prm_geti)( void );
	int (*HspFunc_prm_getdi)( const int defval );
	char *(*HspFunc_prm_gets)( void );
	char *(*HspFunc_prm_getds)( const char *defstr );
	int (*HspFunc_val_realloc)( PVal *pv, int size, int mode );
	int (*HspFunc_fread)( char *fname, void *readmem, int rlen, int seekofs );
	int (*HspFunc_fsize)( char *fname );
	void *(*HspFunc_getbmscr)( int wid );
	int (*HspFunc_getobj)( int wid, int id, void *inf );
	int (*HspFunc_setobj)( int wid, int id, const void *inf );

	//		HSP internal info data (3.0)
	//
	int *npexflg;	// Next Parameter ExFlg
	HSPCTX *hspctx;	// HSP context ptr

	//		Enhanced data (3.0)
	//
	int (*HspFunc_addobj)( int wid );
	void (*HspFunc_puterror)( HSPERROR error );
	HspVarProc *(*HspFunc_getproc)( int type );
	HspVarProc *(*HspFunc_seekproc)( const char *name );

	void (*HspFunc_prm_next)( void );
	int (*HspFunc_prm_get)( void );
	double (*HspFunc_prm_getd)( void );
	double (*HspFunc_prm_getdd)( double defval );
	unsigned short *(*HspFunc_prm_getlb)( void );
	PVal *(*HspFunc_prm_getpval)( void );
	APTR (*HspFunc_prm_getva)( PVal **pval );
	void (*HspFunc_prm_setva)( PVal *pval, APTR aptr, int type, const void *ptr );
	char *(*HspFunc_malloc)( int size );
	void (*HspFunc_free)( void *ptr );
	char *(*HspFunc_expand)( char *ptr, int size );
	IRQDAT *(*HspFunc_addirq)( void );
	int (*HspFunc_hspevent)( int event, int prm1, int prm2, void *prm3 );
	void (*HspFunc_registvar)( int flag, HSPVAR_COREFUNC func );
	void (*HspFunc_setpc)( const unsigned short *pc );
	void (*HspFunc_call)( const unsigned short *pc );
	void (*HspFunc_mref)( PVal *pval, int prm );

	void (*HspFunc_dim)( PVal *pval, int flag, int len0, int len1, int len2, int len3, int len4 );
	void (*HspFunc_redim)( PVal *pval, int lenid, int len );
	void (*HspFunc_array)( PVal *pval, int offset );

} HSPEXINFO30;



//	Plugin info data (3.1 or later)
typedef struct HSPEXINFO
{
	//		HSP internal info data (2.6)
	//
	short ver;		// Version Code
	short min;		// Minor Version
	//
	int *er;		// Not Use
	char *pstr;		// String Buffer (master)
	char *stmp;		// String Buffer (sub)
	PVal **mpval;		// Master PVAL
	//
	int *actscr;		// Active Window ID
	int *nptype;		// Next Parameter Type
	int *npval;			// Next Parameter Value
	int *strsize;		// StrSize Buffer
	char *refstr;		// RefStr Buffer
	//
	void *(*HspFunc_prm_getv)( void );
	int (*HspFunc_prm_geti)( void );
	int (*HspFunc_prm_getdi)( const int defval );
	char *(*HspFunc_prm_gets)( void );
	char *(*HspFunc_prm_getds)( const char *defstr );
	int (*HspFunc_val_realloc)( PVal *pv, int size, int mode );
	int (*HspFunc_fread)( char *fname, void *readmem, int rlen, int seekofs );
	int (*HspFunc_fsize)( char *fname );
	void *(*HspFunc_getbmscr)( int wid );
	int (*HspFunc_getobj)( int wid, int id, void *inf );
	int (*HspFunc_setobj)( int wid, int id, const void *inf );

	//		HSP internal info data (3.0)
	//
	int *npexflg;	// Next Parameter ExFlg
	HSPCTX *hspctx;	// HSP context ptr

	//		Enhanced data (3.0)
	//
	int (*HspFunc_addobj)( int wid );
	void (*HspFunc_puterror)( HSPERROR error );
	HspVarProc *(*HspFunc_getproc)( int type );
	HspVarProc *(*HspFunc_seekproc)( const char *name );

	void (*HspFunc_prm_next)( void );
	int (*HspFunc_prm_get)( void );
	double (*HspFunc_prm_getd)( void );
	double (*HspFunc_prm_getdd)( double defval );
	unsigned short *(*HspFunc_prm_getlb)( void );
	PVal *(*HspFunc_prm_getpval)( void );
	APTR (*HspFunc_prm_getva)( PVal **pval );
	void (*HspFunc_prm_setva)( PVal *pval, APTR aptr, int type, const void *ptr );
	char *(*HspFunc_malloc)( int size );
	void (*HspFunc_free)( void *ptr );
	char *(*HspFunc_expand)( char *ptr, int size );
	IRQDAT *(*HspFunc_addirq)( void );
	int (*HspFunc_hspevent)( int event, int prm1, int prm2, void *prm3 );
	void (*HspFunc_registvar)( int flag, HSPVAR_COREFUNC func );
	void (*HspFunc_setpc)( const unsigned short *pc );
	void (*HspFunc_call)( const unsigned short *pc );
	void (*HspFunc_mref)( PVal *pval, int prm );

	void (*HspFunc_dim)( PVal *pval, int flag, int len0, int len1, int len2, int len3, int len4 );
	void (*HspFunc_redim)( PVal *pval, int lenid, int len );
	void (*HspFunc_array)( PVal *pval, int offset );

	//		Enhanced data (3.1)
	//
	char *(*HspFunc_varname)( int id );
	int (*HspFunc_seekvar)( const char *name );

	//		Enhanced data (3.5)
	//
	char *(*HspFunc_prm_getns)(void);
	char *(*HspFunc_prm_getnds)(const char *defstr);

} HSPEXINFO;


#define HSP3_REPEAT_MAX 32
typedef struct LOOPDAT {
	int		time;				// loop times left
	int		cnt;				// count
	int		step;				// count add value
	unsigned short *pt;			// loop start ptr
} LOOPDAT;


// 実行モード
enum
{
RUNMODE_RUN = 0,
RUNMODE_WAIT,
RUNMODE_AWAIT,
RUNMODE_STOP,
RUNMODE_END,
RUNMODE_ERROR,
RUNMODE_RETURN,
RUNMODE_INTJUMP,
RUNMODE_ASSERT,
RUNMODE_LOGMES,
RUNMODE_EXITRUN,
RUNMODE_MAX
};


struct HSPCTX
{
	//	HSP Context
	//
	HSPHED *hsphed;						// HSP object file header
	unsigned short *mcs;				// current code segment ptr
	unsigned short *mem_mcs;			// code segment ptr
	char *mem_mds;						// data segment ptr
	unsigned char *mem_di;				// Debug info ptr
	int *mem_ot;						// object temp segment ptr

	IRQDAT *mem_irq;					// IRQ data ptr
	int irqmax;							// IRQ data count
	int iparam;							// IRQ Info data1
	int wparam;							// IRQ Info data2
	int lparam;							// IRQ Info data3

	PVal *mem_var;						// var storage index
	HSPEXINFO30 exinfo;					// HSP function data(3.0)
	int runmode;						// HSP execute mode
	int waitcount;						// counter for wait
	int waitbase;						// wait sleep base
	int waittick;						// next tick for await
	int lasttick;						// previous tick
	int sublev;							// subroutine level
	LOOPDAT mem_loop[HSP3_REPEAT_MAX];	// repeat loop info
	int looplev;						// repeat loop level
	HSPERROR err;						// error code
	int hspstat;						// HSP status
	int stat;							// sysvar 'stat'
	int strsize;						// sysvar 'strsize'
	char *refstr;						// RefStr Buffer
	char *fnbuffer;						// buffer for FILENAME
	void *instance;						// Instance Handle (windows)
	int intwnd_id;						// Window ID (interrupt)
	PVal *note_pval;					// MemNote pval
	APTR note_aptr;						// MemNote aptr
	PVal *notep_pval;					// MemNote pval (previous)
	APTR notep_aptr;					// MemNote aptr (previous)
	char *stmp;							// String temporary buffer

	void *prmstack;						// Current parameter stack area
	LIBDAT *mem_linfo;					// Library info
	STRUCTPRM *mem_minfo;				// Parameter info
	STRUCTDAT *mem_finfo;				// Function/Struct info
	int retval_level;					// subroutine level (return code)
	int endcode;						// End result code
	void (*msgfunc) (HSPCTX *);			// Message Callback Proc.
	void *wnd_parent;					// Parent Window Handle
	double refdval;						// sysvar 'refdval'
	char *cmdline;						// Command Line Parameters

	HSPEXINFO *exinfo2;					// HSP function data(3.1)

	int	prmstack_max;					// Parameter Stack Max(hsp3cnv) (3.3)
};

#define HSPCTX_REFSTR_MAX 4096
#define HSPCTX_CMDLINE_MAX 1024

#define HSPSTAT_NORMAL 0
#define HSPSTAT_DEBUG 1
#define HSPSTAT_SSAVER 2
#define HSPSTAT_CONSOLE 0x10
#define HSPSTAT_MAC 0x80
#define HSPSTAT_DISH 0x100
#define HSPSTAT_LINUX 0x1000
#define HSPSTAT_UTF8 0x20000
#define HSPSTAT_HSP64 0x40000

#define TYPE_EX_SUBROUTINE 0x100		// gosub用のスタックタイプ
#define TYPE_EX_CUSTOMFUNC 0x101		// deffunc呼び出し用のスタックタイプ
#define TYPE_EX_ENDOFPARAM 0x200		// パラメーター終端(HSPtoC)
#define TYPE_EX_ARRAY_VARS 0x201		// 配列要素付き変数用スタックタイプ(HSPtoC)
#define TYPE_EX_LOCAL_VARS 0x202		// ローカル変数用スタックタイプ(HSPtoC)

typedef struct
{
	//	Subroutine Context
	//
	int stacklev;						// サブルーチン開始時のスタックレベル
	unsigned short *mcsret;				// 呼び出し元PCポインタ(復帰用)
	STRUCTDAT *param;					// 引数パラメーターリスト
	void *oldtack;						// 以前のスタックアドレス
	int oldlev;							// 以前のスタックレベル

} HSPROUTINE;



//		コールバックのオプション
//
#define HSPEVENT_ENABLE_COMMAND 1	// １ステップ実行時
#define HSPEVENT_ENABLE_HSPIRQ 2	// HSP内での割り込み発生時
#define HSPEVENT_ENABLE_GETKEY 4	// キーチェック時
#define HSPEVENT_ENABLE_FILE 8		// ファイル入出力時
#define HSPEVENT_ENABLE_MEDIA 16	// メディア入出力時
#define HSPEVENT_ENABLE_PICLOAD 32	// picload命令実行時


//		ファンクション型
//
typedef int (* HSP3_CMDFUNC) (int);
typedef void *(* HSP3_REFFUNC) (int *,int);
typedef int (* HSP3_TERMFUNC) (int);
typedef int (* HSP3_MSGFUNC) (int,int,int);
typedef int (* HSP3_EVENTFUNC) (int,int,int,void *);


typedef struct {
	//	型ごとの情報
	//	(*の項目は、親アプリケーションで設定されます)
	//
	short type;							// *型タイプ値
	short option;						// *オプション情報
	HSPCTX *hspctx;						// *HSP Context構造体へのポインタ
	HSPEXINFO *hspexinfo;				// *HSPEXINFO構造体へのポインタ

	//	ファンクション情報
	//
	int (* cmdfunc) (int);				// コマンド受け取りファンクション
	void *(* reffunc) (int *,int);		// 参照受け取りファンクション
	int (* termfunc) (int);				// 終了受け取りファンクション

	// イベントコールバックファンクション
	//
	int (* msgfunc) (int,int,int);				// Windowメッセージコールバック
	int (* eventfunc) (int,int,int,void *);		// HSPイベントコールバック

} HSP3TYPEINFO;


// HSP割り込みID
enum
{
HSPIRQ_ONEXIT = 0,
HSPIRQ_ONERROR,
HSPIRQ_ONKEY,
HSPIRQ_ONCLICK,
HSPIRQ_USERDEF,
HSPIRQ_MAX
};

// HSPイベントID
enum
{
HSPEVENT_NONE = 0,
HSPEVENT_COMMAND,
HSPEVENT_HSPIRQ,
HSPEVENT_GETKEY,
HSPEVENT_STICK,
HSPEVENT_FNAME,
HSPEVENT_FREAD,
HSPEVENT_FWRITE,
HSPEVENT_FEXIST,
HSPEVENT_FDELETE,
HSPEVENT_FMKDIR,
HSPEVENT_FCHDIR,
HSPEVENT_FCOPY,
HSPEVENT_FDIRLIST1,
HSPEVENT_FDIRLIST2,
HSPEVENT_GETPICSIZE,
HSPEVENT_PICLOAD,
HSPEVENT_MAX
};

// ginfo拡張用フィーメド
#define GINFO_EXINFO_MAX 16
#define GINFO_EXINFO_BASE 0x100
#define GINFO_EXINFO_ACCEL_X (GINFO_EXINFO_BASE+0)
#define GINFO_EXINFO_ACCEL_Y (GINFO_EXINFO_BASE+1)
#define GINFO_EXINFO_ACCEL_Z (GINFO_EXINFO_BASE+2)
#define GINFO_EXINFO_GYRO_X (GINFO_EXINFO_BASE+3)
#define GINFO_EXINFO_GYRO_Y (GINFO_EXINFO_BASE+4)
#define GINFO_EXINFO_GYRO_Z (GINFO_EXINFO_BASE+5)


#endif
