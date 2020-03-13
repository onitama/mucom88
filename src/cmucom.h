
//
//	cmucom.cpp structures
//
#ifndef __CMucom_h
#define __CMucom_h

#include <vector>
#include <string>

#include "Z80/Z80.h"

#include "membuf.h"
#include "voiceformat.h"

#include "utils/logwrite.h"
#include "utils/wavwrite.h"

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
#define MUCOM_STATUS_AUDIOMS 11

#define MUCOM_STATUS_SNDDRV 0x100

#define MUCOM_OPTION_FMMUTE 1
#define MUCOM_OPTION_SCCI 2
#define MUCOM_OPTION_FASTFW 4
#define MUCOM_OPTION_STEP 8

#define MUCOM_EM_MUBSIZE_MAX (0x8000-0x0000)
#define MUCOM_EM_BASICSIZE_MAX 0x8000


#define MUCOM_MUBSIZE_MAX (0xE300-0xC200)
#define MUCOM_BASICSIZE_MAX 0x6000

#define MUCOM_HEADER_VERSION1 1		// 1.0 Header
#define MUCOM_HEADER_VERSION2 2		// 2.0 Header

#define MUCOM_FLAG_SJISTAG 0		// TAG dataの文字コードはSJIS
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

#define MUCOM_FMVOICE_MAX 32

#define MUCOM_CMPOPT_USE_EXTROM 1
#define MUCOM_CMPOPT_COMPILE 2
#define MUCOM_CMPOPT_STEP 8
#define MUCOM_CMPOPT_INFO 0x100

#define MUCOM_COMPILE_IGNOREVOICE 1
#define MUCOM_COMPILE_IGNOREPCM 2
#define MUCOM_COMPILE_TO_MUSBUFFER 8

#define MUCOM_EM_FMVOICE_ROOM_ADR 0xC200 // コンパイル初期化前アドレス(em版)

#define MUCOM_FMVOICE_ADR 0x6000
#define MUCOM_FMVOICE_SIZE 0x2000
#define MUCOM_FMVOICE_MAXNO 256

#define MUCOM_FMVOICE_MODE_EXTERNAL 0		// 音色ファイルをオリジナルファイルから読み込んで使用
#define MUCOM_FMVOICE_MODE_INTERNAL 1		// 音色ファイルを一時ファイルから読み込んで使用

#define MUCOM_NOTICE_MMLCHANGE 1	// MMLコード変更の通知
#define MUCOM_NOTICE_VOICECHANGE 2	// 音色変更の通知
#define MUCOM_NOTICE_PCMCHANGE 4	// PCM変更の通知
#define MUCOM_NOTICE_MMLERROR 0x100	// MMLが原因のエラー通知

#define MUCOM_NOTICE_SYSERROR 0x1000	// 不明なシステムエラー通知

#define MUCOM_EDIT_STATUS_NONE 0		// 編集中MMLなし
#define MUCOM_EDIT_STATUS_SAVED 1		// 編集中MML(保存済み)
#define MUCOM_EDIT_STATUS_CHANGED 2		// 編集中MML(未保存)
#define MUCOM_EDIT_STATUS_VOICEEDIT 4	// 音色編集中(未保存)

#define MUCOM_EDIT_OPTION_SJIS 0	// 編集中MMLの文字コードはSJIS
#define MUCOM_EDIT_OPTION_UTF8 1	// 編集中MMLの文字コードはUTF-8

#define MUCOM_DRIVER_UNKNOWN -1 	// 不明
#define MUCOM_DRIVER_NONE 0 		// 未定義
#define MUCOM_DRIVER_MUCOM88 1		// オリジナルのドライバ(1.7)
#define MUCOM_DRIVER_MUCOM88E 2		// オリジナルのドライバ(1.5)
#define MUCOM_DRIVER_MUCOM88EM 4	// 拡張メモリ版ドライバ(1.7)
#define MUCOM_DRIVER_MUCOMDOTNET 8	// MucomDotNET

#define MUCOM_ORIGINAL_VER_17 0		// Mucom88オリジナルのドライバ(1.7)
#define MUCOM_ORIGINAL_VER_15 1		// Mucom88オリジナルのドライバ(1.5)


// 共通
#define MUCOM_ADDRESS_BASIC 0x1000
#define MUCOM_ADDRESS_POLL_VECTOR 0x0eea8


// em 開始アドレス
#define MUCOM_ADDRESS_EM_EXPAND 0xB800
#define MUCOM_ADDRESS_EM_ERRMSG 0x8800
#define MUCOM_ADDRESS_EM_MSUB 0x9000
#define MUCOM_ADDRESS_EM_MUC88 0x9600
#define MUCOM_ADDRESS_EM_SSGDAT 0xBE00
#define MUCOM_ADDRESS_EM_TIME 0xE400
#define MUCOM_ADDRESS_EM_SMON 0xDE00

#define MUCOM_ADDRESS_EM_MUSIC 0xC000

#define MUCOM_ADDRESS_EM_SSGDAT_AFTER 0xc200


// ルーチン
#define MUCOM_ADDRESS_EM_ERAM_TABLE 0x95A0 // 拡張RAM切り替えルーチン テーブル
#define MUCOM_ADDRESS_EM_CINT 0x9600 // コンパイラ初期化

#define MUCOM_ADDRESS_EM_WKGET 0xC02A



// オリジナル 開始アドレス
#define MUCOM_ADDRESS_EXPAND 0xab00
#define MUCOM_ADDRESS_ERRMSG 0x8800
#define MUCOM_ADDRESS_MSUB 0x9000
#define MUCOM_ADDRESS_MUC88 0x9600
#define MUCOM_ADDRESS_SSGDAT 0x5e00
#define MUCOM_ADDRESS_TIME 0xe400
#define MUCOM_ADDRESS_SMON 0xde00

#define MUCOM_ADDRESS_MUSIC 0xb000

// ルーチン
#define MUCOM_ADDRESS_CINT 0x9600 // コンパイラ初期化
#define MUCOM_ADDRESS_RETW 0xb00c // music2:RETW

// 曲データアドレス
#define MUCOM_ADDRESS_SONG 0xc200
#define MUCOM_ADDRESS_EM_SONG 0x0000

// ワークアドレス
#define MUCOM_ADDRESS_JCLOCK 0x8c90
#define MUCOM_ADDRESS_JPLINE 0x8c92
#define MUCOM_ADDRESS_DEFVOICE 0x8c50


// 曲再生ルーチン オフセット
#define MUCOM_MUSIC_OFFSET_MSTART 0x0000
#define MUCOM_MUSIC_OFFSET_MSTOP 0x0003
#define MUCOM_MUSIC_OFFSET_MFADE 0x0006





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


//	Temporary File Storage Class
//
class CMucomTempFileInfo {
public:
	CMucomTempFileInfo();
	~CMucomTempFileInfo();

	std::string filename;		// original file name
	std::string tempfilename;	// temp file name
	std::string pathname;		// path name
};

//
//	CMucom.cpp functions
//
class mucomvm;

class CMucom {
public:
	CMucom();
	~CMucom();

	//	MUCOM88 main service
	void Init(void *window = NULL, int option = 0, int Rate = 0);
	void Reset(int option=0);
	void LoadOriginal(int option);
	void LoadPlayer(int option);
	void SetChannelWork();
	int Play(int num=0);
	int Stop(int option=0);
	int Restart(void);
	int Fade(void);
	int Update(void);
	int PlayEffect(int num=0);

	void PlayMemory();

	//	Service for command line
	void PlayLoop();
	void SetWavFilename(const char *wavFilename);
	void Record(int seconds);
	void RenderAudio(void *mix, int size);
	void UpdateTime(int tick_ms);

	//	PCM file service
	int LoadPCM(const char *fname = MUCOM_DEFAULT_PCMFILE);

	//	MUCOM88 MUC/MUB service
	int LoadMusic(const char *fname, int num = 0);
	int CompileFile(const char *fname, const char *sname, int option=0);
	int CompileMemory(const char* fname, int option = 0);
	int CompileMem(char *mem, int option=0);
	int Compile(char *text, const char *filename, int option=0);
	int Compile(char* text, int option = 0, bool writeMub=false, const char *filenamw = NULL);
	void PutMucomHeader(const char *stmp);
	int ProcessFile(const char *fname);
	int ProcessHeader(char *text);
	int SaveMusic(const char *fname, int start, int length, int option = 0);
	int LoadTagFromMusic(int num);
	void AddExtraInfo(char *mmlsource);

	//	VM log service
	const char *GetMessageBuffer(void);
	int GetMessageBufferSize(void);
	int GetStatus(int option);
	void SetVMOption(int option, int mode);
	void SetAudioRate(int rate);

	//	MML TAG service
	const char *GetInfoBuffer(void);
	const char *GetInfoBufferByName(const char *name);
	void DeleteInfoBuffer(void);
	void PrintInfoBuffer(void);

	//	MUB Header utility
	int MUBGetHeaderVersion(MUBHED *hed);
	char *MUBGetData(MUBHED *hed, int &size);
	char *MUBGetTagData(MUBHED *hed, int &size);
	char *MUBGetPCMData(MUBHED *hed, int &size);

	//	Editor Support Service
	void EditorReset(const char *mml=NULL, int option = 0);
	void EditorSetFileName(const char *filename, const char *pathname="", bool sessionstart=false);
	int UpdateEditor(void);
	int GetEditorNotice(void) { return edit_notice; }
	int GetEditorStatus(void) { return edit_status; }
	int GetEditorOption(void) { return edit_option; }
	const char *GetEditorFileName(void) { return edit_filename.c_str(); }
	const char *GetEditorPathName(void) { return edit_pathname.c_str(); }
	int CheckEditorUpdate(void);
	int UpdateEditorMML(const char *mml);
	char *GetEditorMML(void) { return edit_buffer; }
	const char *GetRequestMML(void);
	int GetEditorPosToLine(int pos);
	int SaveEditorMML(const char *filename);
	int RequestEditorMML(const char *mml);

	//	Plugin Service
	int AddPlugins(const char *filename, int bootopt);
	void NoticePlugins(int cmd, void *p1 = NULL, void *p2 = NULL);

	//	Utility
	int ConvertADPCM(const char *fname, const char *sname);
	void GetMD5(char *res, char *buffer, int size);
	void SetUUID(char *uuid);

	//	Player Service
	void SetFastFW(int value);
	void SetVolume(int fmvol, int ssgvol);
	int GetChannelData(int ch, PCHDATA *result);

	//	FM Voice Service
	void InitFMVoice(unsigned char *voice = NULL);
	int LoadFMVoiceFromTAG(void);
	int LoadFMVoice(const char *fname = MUCOM_DEFAULT_VOICEFILE, bool sw = false);
	int GetFMVoiceMode(void) { return fmvoice_mode; }
	int SaveFMVoice(bool sw = true);
	void StoreFMVoice(unsigned char *voice);
	void DumpFMVoice(int no);
	char *DumpFMVoiceAll(void);
	MUCOM88_VOICEFORMAT *GetFMVoice(int no);
	int UpdateFMVoice(int no, MUCOM88_VOICEFORMAT *voice);
	int StoreFMVoiceFromEmbed(void);
	int SendFMVoiceMemory(const unsigned char* src, int offset, int size);
	const char *GetVoiceFileName(void) { return voicefilename.c_str(); }
	const char *GetVoicePathName(void) { return voice_pathname.c_str(); }
	MUCOM88_VOICEFORMAT *GetVoiceData(void) { return fmvoice_internal; }
	MUCOM88_VOICEFORMAT *GetVoiceDataOrg(void) { return fmvoice_original; }
	int GetUseVoiceNum(int num) { return (int)fmvoice_use[num]; }
	int GetUseVoiceMax(void) { return fmvoice_usemax; }

	// FM Log Service
	void SetLogFilename(const char *name);

	// Other Service
	void GetFMRegMemory(unsigned char* data, int address, int length);
	void GetMemory(unsigned char *data, int address, int length);

	void GetMainMemory(unsigned char* data, int address, int length);
	void SetMainMemory(unsigned char* data, int address, int length);

	void GetExtMemory(unsigned char* data, int bank, int address, int length);
	void SetExtMemory(unsigned char* data, int bank, int address, int length);

	void SetChMute(int ch, bool sw);
	bool GetChMute(int ch);
	void FMRegDataOut(int reg, int data);
	int FMRegDataGet(int reg);

	int Peek(uint16_t adr);
	int Peekw(uint16_t adr);
	void Poke(uint16_t adr, uint8_t data);
	void Pokew(uint16_t adr, uint16_t data);

	//	driver selector
	int GetDriverMode(char* fname);
	int GetDriverModeMUB(char* fname);
	int GetDriverModeMem(char* mem);
	int GetDriverModeString(const char* name);
	void SetDriverMode(int driver);

	// ExtRam
	void GetExtramVector();

	void ChangeMemoryToSong();
	void RestoreMemory();

	int GetSongAddress();

	// Debug
	void EnableBreakPoint(uint16_t adr);
	void DisableBreakPoint();
	void DebugRun();
	void DebugInstExec();
	void DebugPause();
	void GetRegSet(RegSet *reg);


private:
	//		Settings
	//
	int	flag;			// flag (0=none/1=active)
	bool playflag;		// playing flag
	char pcmfilename[MUCOM_FILE_MAXSTR];	// loaded PCM file
	int mubver;			// playing MUB version
	MUBHED *hedmusic;	// playing Music data header

	ILogWrite *p_log;
	WavWriter *p_wav;

	bool original_mode; // original mode
	bool octreverse_mode; // octave reverse mode
	bool compiler_initialized; // 初期化後
	bool use_extram;
	int  original_ver;	// original mode version (MUCOM_ORIGINAL_VER_*)
	int extram_disable_vec;
	int extram_enable_vec;

	int extram_last_bank;
	int extram_last_mode;

	int music_start_address;

	//		FM voice status
	//
	int fmvoice_mode;
	std::string voicefilename;	// loaded VOICE file
	std::string tempfilename;	// temp VOICE file
	std::string voice_pathname;	// voice pathname
	MUCOM88_VOICEFORMAT fmvoice_internal[MUCOM_FMVOICE_MAXNO];
	MUCOM88_VOICEFORMAT *fmvoice_original;
	unsigned char fmvoice_use[MUCOM_FMVOICE_MAX];	// use FM voice no. table
	int fmvoice_usemax;							// use FM voice table max

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

	//		Editor Status
	//
	int edit_status;			// current status
	int	edit_notice;			// notice code (MUCOM_NOTICE_*)
	int edit_option;			// option (MUCOM_EDIT_OPTION_*)
	int edit_autosave;			// auto save mode (1=on)
	int edit_autosave_time;		// auto save timer (sec)
	int edit_autosave_next;		// auto save next time (Tick)
	char *edit_buffer;			// current buffer
	std::string edit_master;	// saved buffer
	std::string edit_request;	// external request buffer
	std::string edit_filename;	// MML filename
	std::string edit_pathname;	// MML pathname

	//		Internal Utility
	//
	int htoi_sub(char hstr);
	int htoi(char *str);
	int strpick_spc(char *target, char *dest, int strmax);

	int GetMultibyteCharacter(const unsigned char *text);
	const char *GetTextLine(const char *text);
	int StoreBasicSource(char *text, int line, int add);
	bool hasMacro(char *text);

	void InitCompiler();
	void LoadModBinary(int option);
	void LoadExternalCompiler();
	void LoadInternalCompiler();


	//		Virtual Machine
	//
	mucomvm *vm;

	//		Sound Buffer
	void MusicBufferInit(void);
	void MusicBufferTerm(void);
	CMemBuf *musbuf[MUCOM_MUSICBUFFER_MAX];

	//		Audio
	int AudioCurrentRate;
	double AudioStep;
	double AudioLeftMs;

};


#endif
