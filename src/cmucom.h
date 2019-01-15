
//
//	cmucom.cpp structures
//
#ifndef __CMucom_h
#define __CMucom_h

#include "membuf.h"

/*------------------------------------------------------------*/

#define MUCOM_DEFAULT_PCMFILE "mucompcm.bin"
#define MUCOM_DEFAULT_VOICEFILE "voice.dat"

#define MUCOM_CH_FM1 0
#define MUCOM_CH_PSG 3
#define MUCOM_CH_FM2 7
#define MUCOM_CH_RHYTHM 6
#define MUCOM_CH_ADPCM 10
#define MUCOM_MAXCH 11				// 最大ch(固定)

#define MUCOM_CHDATA_SIZE 64		// chデータのサイズ
#define MUCOM_LINE_MAXSTR 512		// 1行あたりの最大文字数
#define MUCOM_FILE_MAXSTR 512		// ファイル名の最大文字数

#define MUCOM_DATA_ADDRESS 0xc200

#ifndef USE_SDL
#define MUCOM_AUDIO_RATE 55467		// Sampling Rate 55K
#else
#define MUCOM_AUDIO_RATE 44100
#endif

#define MUCOM_RESET_PLAYER 0
#define MUCOM_RESET_EXTFILE 1
#define MUCOM_RESET_COMPILE 2

#define MUCOM_COMPILE_NORMAL 0

#define MUCOM_MUSICBUFFER_MAX 16

#define MUCOM_STATUS_PLAYING 0
#define MUCOM_STATUS_INTCOUNT 1
#define MUCOM_STATUS_PASSTICK 2
#define MUCOM_STATUS_MAJORVER 3
#define MUCOM_STATUS_MINORVER 4
#define MUCOM_STATUS_COUNT 5
#define MUCOM_STATUS_MAXCOUNT 6
#define MUCOM_STATUS_MUBSIZE 7
#define MUCOM_STATUS_MUBRATE 8
#define MUCOM_STATUS_BASICSIZE 9
#define MUCOM_STATUS_BASICRATE 10

#define MUCOM_OPTION_FMMUTE 1
#define MUCOM_OPTION_SCCI 2
#define MUCOM_OPTION_FASTFW 4
#define MUCOM_OPTION_STEP 8

#define MUCOM_MUBSIZE_MAX (0xE300-0xC200)
#define MUCOM_BASICSIZE_MAX 0x6000

#define MUCOM_HEADER_VERSION1 1		// 1.0 Header
#define MUCOM_HEADER_VERSION2 2		// 2.0 Header

#define MUCOM_FLAG_UTF8TAG 1		// TAG dataの文字コードはUTF8

//	MUBのバイナリデータ生成に関する情報
#define MUCOM_SYSTEM_UNKNOWN 0		// 不明なシステムによる生成
#define MUCOM_SYSTEM_PC88 1			// PC88互換のシステムによる生成
#define MUCOM_SYSTEM_PC88UC 2		// PC88と上位互換のシステムによる生成
#define MUCOM_SYSTEM_NATIVE 3		// ネイティブなシステムによる生成

//	MUBが演奏時に想定するシステムの情報
#define MUCOM_TARGET_UNKNOWN 0		// 未指定
#define MUCOM_TARGET_YM2203 1		// YM2203による演奏
#define MUCOM_TARGET_YM2608 2		// YM2608による演奏
#define MUCOM_TARGET_YM2151 3		// YM2151による演奏
#define MUCOM_TARGET_MULTI 0x80		// 複数のチップによる演奏

#define MUCOM_FMVOICE_MAX 16

#define MUCOM_CMPOPT_USE_EXTROM 1
#define MUCOM_CMPOPT_COMPILE 2
#define MUCOM_CMPOPT_STEP 8
#define MUCOM_CMPOPT_INFO 0x100


//	MUBHED structure
//
typedef struct
{
	//	Memory Data structure
	//
	char magic[4];		// magic number
	int dataoffset;		// DATA offset
	int datasize;		// DATA size
	int tagdata;		// TAG data file offset (0=none)
	int tagsize;		// TAG data size
	int pcmdata;		// PCM data offset (0=none)
	int pcmsize;		// PCM data size
	short jumpcount;	// Jump count (for skip)
	short jumpline;		// Jump line number (for skip)

	//	Extend data (2.0 Header)
	//
	short ext_flags;		// Option flags ( MUCOM_FLAG_* )
	char ext_system;		// build system ( MUCOM_SYSTEM_* )
	char ext_target;		// playback target ( MUCOM_TARGET_* )
	short ext_channel_num;	// Max channel table num
	short ext_fmvoice_num;	// internal FM voice table num

	int ext_player;			// external player option
	int pad1;				// not use (reserved)
	unsigned char ext_fmvoice[MUCOM_FMVOICE_MAX];	// FM voice no.(orginal) table
} MUBHED;

//	PCHDATA structure
//
#define MUCOM_PCHDATA_PC88_SIZE 38

typedef struct
{
	//	Player Channel Data structure
	//
	int length;				// LENGTH ｶｳﾝﾀｰ		IX + 0
	int vnum;				// ｵﾝｼｮｸ ﾅﾝﾊﾞｰ		1
	int wadr,wadr2;			// DATA ADDRES WORK	2, 3
	int tadr, tadr2;		// DATA TOP ADDRES	4, 5
	int volume;				// VOLUME DATA		6
	int alg;				// ｱﾙｺﾞﾘｽﾞﾑ No.		7
	int chnum;				// ﾁｬﾝﾈﾙ ﾅﾝﾊﾞｰ     	8
	int detune, dtmp;		// ﾃﾞﾁｭｰﾝ DATA		9, 10
	int tllfo;				// for TLLFO		11
	int reverb;				// for ﾘﾊﾞｰﾌﾞ		12
	int d1, d2, d3, d4, d5;
							// SOFT ENVE DUMMY	13 - 17
	int quantize;			// qｵﾝﾀｲｽﾞ		18
	int lfo_delay;			// LFO DELAY		19
	int work1;				// WORK			20
	int lfo_counter;		// LFO COUNTER		21
	int work2;				// WORK			22
	int lfo_diff, ldtmp;	// LFO ﾍﾝｶﾘｮｳ 2BYTE	23, 24
	int work3, work4;		// WORK			25, 26
	int lfo_peak;			// LFO PEAK LEVEL	27
	int work5;				// WORK			28
	int fnum1;				// FNUM1 DATA		29
	int fnum2;				// B / FNUM2 DATA		30
	int flag;				
							// bit 7 = LFO FLAG	31
							// bit 6 = KEYOFF FLAG
							// bit 5 = LFO CONTINUE FLAG
							// bit 4 = TIE FLAG
							// bit 3 = MUTE FLAG
							// bit 2 = LFO 1SHOT FLAG
							// bit 0,1 = LOOPEND FLAG

	int code;				// BEFORE CODE		32
	int flag2;				// bit 6 = TL LFO FLAG     33
							// bit 5 = REVERVE FLAG
							// bit 4 = REVERVE MODE
	int retadr1, retadr2;	// ﾘﾀｰﾝｱﾄﾞﾚｽ	34, 35
	int pan, keyon;			// 36, 37 (ｱｷ) = 代わりにpan,keyon dataを入れている

	int vnum_org;			// 音色No.(オリジナル)
	int vol_org;			// ボリューム(オリジナル)

} PCHDATA;


//
//	CMucom.cpp functions
//
class mucomvm;

class CMucom {
public:
	CMucom();
	~CMucom();

	void Init(void *window = NULL, int option = 0, int Rate = 0);
	void Reset(int option=0);

	int Play(int num=0);
	int Stop(int option=0);
	int Fade(void);
	int PlayEffect(int num=0);

	void PlayLoop();
	void RenderAudio(void *mix, int size);
	void UpdateTime(int tick_ms);

	int LoadPCM(const char *fname = MUCOM_DEFAULT_PCMFILE);
	int LoadFMVoice(const char *fname = MUCOM_DEFAULT_VOICEFILE);
	int LoadMusic(const char *fname, int num = 0);
	int CompileFile(const char *fname, const char *sname, int option=0);
	int Compile(char *text, const char *filename, int option=0);
	int ProcessFile(const char *fname);
	int ProcessHeader(char *text);
	int SaveMusic(const char *fname, int start, int length, int option = 0);
	int LoadTagFromMusic(int num);
	void AddExtraInfo(char *mmlsource);

	const char *GetMessageBuffer(void);
	int GetStatus(int option);
	void SetVMOption(int option, int mode);
	void SetAudioRate(int rate);

	const char *GetInfoBuffer(void);
	const char *GetInfoBufferByName(const char *name);
	void DeleteInfoBuffer(void);
	void PrintInfoBuffer(void);

	int MUBGetHeaderVersion(MUBHED *hed);
	char *MUBGetData(MUBHED *hed, int &size);
	char *MUBGetTagData(MUBHED *hed, int &size);
	char *MUBGetPCMData(MUBHED *hed, int &size);

	int ConvertADPCM(const char *fname, const char *sname);
	void GetMD5(char *res, char *buffer, int size);
	void SetUUID(char *uuid);
	void SetVolume(int fmvol, int ssgvol);
	void SetFastFW(int value);

	int GetChannelData(int ch, PCHDATA *result);

private:
	//		Settings
	//
	int	flag;			// flag (0=none/1=active)
	bool playflag;		// playing flag
	char pcmfilename[MUCOM_FILE_MAXSTR];	// loaded PCM file
	int mubver;			// playing MUB version
	MUBHED *hedmusic;	// playing Music data header

	//		Compile Status
	//
	int maxch;
	int fmvoice;
	int tcount[MUCOM_MAXCH];
	int lcount[MUCOM_MAXCH];
	int maxcount;
	int basicsize;
	int mubsize;
	short jumpcount;	// Jump count (for skip)
	short jumpline;		// Jump line number (for skip)
	unsigned char linebuf[MUCOM_LINE_MAXSTR];
	char infoname[64];
	CMemBuf *infobuf;
	char user_uuid[64];

	const char *GetTextLine(const char *text);
	int StoreBasicSource(char *text, int line, int add);

	//		Virtual Machine
	//
	mucomvm *vm;

	//		Sound Buffer
	void MusicBufferInit(void);
	void MusicBufferTerm(void);
	CMemBuf *musbuf[MUCOM_MUSICBUFFER_MAX];

	//		Audio
	double AudioStep;
	double AudioLeftMs;

};


#endif
