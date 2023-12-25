
//
//		MUCOM88 access class
//		(PC-8801のVMを介してMUCOM88の機能を提供します)
//			MUCOM88 by Yuzo Koshiro Copyright 1987-2020(C) 
//			Windows version by onion software/onitama 2018/11
//

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils/s98write.h"
#include "utils/vgmwrite.h"
#include "utils/wavwrite.h"

#include "cmucom.h"
#include "mucomvm.h"
#include "mucomerror.h"
#include "md5.h"

#include "bin_15/bin_music2_15.h"

#include "bin_15/bin_expand15.h"
#include "bin_15/bin_errmsg15.h"
#include "bin_15/bin_msub15.h"
#include "bin_15/bin_muc88_15.h"
#include "bin_15/bin_ssgdat15.h"
#include "bin_15/bin_time15.h"
#include "bin_15/bin_voice15.h"

#include "bin_17/bin_music2.h"

#include "bin_17/bin_expand.h"
#include "bin_17/bin_errmsg.h"
#include "bin_17/bin_msub.h"
#include "bin_17/bin_muc88.h"
#include "bin_17/bin_ssgdat.h"
#include "bin_17/bin_time.h"
#include "bin_17/bin_voice.h"
#include "bin_17/bin_smon.h"

#include "bin_em/bin_expand_em.h"
#include "bin_em/bin_errmsg_em.h"
#include "bin_em/bin_msub_em.h"
#include "bin_em/bin_muc88_em.h"
#include "bin_em/bin_ssgdat_em.h"
#include "bin_em/bin_time_em.h"
#include "bin_em/bin_smon_em.h"
#include "bin_em/bin_music_em.h"

#include "bin_cnvkana.h"

#ifdef __APPLE__
#define STRCASECMP strcasecmp
#else
#ifdef _WIN32
#define STRCASECMP _strcmpi
#else
#define STRCASECMP strcasecmp
#endif
#endif

#define PRINTF vm->Msgf
#define PRINTF_NOCONV vm->MsgfNoConvert

int CMucom::htoi_sub(char hstr)
{
	//	exchange hex to int

	char a1;
	a1 = tolower(hstr);
	if ((a1 >= '0') && (a1 <= '9')) return a1 - '0';
	if ((a1 >= 'a') && (a1 <= 'f')) return a1 - 'a' + 10;
	return 0;
}


int CMucom::htoi(char *str)
{
	//	16進->10進変換
	//
	char a1;
	int d;
	int conv;
	conv = 0;
	d = 0;
	while (1) {
		a1 = str[d++]; if (a1 == 0) break;
		conv = (conv << 4) + htoi_sub(a1);
	}
	return conv;
}

int CMucom::strpick_spc(char *target, char *dest, int strmax)
{
	//		strの先頭からspaceまでを小文字として取り出す
	//
	unsigned char *p;
	unsigned char *dst;
	unsigned char a1;
	int len = 0;
	int mulchr;
	p = (unsigned char *)target;
	dst = (unsigned char *)dest;
	while (1) {
		if (len >= strmax) break;
		a1 = *p; if (a1 == 0) return 0;
		if (a1 == 32) break;

		mulchr = GetMultibyteCharacter(p);
		if (mulchr == 1) {
			*dst++ = tolower(a1);
			p++;
			len++;
			continue;
		}
		while (mulchr > 0) {
			*dst++ = *p++;
			len++;
		}
	}
	*dst++ = 0;
	return len;
}


/*------------------------------------------------------------*/
/*
		interface
*/
/*------------------------------------------------------------*/

CMucom::CMucom( void )
{
	playflag = false;
	original_mode = true;
	octreverse_mode = false;
	original_ver = MUCOM_ORIGINAL_VER_17;
	compiler_initialized = false;
	use_extram = false;
	flag = 0;
	vm = NULL;
	infobuf = NULL;
	edit_buffer = NULL;
	edit_status = MUCOM_EDIT_STATUS_NONE;
	user_uuid[0] = 0;
	hedmusic = NULL;

	music_start_address = MUCOM_ADDRESS_MUSIC;

	p_log = NULL;
	p_wav = NULL;
}


CMucom::~CMucom( void )
{
	Stop(1);
	DeleteInfoBuffer();
	MusicBufferTerm();
	if (vm != NULL) { delete vm; vm = NULL; }
	if (p_log != NULL) { delete p_log; p_log = NULL; }
	if (p_wav != NULL) { delete p_wav; p_wav = NULL; }

}

void CMucom::SetLogFilename(const char *filename) {
	if (p_log != NULL) delete p_log;
	const char *p = strrchr(filename, '.');

	bool use_vgm = false;

	if (p != NULL && STRCASECMP(p, ".vgm") == 0) use_vgm = true;

	if (use_vgm) {
		p_log = new VGMWrite();
	} else {
		p_log = new S98Write();
	}

	p_log->Open(filename);
}


void CMucom::Init(void *window, int option, int rate)
{
	//		MUCOM88の初期化(初回だけ呼び出してください)
	//		window : 0   = ウインドウハンドル(HWND)
	//		               (NULLの場合はアクティブウインドウが選択される)
	//		option : 0   = 1:FMをミュート  2:SCCIを使用
	//
	vm = new mucomvm;
	flag = 1;
	if (original_mode) vm->SetOrignalMode();

	// レート設定
	AudioCurrentRate = rate;
	if (rate == 0) AudioCurrentRate = MUCOM_AUDIO_RATE;
	SetAudioRate(AudioCurrentRate);

	if (window != NULL) {
		vm->SetWindow(window);
	}
	vm->SetOption(option);
	vm->InitSoundSystem(AudioCurrentRate);
	MusicBufferInit();
	vm->SetMucomInstance(this);			// Mucomのインスタンスを通知する(プラグイン用)

	//		オートセーブを初期化(デフォルトは無効)
	edit_autosave = 0;
	edit_autosave_time = 0;
	edit_autosave_next = 0;

	//		エディタが保持する情報のリセット
	EditorReset();
	InitFMVoice();
}


int CMucom::AddPlugins(const char *filename, int bootopt)
{
	//		プラグイン追加
	//		filename = DLLファイル名(今のところWinのみ)
	//		bootopt = 起動オプション
	//
	return vm->AddPlugins(filename, bootopt);
}


void CMucom::NoticePlugins(int cmd, void *p1, void *p2)
{
	//		プラグイン通知
	//
	vm->NoticePlugins(cmd,p1,p2);
}


int CMucom::DeletePlugins(const char* filename)
{
	//		プラグイン削除
	//		filename = DLLファイル名(今のところWinのみ)
	//
	return vm->DeletePlugins(filename);
}


void CMucom::SetAudioRate(int rate)
{
	//		外部レンダリング用の出力レート設定
	//
	AudioStep = (double)1000.0 / rate;
	AudioLeftMs = 0.0;
}


void CMucom::Reset(int option)
{
	//		MUCOM88のリセット(何度でも呼べます)
	//		option : 0   = 内部のプレイヤー初期化
	//		         1   = 外部ファイルによる初期化
	//		         2,3 = コンパイラを初期化
	//
	int devres;

	vm->SetOrignalMode(original_mode);

	vm->SetLogWriter(p_log);
	vm->SetWavWriter(p_wav);

	music_start_address = original_mode ? MUCOM_ADDRESS_MUSIC : MUCOM_ADDRESS_EM_MUSIC;

	vm->Reset();
	PRINTF("#OpenMucom88 Ver.%s Copyright 1987-2020(C) Yuzo Koshiro\r\n",VERSION);
	pcmfilename[0] = 0;

	devres = vm->DeviceCheck();
	if (devres) {
		PRINTF("#Device error(%d)\r\n", devres);
	}

	if (original_mode) LoadOriginal(option); else LoadModBinary(option);


	//	実行用メモリをシャドーコピーとして保存しておく
	vm->SendMemoryToShadow();

	DeleteInfoBuffer();

	SetChannelWork();

	NoticePlugins(MUCOM88IF_NOTICE_RESET);
}

void CMucom::LoadOriginal(int option)
{
	LoadInternalCompiler();

	if (option & MUCOM_CMPOPT_COMPILE) {
		if (option & MUCOM_CMPOPT_USE_EXTROM) {
			LoadExternalCompiler();
		}

		vm->FillMem(MUCOM_ADDRESS_MUSIC, 0xc9, 0x40);
		vm->FillMem(MUCOM_ADDRESS_BASIC, 0xc9, 0x4000);
	}

	LoadPlayer(option);
}

void CMucom::LoadPlayer(int option)
{
	if (!original_mode) return;

	if (option & MUCOM_CMPOPT_USE_EXTROM) {
		//	プレイヤーをファイルから読む
		vm->LoadMem("music", MUCOM_ADDRESS_MUSIC, 0);
	} else {
		//	内部のプレイヤーを読む
		vm->SendMem(bin_music2, MUCOM_ADDRESS_MUSIC, music2_size);
	}
}

void CMucom::SetChannelWork()
{
	int i, adr;
	vm->InitChData(MUCOM_MAXCH, MUCOM_CHDATA_SIZE);
	for (i = 0; i < MUCOM_MAXCH; i++) {
		if (original_mode) {
			adr = vm->CallAndHaltWithA(MUCOM_ADDRESS_RETW, i); // B00C = music2:RETW
		}
		else {
			adr = vm->CallAndHaltWithB(MUCOM_ADDRESS_EM_WKGET, i + 1); // B02A = WKGET
		}

		vm->SetChDataAddress(i, adr);
	}
}

void CMucom::LoadModBinary(int option)
{
	// 外部ファイル
	if (original_mode) { return; }

	vm->SendMem(bin_expand_em, MUCOM_ADDRESS_EM_EXPAND, expand_em_size);
	vm->SendMem(bin_errmsg_em, MUCOM_ADDRESS_EM_ERRMSG, errmsg_em_size);
	vm->SendMem(bin_msub_em, MUCOM_ADDRESS_EM_MSUB, msub_em_size);
	vm->SendMem(bin_muc88_em, MUCOM_ADDRESS_EM_MUC88, muc88_em_size);
	vm->SendMem(bin_ssgdat_em, MUCOM_ADDRESS_EM_SSGDAT, ssgdat_em_size);
	vm->SendMem(bin_time_em, MUCOM_ADDRESS_EM_TIME, time_em_size);
	vm->SendMem(bin_smon_em, MUCOM_ADDRESS_EM_SMON, smon_em_size);

	vm->SendMem(bin_music_em, MUCOM_ADDRESS_EM_MUSIC, music_em_size);
	StoreFMVoice((unsigned char*)bin_voice_dat);

	if (option & MUCOM_CMPOPT_USE_EXTROM) {
		vm->LoadMem("expand", MUCOM_ADDRESS_EM_EXPAND, 0);
		vm->LoadMem("errmsg", MUCOM_ADDRESS_EM_ERRMSG, 0);
		vm->LoadMem("msub", MUCOM_ADDRESS_EM_MSUB, 0);
		vm->LoadMem("muc88", MUCOM_ADDRESS_EM_MUC88, 0);
		vm->LoadMem("ssgdat", MUCOM_ADDRESS_EM_SSGDAT, 0);
		vm->LoadMem("time", MUCOM_ADDRESS_EM_TIME, 0);
		vm->LoadMem("smon", MUCOM_ADDRESS_EM_SMON, 0);

		vm->LoadMem("music", MUCOM_ADDRESS_EM_MUSIC, 0);

		LoadFMVoice(MUCOM_DEFAULT_VOICEFILE, true);
	}
}

void CMucom::LoadExternalCompiler()
{
	if (!original_mode) { return; }

	//	コンパイラをファイルから読む
	vm->LoadMem("expand", MUCOM_ADDRESS_EXPAND, 0);
	vm->LoadMem("errmsg", MUCOM_ADDRESS_ERRMSG, 0);
	vm->LoadMem("msub", MUCOM_ADDRESS_MSUB, 0);
	vm->LoadMem("muc88", MUCOM_ADDRESS_MUC88, 0);
	vm->LoadMem("ssgdat", MUCOM_ADDRESS_SSGDAT, 0);
	vm->LoadMem("time", MUCOM_ADDRESS_TIME, 0);
	vm->LoadMem("smon", MUCOM_ADDRESS_SMON, 0);
	LoadFMVoice(MUCOM_DEFAULT_VOICEFILE, true);
}

void CMucom::LoadInternalCompiler()
{
	if (!original_mode) { return; }

	if (original_ver == MUCOM_ORIGINAL_VER_17) {
		//	内部のコンパイラを読む(1.7)
		vm->SendMem(bin_expand, MUCOM_ADDRESS_EXPAND, expand_size);
		vm->SendMem(bin_errmsg, MUCOM_ADDRESS_ERRMSG, errmsg_size);
		vm->SendMem(bin_msub, MUCOM_ADDRESS_MSUB, msub_size);
		vm->SendMem(bin_muc88, MUCOM_ADDRESS_MUC88, muc88_size);
		vm->SendMem(bin_ssgdat, MUCOM_ADDRESS_SSGDAT, ssgdat_size);
		vm->SendMem(bin_time, MUCOM_ADDRESS_TIME, time_size);
		vm->SendMem(bin_smon, MUCOM_ADDRESS_SMON, smon_size);
		StoreFMVoice((unsigned char*)bin_voice_dat);
	}

	if (original_ver == MUCOM_ORIGINAL_VER_15) {
		//	内部のコンパイラを読む(1.5)
		vm->SendMem(bin15_expand, MUCOM_ADDRESS_EXPAND, expand_size15);
		vm->SendMem(bin15_errmsg, MUCOM_ADDRESS_ERRMSG, errmsg_size15);
		vm->SendMem(bin15_msub, MUCOM_ADDRESS_MSUB, msub_size15);
		vm->SendMem(bin15_muc88, MUCOM_ADDRESS_MUC88, muc88_size15);
		vm->SendMem(bin15_ssgdat, MUCOM_ADDRESS_SSGDAT, ssgdat_size15);
		vm->SendMem(bin15_time, MUCOM_ADDRESS_TIME, time_size15);
		vm->SendMem(bin_smon, MUCOM_ADDRESS_SMON, smon_size);
		StoreFMVoice((unsigned char*)bin15_voice_dat);
	}
}

void CMucom::SetUUID(char *uuid)
{
	if (uuid == NULL) {
		user_uuid[0] = 0;
		return;
	}
	strncpy(user_uuid,uuid,64);
}


const char *CMucom::GetMessageBuffer(void)
{
	return vm->GetMessageBuffer();
}

int CMucom::GetMessageBufferSize(void)
{
	return vm->GetMessageBufferSize();
}


int CMucom::Play(int num, bool start)
{
	//		MUCOM88音楽再生
	//		num : 0   = 音楽No. (0～15)
	//		(戻り値が0以外の場合はエラー)
	//
	char *data;
	char *pcmdata;
	int datasize;
	int pcmsize;

	if ((num < 0) || (num >= MUCOM_MUSICBUFFER_MAX)) return -1;

	Stop();

	hedmusic = NULL;

	CMemBuf *buf = musbuf[num];
	if (buf == NULL) return -1;

	hedmusic = (MUBHED *)buf->GetBuffer();
	mubver = MUBGetHeaderVersion(hedmusic);
	if (mubver < 0) return -2;

	LoadTagFromMusic(num);

	data = MUBGetData(hedmusic, datasize);
	if (data == NULL) return -3;

	if (hedmusic->pcmdata) {
		int skippcm = 0;
		const char *pcmname = GetInfoBufferByName("pcm");
		if (pcmname[0] != 0) {
			//	既に同名のPCMが読み込まれているか?
			if (strcmp(pcmname, pcmfilename) == 0) skippcm = 1;
		}
		if (skippcm==0) {
			//	埋め込みPCMを読み込む
			pcmdata = MUBGetPCMData(hedmusic, pcmsize);
			vm->LoadPcmFromMem(pcmdata, pcmsize);
		}
	}

	LoadFMVoiceFromTAG();

	ChangeMemoryToSong();
	int song_address = GetSongAddress();
	PRINTF("#Load song:%04x size:%04x\r\n", song_address, datasize);

	vm->SendMem((const unsigned char *)data, song_address, datasize);
	RestoreMemory();

	StoreFMVoiceFromEmbed();

	NoticePlugins(MUCOM88IF_NOTICE_PREPLAY);

	if (start) {
		PRINTF("#Play[%d]\r\n", num);
		PlayMemory();
	}

	return 0;
}

// 曲データ再生
void CMucom::PlayMemory() {

	vm->DebugEnable();

	bool CompileMode = hedmusic == NULL;

	if (!original_mode) {
		if (!CompileMode) InitCompiler();

		int vec = vm->Peekw(MUCOM_ADDRESS_POLL_VECTOR); // EEA8 = POLLコマンドアドレス
		PRINTF("#poll s $%x.\r\n", vec);
		vm->CallAndHalt2(vec, 'S');
	} else {
		vm->CallAndHalt(music_start_address + MUCOM_MUSIC_OFFSET_MSTART); // MSTART
	}

	//int vec = vm->Peekw(0xf308);
	//PRINTF("#INT3 $%x.\r\n", vec);

	if (original_mode) {
		if (vm->GetSB2Present() == false) {
			vm->Poke(0xbe78, 1);				// SB1の場合はnotsb2フラグを入れる
		}
	}

	NoticePlugins(MUCOM88IF_NOTICE_PLAY);

	if (!CompileMode) {
		int jcount = hedmusic->jumpcount;
		vm->SkipPlay(jcount);
	}

	vm->StartINT3();

	playflag = true;
}

void CMucom::GetFMRegMemory(unsigned char* data, int address, int length)
{
	vm->GetFMRegMemory(data, address, length);
}

void CMucom::GetMemory(unsigned char *data, int address, int length) {
	vm->GetMemory(data, address, length);
}

void CMucom::GetMainMemory(unsigned char* data, int address, int length) {
	vm->GetMainMemory(data, address, length);
}

void CMucom::SetMainMemory(unsigned char* data, int address, int length) {
	vm->SetMainMemory(data, address, length);
}

void CMucom::GetExtMemory(unsigned char* data, int bank, int address, int length) {
	vm->GetExtMemory(data, bank, address, length);
}


void CMucom::SetExtMemory(unsigned char* data, int bank, int address, int length) {
	vm->SetExtMemory(data, bank, address, length);
}

void CMucom::SetChMute(int ch, bool sw)
{
	vm->SetChMute(ch, sw);
}

bool CMucom::GetChMute(int ch)
{
	return vm->GetChMute(ch);
}

void CMucom::FMRegDataOut(int reg, int data)
{
	vm->FMRegDataOut(reg, data);
}

int CMucom::FMRegDataGet(int reg)
{
	return vm->FMRegDataGet(reg);
}

int CMucom::Peek(uint16_t adr)
{
	return vm->Peek(adr);
}

int CMucom::Peekw(uint16_t adr)
{
	return vm->Peekw(adr);
}

void CMucom::Poke(uint16_t adr, uint8_t data)
{
	vm->Poke(adr, data);
}

void CMucom::Pokew(uint16_t adr, uint16_t data)
{
	vm->Pokew(adr, data);
}

void CMucom::GetExtramVector()
{
	// ソースの位置を確認
	int eram_tbl = MUCOM_ADDRESS_EM_ERAM_TABLE;
	if (vm->Peek(eram_tbl) != 0xc3) return;
	if (vm->Peek(eram_tbl + 9) != 0xc3) return;
	use_extram = true;
	extram_disable_vec = eram_tbl;
	extram_enable_vec = eram_tbl + 9;
}

void CMucom::ChangeMemoryToSong()
{
	extram_last_bank = vm->GetExtRamBank();
	extram_last_mode = vm->GetExtRamMode();
	vm->ChangeExtRam(0x11, 0x00);
}

void CMucom::RestoreMemory()
{
	vm->ChangeExtRam(extram_last_bank, extram_last_mode);
}

void CMucom::PlayLoop() {
	vm->PlayLoop();
}

// AudioCurrentRateの設定が必要
void CMucom::SetWavFilename(const char *fname) {
	if (p_wav != NULL) delete p_wav;

	p_wav = new WavWriter();
	p_wav->SetFormat(AudioCurrentRate, 16, 2);
	if (!p_wav->Open(fname)) return;
}


void CMucom::Record(int seconds) {
	int buf[512];

	int TotalSamples = 0;

	SetVMOption(VM_OPTION_STEP, 1);		// オプションを設定

	while (TotalSamples < AudioCurrentRate * seconds) {
		int samples = 16;
		RenderAudio(buf, samples);
		TotalSamples += samples;
	}

	SetVMOption(VM_OPTION_STEP, 2);		// オプションを解除
}

// 時間を進めてレンダリングを行う
void CMucom::RenderAudio(void *mix, int size) {
	AudioLeftMs += size * AudioStep;
	int ms = (int)AudioLeftMs;
	if (ms > 0) UpdateTime(ms);
	AudioLeftMs -= ms;

	memset(mix, 0, size * 2 * sizeof(int));
	vm->RenderAudio(mix, size);
}


// 時間のみ更新
void CMucom::UpdateTime(int tick_ms) {
	vm->UpdateTime(tick_ms << TICK_SHIFT);
}

int CMucom::LoadTagFromMusic(int num)
{
	//		MUCOM88音楽データからタグ一覧を取得する
	//		num : 0   = 音楽No. (0～15)
	//		(戻り値が0以外の場合はエラー)
	//
	MUBHED *hed;
	int ver;
	int tagsize;

	if ((num < 0) || (num >= MUCOM_MUSICBUFFER_MAX)) return -1;

	DeleteInfoBuffer();
	infobuf = new CMemBuf();

	CMemBuf *buf = musbuf[num];
	if (buf == NULL) {
		infobuf->Put((int)0);
		return -1;
	}

	hed = (MUBHED *)buf->GetBuffer();
	ver = MUBGetHeaderVersion(hed);
	if (ver < 0) {
		infobuf->Put((int)0);
		return -1;
	}

	if (hed->tagdata) {
		infobuf->PutStr(MUBGetTagData(hed, tagsize));
		infobuf->Put((int)0);
	}

	return 0;
}


int CMucom::Stop(int option)
{
	//		MUCOM88音楽再生の停止
	//		(戻り値が0以外の場合はエラー)
	//

	vm->DebugDisable();

	playflag = false;
	vm->WaitReady();

	vm->StopINT3();
	vm->CallAndHalt(music_start_address + MUCOM_MUSIC_OFFSET_MSTOP);
	if (option & 1) { vm->ResetFM(); }

	NoticePlugins(MUCOM88IF_NOTICE_STOP);
	return 0;
}


int CMucom::Restart(void)
{
	//		MUCOM88音楽再生の再開(停止後)
	//		(戻り値が0以外の場合はエラー)
	//
	vm->DebugDisable();

	NoticePlugins(MUCOM88IF_NOTICE_PLAY); 
	vm->RestartINT3();

	vm->DebugEnable();
	playflag = true;
	return 0;
}


int CMucom::Fade(void)
{
	//		MUCOM88音楽フェードアウト
	//		(戻り値が0以外の場合はエラー)
	//
	vm->DebugDisable();

	if (playflag == false) return -1;
	vm->CallAndHalt(music_start_address + MUCOM_MUSIC_OFFSET_MFADE);
	return 0;
}


int CMucom::LoadPCM(const char * fname)
{
	//		ADPCMデータ読み込み
	//		fname = PCMデータファイル (デフォルトはMUCOM_DEFAULT_PCMFILE)
	//		(戻り値が0以外の場合はエラー)
	//
	if (strcmp(pcmfilename,fname)==0) return 0;			// 既に読み込んでいる場合はスキップ
	strncpy(pcmfilename, fname, MUCOM_FILE_MAXSTR-1);
	if (vm->LoadPcm(fname) == 0) return 0;
	PRINTF("#PCM file not found [%s].\r\n", fname);
	return -1;
}

int CMucom::LoadMusic(const char * fname, int num)
{
	//		音楽データ読み込み
	//		fname = 音楽データファイル
	//		num : 0   = 音楽No. (0～15)
	//		(戻り値が0以外の場合はエラー)
	//
	if ((num < 0) || (num >= MUCOM_MUSICBUFFER_MAX)) return -1;

	CMemBuf *buf = new CMemBuf();
	if (buf->PutFile((char *)fname) < 0) {
		PRINTF("#MUSIC file not found [%s].\r\n", fname);
		delete buf;
		return -1;
	}

	if (musbuf[num] != NULL) {
		delete musbuf[num];
	}

	musbuf[num] = buf;
	//if (vm->LoadMem(fname, 0xc200, 0) >= 0) return 0;

	NoticePlugins(MUCOM88IF_NOTICE_LOADMUB);

	return 0;
}

void CMucom::MusicBufferInit(void)
{
	for (int i = 0; i < MUCOM_MUSICBUFFER_MAX; i++) {
		musbuf[i] = NULL;
	}
}

void CMucom::MusicBufferTerm(void)
{
	for (int i = 0; i < MUCOM_MUSICBUFFER_MAX; i++) {
		if (musbuf[i] != NULL) {
			delete musbuf[i];
			musbuf[i] = NULL;
		}
	}
}

int CMucom::GetStatus(int option)
{
	//		MUCOMステータス読み出し
	//		option : 0  停止=0/演奏中=1
	//		         1  演奏開始からの割り込みカウント
	//		         2  ストリーム再生にかかった負荷(ms)
	//		         3  メジャーバージョンコード
	//		         4  マイナーバージョンコード
	//		(戻り値は32bit整数)
	//
	int i;
	if ( option & MUCOM_STATUS_SNDDRV ) {
		return vm->GetDriverStatus(option & (MUCOM_STATUS_SNDDRV-1) );
	}

	switch (option) {
	case MUCOM_STATUS_PLAYING:
		if (playflag) return 1;
		break;
	case MUCOM_STATUS_INTCOUNT:
		return vm->GetIntCount();
	case MUCOM_STATUS_PASSTICK:
		return vm->GetMasterCount();
	case MUCOM_STATUS_MAJORVER:
		return MAJORVER;
	case MUCOM_STATUS_MINORVER:
		return MINORVER;

	case MUCOM_STATUS_COUNT:
		i = vm->GetIntCount();
		if (maxcount) {
			i = i % maxcount;
		}
		return i;
	case MUCOM_STATUS_MAXCOUNT:
		return maxcount;
	case MUCOM_STATUS_MUBSIZE:
		return mubsize;
	case MUCOM_STATUS_MUBRATE:
		return original_mode ? (mubsize * 100 / MUCOM_MUBSIZE_MAX) : (mubsize * 100 / MUCOM_EM_MUBSIZE_MAX);
	case MUCOM_STATUS_BASICSIZE:
		return basicsize;
	case MUCOM_STATUS_BASICRATE:
		return original_mode ? (basicsize * 100 / MUCOM_BASICSIZE_MAX) : (basicsize * 100 / MUCOM_EM_BASICSIZE_MAX);
	case MUCOM_STATUS_AUDIOMS:
		return vm->GetAudioOutputMs();
	default:
		break;
	}
	return 0;
}


/*------------------------------------------------------------*/
/*
FM Voice file support
*/
/*------------------------------------------------------------*/

void CMucom::InitFMVoice(unsigned char *voice)
{
	//	FM音色データを初期化する
	//
	fmvoice_mode = MUCOM_FMVOICE_MODE_EXTERNAL;
	fmvoice_original = fmvoice_internal;
	StoreFMVoice(voice);
	voicefilename.clear();
	tempfilename.clear();
}


int CMucom::SaveFMVoice(bool sw)
{
	//	FM音色データを保存する
	//	(sw true=保存、false=保存せずに一時ファイルを消去)
	//
	int res;
	char curdir[MUCOM_FILE_MAXSTR];

	if (fmvoice_mode == MUCOM_FMVOICE_MODE_EXTERNAL) return 0;	// 一時ファイルがなければ保存の必要なし
	if (voicefilename.empty()) return -1;

	*curdir = 0;
	vm->GetDirectory(curdir, MUCOM_FILE_MAXSTR);			// 現在のディレクトリ
	vm->ChangeDirectory(voice_pathname.c_str());

	//	保存
	res = 0;
	if (sw) {
		SendFMVoiceMemory((unsigned char*)fmvoice_internal, 0, MUCOM_FMVOICE_SIZE);
		res = vm->SaveToFile(voicefilename.c_str(), (unsigned char*)fmvoice_internal, MUCOM_FMVOICE_SIZE);
		//Alertf("[%s]%s", voice_pathname.c_str(), voicefilename.c_str());
	}

	//	FM音色データの一時ファイルを破棄する
	if (tempfilename.empty() == false) {
		if (res == 0) {
			vm->KillFile(tempfilename.c_str());
			tempfilename.clear();
		}
	}
	vm->ChangeDirectory(curdir);
	fmvoice_mode = MUCOM_FMVOICE_MODE_EXTERNAL;
	fmvoice_original = fmvoice_internal;
	return 0;
}


void CMucom::StoreFMVoice(unsigned char *voice)
{
	//	FM音色データをメモリに転送する
	//
	if (voice == NULL) return;
	SendFMVoiceMemory(voice, 0, MUCOM_FMVOICE_SIZE);
	memcpy(fmvoice_internal, voice, MUCOM_FMVOICE_SIZE);
}


int CMucom::LoadFMVoice(const char *fname, bool sw)
{
	//		FM音色データ読み込み
	//		fname = FM音色データファイル (空文字はMUCOM_DEFAULT_VOICEFILE)
	//		sw = trueの場合は強制的に読み込む、falseの場合は仮ファイルを検索する
	//		(戻り値が0以外の場合はエラー)
	//
	int voicesize;
	char *voicedata;
	char *voicedata_org;
	char dirname[MUCOM_FILE_MAXSTR];

	voicefilename.clear();
	voice_pathname.clear();
	tempfilename.clear();

	fmvoice_mode = MUCOM_FMVOICE_MODE_EXTERNAL;
	if (*fname != 0) {
		voicefilename = std::string(fname);
	}
	else {
		voicefilename = std::string(MUCOM_DEFAULT_VOICEFILE);
	}

	tempfilename = voicefilename + "_tmp";
	*dirname = 0;
	vm->GetDirectory(dirname, MUCOM_FILE_MAXSTR);
	voice_pathname = std::string(dirname);

	voicedata = NULL;
	voicedata_org = NULL;

	if (fmvoice_original != fmvoice_internal) {
		vm->LoadAllocFree((char *)fmvoice_original);		// オリジナルデータがあれば破棄する
		fmvoice_original = fmvoice_internal;
	}

	voicedata_org = vm->LoadAlloc(voicefilename.c_str(), &voicesize);
	if (voicedata_org == NULL) {
		PRINTF("#Voice file not found [%s].\r\n", fname);
		return -1;
	}
	fmvoice_original = (MUCOM88_VOICEFORMAT *)voicedata_org;

	if (sw == false) {
		voicedata = vm->LoadAlloc(tempfilename.c_str(), &voicesize);
	}

	if (voicedata == NULL) {
		// オリジナルファイルのみの場合
		StoreFMVoice((unsigned char *)voicedata_org);
		//Alertf("[%s]%s", voice_pathname.c_str(), voicefilename.c_str());
	}
	else {
		// 一時ファイルがあった場合
		StoreFMVoice((unsigned char *)voicedata);
		vm->LoadAllocFree(voicedata);
		fmvoice_mode = MUCOM_FMVOICE_MODE_INTERNAL;
		//Alertf("[%s]%s", voice_pathname.c_str(), tempfilename.c_str());
	}
	return 0;
}


MUCOM88_VOICEFORMAT *CMucom::GetFMVoice(int no)
{
	//	内部保存された音色データを取得する
	//	(no=音色番号0～255)
	//
	if ((no < 0) || (no >= MUCOM_FMVOICE_MAXNO)) return NULL;
	return &fmvoice_internal[no];
}


int CMucom::UpdateFMVoice(int no, MUCOM88_VOICEFORMAT *voice)
{
	//	音色データを更新する
	//	(no=音色番号0～255)
	//
	char curdir[MUCOM_FILE_MAXSTR];

	if ((no < 0) || (no >= MUCOM_FMVOICE_MAXNO)) return -1;

	fmvoice_internal[no] = *voice;

	*curdir = 0;
	if (voicefilename.empty()) return -1;


	vm->GetDirectory(curdir, MUCOM_FILE_MAXSTR);			// 現在のディレクトリ
	vm->ChangeDirectory(voice_pathname.c_str());

	if (tempfilename.empty() == false) {
		fmvoice_mode = MUCOM_FMVOICE_MODE_INTERNAL;
		SendFMVoiceMemory((unsigned char*)fmvoice_internal, 0, MUCOM_FMVOICE_SIZE);
		return vm->SaveToFile((char*)tempfilename.c_str(), (unsigned char*)fmvoice_internal, MUCOM_FMVOICE_SIZE);
	}
	vm->ChangeDirectory(curdir);

	return 0;
}


char *CMucom::DumpFMVoiceAll(void)
{
	//	使用中のFM音色をダンプする
	//
	int i, max;
	vm->ResetMessageBuffer();
	max = GetUseVoiceMax();
	for (i = 0; i < max; i++) {
		DumpFMVoice(GetUseVoiceNum(i));
	}
	return (char *)GetMessageBuffer();
}


void CMucom::DumpFMVoice(int no)
{
	//	音色データを表示する
	//	(no=音色番号1～255)
	//
	unsigned char name[8];
	unsigned char *p;
	unsigned char a1;
	MUCOM88_VOICEFORMAT *v = GetFMVoice(no);
	if (v==NULL) return;

	p = (unsigned char *)v->name;
	int len = 0;
	while (1) {
		if (len >= 6) break;
		a1 = *p++;
		if (a1 < 32) a1 = 32;		// コントロールコードはスペースに変換
		if (a1 >= 128) a1 = 32;		// 半角文字はスペースに変換
		name[len++] = a1;
	}
	name[len++] = 0;

	PRINTF("  @%d:{\r\n", no);
	PRINTF("  %d, %d\r\n", v->fb, v->al);
	/*
	  AR,DR,SR,RR,SL,TL,KS,ML,DT
	*/
	PRINTF("  %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d\r\n", v->ar_op1, v->dr_op1, v->sr_op1, v->rr_op1, v->sl_op1, v->tl_op1, v->ks_op1, v->ml_op1, v->dt_op1);
	PRINTF("  %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d\r\n", v->ar_op2, v->dr_op2, v->sr_op2, v->rr_op2, v->sl_op2, v->tl_op2, v->ks_op2, v->ml_op2, v->dt_op2);
	PRINTF("  %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d\r\n", v->ar_op3, v->dr_op3, v->sr_op3, v->rr_op3, v->sl_op3, v->tl_op3, v->ks_op3, v->ml_op3, v->dt_op3);
	PRINTF("  %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, %02d, \"%s\"}\r\n", v->ar_op4, v->dr_op4, v->sr_op4, v->rr_op4, v->sl_op4, v->tl_op4, v->ks_op4, v->ml_op4, v->dt_op4,name);
}

// 曲データのアドレス
int CMucom::GetSongAddress() {
	return original_mode ? MUCOM_ADDRESS_SONG : MUCOM_ADDRESS_EM_SONG;
}

int CMucom::StoreFMVoiceFromEmbed(void)
{
	//	演奏データ(MUB)が持つ音色データを内部保存データに反映させる
	//	(VMの$c200から演奏データを読み込んでおくこと)
	//		voicelist = 出力される音色番号のリスト
	//		返値 = voicelistに出力された音色番号の数
	//
	int i,j,no;
	char *v;
	int vmmem;

	int song_address = GetSongAddress();

	ChangeMemoryToSong();
	int vdata = vm->Peekw(song_address + 1) + song_address;
	int result = vm->Peek(vdata++);
	RestoreMemory();

	fmvoice_usemax = result;

	for (i = 0; i < result; i++) {
		no = (int)hedmusic->ext_fmvoice[i] - 1;
		fmvoice_use[i] = (unsigned char)no;
		v = (char *)GetFMVoice(no);
		vmmem = no * sizeof(MUCOM88_VOICEFORMAT);
		SendFMVoiceMemory( (unsigned char *)v, vmmem, sizeof(MUCOM88_VOICEFORMAT));
		v++;
		for (j = 0; j < 25; j++) {			// 25byteの音色データをコピー
			*v++ = vm->Peek(vdata++);
		}
	}

	return result;
}


int CMucom::SendFMVoiceMemory(const unsigned char* src, int offset, int size)
{
	if (original_mode) {
		vm->SendMem(src, MUCOM_FMVOICE_ADR + offset, size);
		return 0;
	}
	
	vm->SendExtMem(src, 1, MUCOM_FMVOICE_ADR + offset, size);	
	return 0;
}



int CMucom::LoadFMVoiceFromTAG(void)
{
	//	(#voice)タグ情報から音色データをロードする(音色エディタ用の準備)
	//

	//	voiceファイルをロードしておく
	char voicefile[MUCOM_FILE_MAXSTR];
	strncpy(voicefile, GetInfoBufferByName("voice"), MUCOM_FILE_MAXSTR);
	if (voicefile[0]) {
		LoadFMVoice(voicefile);
	}

	return 0;
}

void CMucom::EnableBreakPoint(uint16_t adr)
{
	vm->EnableBreakPoint(adr);
}

void CMucom::DisableBreakPoint()
{
	vm->DisableBreakPoint();
}

void CMucom::DebugRun()
{
	vm->DebugRun();
}

void CMucom::DebugInstExec()
{
	vm->DebugInstExec();
}

void CMucom::DebugPause()
{
	vm->DebugPause();
}


void CMucom::GetRegSet(RegSet* reg)
{
	vm->GetRegSet(reg);
}



/*------------------------------------------------------------*/
/*
Editor support
*/
/*------------------------------------------------------------*/

void CMucom::EditorReset(const char *mml, int option)
{
	//		エディタ編集項目のリセット
	//		(mml=編集読み込み時のMMLバッファ内容/NULLの場合は空になる)
	//		(option=MUCOM_EDIT_OPTION_*)
	//
	edit_status = MUCOM_EDIT_STATUS_NONE;
	edit_option = option;
	edit_notice = 0;
	edit_master.clear();
	edit_filename.clear();
	edit_pathname.clear();
	edit_request.clear();
	if (mml) {
		edit_master = std::string(mml);
		edit_status = MUCOM_EDIT_STATUS_SAVED;
	}
	edit_buffer = (char *)edit_master.c_str();
}


void CMucom::EditorSetFileName(const char *filename, const char *pathname,bool sessionstart)
{
	//		エディタ編集ファイル名・パスを設定する
	//		(EditorReset後に設定すること)
	//		(filename+pathnameが保存されるファイルになります)
	//		(sessionstart=true時はファイルの内容をチェックします)
	//
	if (filename) edit_filename = std::string(filename);
	if (pathname) edit_pathname = std::string(pathname);
	if (sessionstart) {
		int res;
		res = ProcessFile(filename);
		if (res == 0) {
			NoticePlugins(MUCOM88IF_NOTICE_SESSION);
		}
	}
}


int CMucom::CheckEditorUpdate(void)
{
	//		エディタ編集項目が修正されているか?
	//		return 0=未初期化、1=保存済み、2=編集中
	//
	char *p = (char *)edit_master.c_str();
	if (edit_status == MUCOM_EDIT_STATUS_NONE) return MUCOM_EDIT_STATUS_NONE;
	if (strcmp(p, edit_buffer) == 0) {
		edit_status = MUCOM_EDIT_STATUS_SAVED;
		if (fmvoice_mode == MUCOM_FMVOICE_MODE_INTERNAL) {
			edit_status = MUCOM_EDIT_STATUS_VOICEEDIT;
		}
	}
	else {
		edit_status = MUCOM_EDIT_STATUS_CHANGED;
	}

	return edit_status;
}


int CMucom::SaveEditorMML(const char *filename)
{
	//		エディタ編集内容を保存する
	//		(filenameが""の場合はデフォルト名、指定された場合はその名前で保存する)
	//
	SaveFMVoice();
	//
	if (*filename != 0) {
		return vm->SaveToFile(filename, (unsigned char *)edit_buffer, strlen(edit_buffer));
	}
	edit_master = std::string(edit_buffer);
	edit_status = MUCOM_EDIT_STATUS_SAVED;
	std::string savename = edit_pathname + edit_filename;
	return vm->SaveToFile(savename.c_str(), (unsigned char *)edit_buffer, strlen(edit_buffer));
}


int CMucom::UpdateEditorMML(const char *mml)
{
	//		エディタ編集内容を更新する
	//		(外部のエディタでMMLが変化したことを通知します)
	//		(mmlのポインタのみ更新します、内容は外部で保持されている必要があります)
	//
	edit_buffer = (char *)mml;
	edit_status = MUCOM_EDIT_STATUS_CHANGED;
	return 0;
}


int CMucom::GetEditorPosToLine(int pos)
{
	//		エディタのカーソル位置から行数を得る
	//
	int line = 1;
	int myline = 1;
	int cur;
	const char *src = edit_buffer;

	while (1) {
		if (src == NULL) break;
		cur = src - edit_buffer;
		if (cur<=pos) myline = line;
		src = GetTextLine(src);
		line++;
	}
	return myline;
}


int CMucom::RequestEditorMML(const char *mml)
{
	//		エディタのMMLファイルを外部から更新するリクエストを出す
	//		(リクエストをエディタ側が取得しない限り更新されません)
	//
	edit_notice |= MUCOM_NOTICE_MMLCHANGE;
	edit_request = std::string(mml);
	return 0;
}


const char *CMucom::GetRequestMML(void)
{
	//		外部リクエストによるMMLを取得する
	//		(リクエストがない場合はNULLを返す)
	//
	if (edit_notice & MUCOM_NOTICE_MMLCHANGE) {
		edit_notice ^= MUCOM_NOTICE_MMLCHANGE;
		return edit_request.c_str();
	}
	return NULL;
}


int CMucom::UpdateEditor(void)
{
	//		エディタ更新
	//
	return edit_notice;
}


/*------------------------------------------------------------*/
/*
Compiler support
*/
/*------------------------------------------------------------*/

int CMucom::GetMultibyteCharacter(const unsigned char *text)
{
	//		マルチバイト文字のサイズを得る
	//
	const unsigned char *p = text;
	unsigned char a1;
	int mulchr = 1;

	a1 = *p;

#ifndef MUCOM88UTF8
	if (a1 >= 129) {				// 全角文字チェック(SJIS)
		if ((a1 <= 159) || (a1 >= 224)) {
			mulchr++;
		}
	}
#endif

#ifdef MUCOM88UTF8
	if (a1 & 0x80) {				// 全角文字チェック(UTF8)
		int utf8bytes = 0;
		if ((a1 & 0xe0) == 0x0c0) utf8bytes = 1;
		if ((a1 & 0xf0) == 0x0e0) utf8bytes = 2;
		if ((a1 & 0xf8) == 0x0f0) utf8bytes = 3;

		int utf8cnt = 0;
		while(utf8bytes > 0) {
			if ((*(++p) & 0xc0) != 0x80) break;
			utf8cnt++;
			utf8bytes--;
		}
		mulchr += utf8cnt;
	}
#endif

	return mulchr;
}


const char *CMucom::GetTextLine(const char *text)
{
	//	1行分のデータを格納
	//
	const unsigned char *p = (const unsigned char *)text;
	unsigned char a1;
	int mptr = 0;
	int mulchr;

	a1 = *p;
	if ((a1 >= '0') && (a1 <= '9')) {	// 行番号付きの場合は'までスキップ
		while (1) {
			a1 = *p;
			if (a1 == 0) break;
			if (a1 == 0x27) { p++; break; }
			p+= GetMultibyteCharacter(p);
		}
	}

	while (1) {
		a1 = *p;
		if (a1 == 0) {
			p = NULL;  break;			// End of text
		}
		if (a1 == 10) {					// LF
			p++;
			break;
		}
		if (a1 == 13) {
			p++;
			if (*p == 10) p++;
			break;						// CR/LF
		}
		if (a1 == 9) {					// TAB->space
			a1 = 32;
			linebuf[mptr++] = a1;
			p++;
			continue;
		}

		mulchr = GetMultibyteCharacter(p);
		while (mulchr>0) {
			linebuf[mptr++] = *p++;
			mulchr--;
		}

	}
	linebuf[mptr++] = 0;
	return (const char *)p;
}


const char *CMucom::GetInfoBuffer(void)
{
	if (infobuf == NULL) return "";
	return infobuf->GetBuffer();
}


const char *CMucom::GetInfoBufferByName(const char *name)
{
	//		infobuf内の指定タグ項目を取得
	//		name = タグ名(英小文字)
	//		(結果が""の場合は該当項目なし)
	//
	int len;

	if (infobuf == NULL) return "";

	const char *src = GetInfoBuffer();
	while (1) {
		if (src == NULL) break;
		src = GetTextLine(src);

		len = strpick_spc((char *)linebuf+1, infoname, 63);
		if (len > 0) {
			//printf("[%s]\n", infoname);
			if (strcmp(name, infoname) == 0) {
				return (char *)(linebuf+1+len+1);
			}
		}
	}
	return "";
}


void CMucom::DeleteInfoBuffer(void)
{
	if (infobuf) {
		delete infobuf; infobuf = NULL;
	}
}


void CMucom::PrintInfoBuffer(void)
{
	//		infobufの内容を出力
	//
	PRINTF("%s", GetInfoBuffer());
}


int CMucom::ProcessFile(const char *fname)
{
	//		MUCOM88 MMLソースファイル内のタグを処理
	//		fname = MML書式のテキストデータファイル(UTF8)
	//		(結果は、infobufに入ります)
	//		(戻り値が0以外の場合はエラー)
	//
	int sz;
	char *mml;
	DeleteInfoBuffer();
	if (*fname == 0) {
		mml = GetEditorMML();
	}
	else {
		mml = vm->LoadAlloc(fname, &sz);
	}
	if (mml == NULL) {
		PRINTF("#File not found [%s].\r\n", fname);
		return -1;
	}
	ProcessHeader( mml );
	if (*fname != 0) {
		vm->LoadAllocFree(mml);
	}
	return 0;
}


bool CMucom::hasMacro(char *text)
{
	//		MMLソースの行にマクロ定義(*nn{})が含まれるか調べる
	//		(タグ定義との切り分けをする)
	//		(戻り値がtrueの場合はマクロ定義)
	//
	const unsigned char *p = (const unsigned char *)text;
	unsigned char a1;
	int mulchr;
	bool flag_marco = false;

	if (p[0] != '#') return flag_marco;

	while (1) {
		a1 = *p;
		if (a1 == 0) {
			return flag_marco;
		}
		if ( a1=='*' ) {
			p++;
			break;
		}
		mulchr = GetMultibyteCharacter(p);
		while (mulchr > 0) {
			p++;
			mulchr--;
		}
	}
	a1 = *p++;
	if ((a1 >= '0') && (a1 <= '9')) flag_marco = true;
	return flag_marco;
}


int CMucom::ProcessHeader(char *text)
{
	//		MUCOM88 MMLソース内のタグを処理
	//		text         = MML書式のテキストデータ(UTF8)(終端=0)
	//		(戻り値が0以外の場合はエラー)
	//
	//		#mucom88 1.5
	//		#voice voice.dat
	//		#pcm mucompcm.bin
	//		#composer name
	//		#author name
	//		#title name
	//		#date xxxx/xx/xx
	//		#comment ----
	//		#url ----
	//
	const char *src = text;
	DeleteInfoBuffer();
	infobuf = new CMemBuf();
	while (1) {
		if (src == NULL) break;
		src = GetTextLine(src);
		if (linebuf[0] == '#'){							// タグか?
			if (hasMacro((char *)linebuf)==false) {		// マクロか?
				//printf("[%s]\n", linebuf);
				infobuf->PutStr((char *)linebuf);
				infobuf->PutCR();
			}
		}
	}
	infobuf->Put((char)0);
	return 0;
}


unsigned char CMucom::ConvertUTF8Kana(unsigned char *ptr)
{
	//		UTF8の文字列ポインタが半角カナだった場合、SJISコードを返す
	//		(半角カナでなかった場合は0を返す)
	//
	unsigned char res;
	unsigned int utf8code;
	const unsigned int *kanacode;

	utf8code = (ptr[0]<<16)| (ptr[1]<<8)| (ptr[2]);
	kanacode = cnv_utf8kana;
	res = 0xa1;
	while (1) {
		if ( *kanacode == utf8code ) return res;
		res++;
		kanacode++;
	}
	return 0;
}

int CMucom::StoreBasicSource(char *text, int line, int add)
{
	//	BASICソースの形式でリストを作成
	//
	const char *src = text;
	int ln = line;
	int mptr = 1;
	int linkptr;
	int mulchr;
	int i;
	unsigned char a1;

	while (1) {
		if (src == NULL) break;

		linkptr = mptr;
		mptr += 2;
		vm->Pokew(mptr,ln);
		mptr += 2;
		vm->Poke(mptr, 0x3a);
		vm->Poke(mptr+1, 0x8f);
		vm->Poke(mptr+2, 0xe9);
		mptr += 3;

		src = GetTextLine(src);
		i = 0;
		while (1) {
			a1 = linebuf[i];

			if (a1 == ';') {
				while(a1 != 0) {
					a1 = linebuf[++i];
				}
			}

			if (a1 == 0) {
				vm->Poke(mptr++, 0);
				break;
			}

			mulchr = GetMultibyteCharacter(linebuf+i);
			if (mulchr == 1) {
				vm->Poke(mptr++, a1);	// 半角の文字のみ登録する
			}
#ifdef MUCOM88UTF8
			if (mulchr == 3) {
				a1 = ConvertUTF8Kana(linebuf+i);
				if (a1 != 0) {
					vm->Poke(mptr++, a1);	// 半角カナを登録する
				}
			}
#endif
			i += mulchr;
		}
		vm->Pokew(linkptr, mptr);		// 次のポインタを保存する
		ln += add;
	}

	vm->Pokew(mptr, 0);					// End pointer
	return mptr;
}

//		MUCOM88コンパイル(Resetが必要)
//		text         = MML書式のテキストデータ(UTF8)(終端=0)
//		filename     = 出力される音楽データファイル
//		option : 1   = #タグによるvoice設定を無視
//		         2   = PCM埋め込みをスキップ
//		         4   = 音色の一時バッファ作成を許可する
//		         8   = 演奏バッファ#0に直接保存する
//		(戻り値が0以外の場合はエラー)
//
int CMucom::Compile(char *text, const char *filename, int option)
{
	return Compile(text, option, true, filename);
}

// writeMub = mub出力
int CMucom::Compile(char *text, int option, bool writeMub, const char *filename)
{

	int i, res;
	int start, length;
	char* adr_start;
	char* adr_length;
	int workadr;
	int pcmflag;

	maxch = MUCOM_MAXCH;				// ワークの取得が難しいため固定値

	AddExtraInfo(text);

	//		voiceタグの解析
	if ((option & MUCOM_COMPILE_IGNOREVOICE) == 0) {
		char voicefile[MUCOM_FILE_MAXSTR];
		strncpy(voicefile, GetInfoBufferByName("voice"), MUCOM_FILE_MAXSTR);
		if (voicefile[0]) {
			LoadFMVoice(voicefile);
		}
	}

	NoticePlugins(MUCOM88IF_NOTICE_MMLSEND);

	basicsize = StoreBasicSource(text, 1, 1);

	InitCompiler();
	int vec = vm->Peekw(MUCOM_ADDRESS_POLL_VECTOR);

	if (octreverse_mode) {
		//	#invert onタグがあった場合はpoll iを実行する
		res = vm->CallAndHalt2(vec, 'I');
	}
	
	PRINTF("#poll a $%x.\r\n", vec);

	int loopst = 0xf25a;
	vm->Pokew(loopst, 0x0101);		// ループ情報スタックを初期化する(ループ外の'/'でエラーを出すため)

	res = vm->CallAndHalt2(vec, 'A');
	if (res) {
		int line = vm->Peekw(0x0f32e);
		int msgid = vm->GetMessageId();
		if (msgid > 0) {
			PRINTF_NOCONV("#error %d in line %d.\r\n-> %s (%s)\r\n", msgid, line, mucom_geterror_j(msgid), mucom_geterror(msgid));
		}
		else {
			PRINTF("#unknown error in line %d.\r\n", line);
		}
		return -1;
	}

	char stmp[128];
	vm->PeekToStr(stmp, 0xf3c8, 80);		// 画面最上段のメッセージ
	if (original_mode) {
		PRINTF("%s\r\n", stmp);
	} else {
		PutMucomHeader(stmp);
	}

	workadr = 0xf320;
	fmvoice = vm->Peek(workadr + 50);
	pcmflag = 0;
	maxcount = 0;
	mubsize = 0;

	jumpcount = vm->Peekw(MUCOM_ADDRESS_JCLOCK);		// JCLOCKの値(Jコマンドのタグ位置)
	jumpline = vm->Peekw(MUCOM_ADDRESS_JPLINE);		// JPLINEの値(Jコマンドの行番号)

	PRINTF("Used FM voice:%d", fmvoice);

	if (jumpcount > 0) {
		PRINTF("  Jump to line:%d", jumpline);
	}

	PRINTF("\r\n");

	bool badvoice = false;
	for (i = 0; i < fmvoice; i++) {
		unsigned char voiceid = (unsigned char)vm->Peek(MUCOM_ADDRESS_DEFVOICE + i);
		//PRINTF("#VOICE %d.\r\n", (int)voiceid);
		if (voiceid <= 1) badvoice = true;
	}
	if (badvoice) {
		PRINTF_NOCONV("#Abort: bad voice No. detected.\r\n-> 音色番号 @0 は使用できません。\r\n");
		return -1;
	}

	PRINTF("[ Total count ]\r\n");

	for (i = 0; i < maxch; i++) {
		int tcnt, lcnt;
		tcnt = vm->Peekw(0x8c10 + i * 4);
		lcnt = vm->Peekw(0x8c12 + i * 4);
		if (lcnt) { lcnt = tcnt - (lcnt - 1); }
		if (tcnt > maxcount) maxcount = tcnt;
		tcount[i] = tcnt;
		lcount[i] = lcnt;
		PRINTF("%c:%d ", 'A' + i, tcount[i]);
	}
	PRINTF("\r\n");

	PRINTF("[ Loop count  ]\r\n");

	for (i = 0; i < maxch; i++) {
		PRINTF("%c:%d ", 'A' + i, lcount[i]);
	}
	PRINTF("\r\n");

	adr_start = stmp + 31;
	adr_start[4] = 0;
	start = htoi(adr_start);

	adr_length = stmp + 41;
	adr_length[4] = 0;
	length = htoi(adr_length);

	mubsize = length;

	PRINTF("#Data Buffer $%04x-$%04x ($%04x)\r\n", start, start + length - 1, length);
	PRINTF("#MaxCount:%d Basic:%04x(%d%%) Data:%04x(%d%%)\r\n", maxcount, basicsize, GetStatus(MUCOM_STATUS_BASICRATE), mubsize, GetStatus(MUCOM_STATUS_MUBRATE));

	if (tcount[maxch - 1] == 0) pcmflag = 2;	// PCM chが使われてなければPCM埋め込みはスキップ

	NoticePlugins(MUCOM88IF_NOTICE_COMPEND);

	return !writeMub ? 0 : SaveMusic(filename, start, length, option | pcmflag);
}

void CMucom::PutMucomHeader(const char *stmp)
{
	const char* mpos = strstr(stmp, "MUCOM88");

	if (mpos == NULL) {
		PRINTF("%s\r\n", stmp);
		return;
	}

	PRINTF("[  %s\r\n", mpos);
}

void CMucom::InitCompiler()
{
	vm->DebugEnable();

	// オリジナル
	if (original_mode) {
		vm->CallAndHalt(MUCOM_ADDRESS_CINT); // CINT コンパイラ初期化
		compiler_initialized = true;
		return;
	}

	vm->CallAndHalt(MUCOM_ADDRESS_EM_CINT); // CINT コンパイラ初期化
}


int CMucom::CompileFile(const char *fname, const char *sname, int option)
{
	//		MUCOM88コンパイル(Resetが必要)
	//		fname     = MML書式のテキストファイル(UTF8)
	//		sname     = 出力される音楽データファイル
	//		option : 1   = #タグによるvoice設定を無視
	//		(戻り値が0以外の場合はエラー)
	//
	int res;
	int sz;
	char *mml;
	PRINTF("#Compile[%s] to %s.\r\n", fname, sname);
	mml = vm->LoadAlloc(fname, &sz);
	if (mml == NULL) {
		PRINTF("#File not found [%s].\r\n", fname);
		return -1;
	}

	ProcessHeader(mml);

	res = Compile(mml, sname, option);
	vm->LoadAllocFree(mml);

	if (res) return res;

	return 0;
}

int CMucom::CompileMemory(const char* fname, int option)
{
	//		MUCOM88メモリコンパイル(Resetが必要)
	//		fname     = MML書式のテキストファイル(UTF8)
	//		option : 1   = #タグによるvoice設定を無視
	//		(戻り値が0以外の場合はエラー)
	//
	int res;
	int sz;
	char* mml;
	PRINTF("#Compile[%s].\r\n", fname);
	mml = vm->LoadAlloc(fname, &sz);
	if (mml == NULL) {
		PRINTF("#File not found [%s].\r\n", fname);
		return -1;
	}

	ProcessHeader(mml);

	res = Compile(mml, option);
	vm->LoadAllocFree(mml);

	if (res) return res;

	return 0;
}



int CMucom::CompileMem(char *mem, int option)
{
	//		MUCOM88コンパイル(Resetが必要)
	//		fname     = MMLデータ
	//		option : 1   = #タグによるvoice設定を無視
	//		(戻り値が0以外の場合はエラー)
	//
	int res;
	PRINTF("#Compile from mem.\r\n");
	if (mem == NULL) {
		PRINTF("#No data.\r\n");
		return -1;
	}
	ProcessHeader(mem);
	res = Compile(mem, "mem", option| MUCOM_COMPILE_TO_MUSBUFFER);
	if (res) return res;
	return 0;
}


int CMucom::GetDriverMode(char *fname)
{
	//		MUCOM88 #driverタグ文字列からドライバーモードを取得する(MMLファイルから取得)
	//		fname      = MMLファイル名
	//		(戻り値がマイナスの場合はエラー)
	//		(正常な場合は、MUCOM_DRIVER_* の値が返る)
	//
	int res;
	int sz;
	char *mml;
	if (fname == NULL) return -1;
	mml = vm->LoadAlloc(fname, &sz);
	if (mml == NULL) {
		PRINTF("#File not found [%s].\r\n", fname);
		return -1;
	}
	res = GetDriverModeMem(mml);
	vm->LoadAllocFree(mml);
	return res;
}


int CMucom::GetDriverModeMUB(char *fname)
{
	//		MUCOM88 #driverタグ文字列からドライバーモードを取得する(MUBファイルから取得)
	//		fname      = MUBファイル名
	//		(戻り値がマイナスの場合はエラー)
	//		(正常な場合は、MUCOM_DRIVER_* の値が返る)
	//
	int res;
	if (fname == NULL) return -1;

	res = LoadMusic(fname,0);
	if (res) {
		PRINTF("#File not found [%s].\r\n", fname);
		return -1;
	}
	LoadTagFromMusic(0);

	res = GetDriverModeMem(NULL);
	return res;
}


int CMucom::GetDriverModeMem(char *mem)
{
	//		MUCOM88 #driverタグ文字列からドライバーモードを取得する(MMLからダイレクトに取得)
	//				#octaveタグ文字列からオクターブモードを反映させる
	//		mem     = MMLデータ
	//		(戻り値がマイナスの場合はエラー)
	//		(正常な場合は、MUCOM_DRIVER_* の値が返る)
	//
	int res;
	const char *driver;
	const char *octave;
	if (mem) {
		res = ProcessHeader(mem);
		if (res) return res;
	}

	//		#octaveタグを解析する
	//		-> octreverse_modeに反映させる
	octave = GetInfoBufferByName("invert");
	if (STRCASECMP(octave, "on") == 0) {
		octreverse_mode = true;
	}
	else {
		octreverse_mode = false;
	}

	driver = GetInfoBufferByName("driver");
	res = GetDriverModeString(driver);
	return res;
}


int CMucom::GetDriverModeString(const char* name)
{
	//		MUCOM88 #driverタグ文字列からドライバーモードを取得する(文字列をダイレクトに指定)
	//		name     = タグ文字列
	//		(戻り値がマイナスの場合はエラー)
	//		(正常な場合は、MUCOM_DRIVER_* の値が返る)
	//
	int res = MUCOM_DRIVER_UNKNOWN;

	if (*name == 0) res = MUCOM_DRIVER_NONE;
	if (STRCASECMP(name, "mucom88") == 0) res = MUCOM_DRIVER_MUCOM88;
	if (STRCASECMP(name, "mucom88e") == 0) res = MUCOM_DRIVER_MUCOM88E;
	if (STRCASECMP(name, "mucom88em") == 0) res = MUCOM_DRIVER_MUCOM88EM;
	if (STRCASECMP(name, "mucomdotnet") == 0) res = MUCOM_DRIVER_MUCOMDOTNET;
	return res;
}


void CMucom::SetDriverMode(int driver)
{
	//		ドライバーモード(MUCOM_DRIVER_*)から適切な設定を行う
	//		(original_mode,original_verが設定される)
	//
	bool orig = true;
	int ver = MUCOM_ORIGINAL_VER_17;
	switch (driver) {
	case MUCOM_DRIVER_MUCOM88EM:
		orig = false;
		break;
	case MUCOM_DRIVER_MUCOM88E:
		ver = MUCOM_ORIGINAL_VER_15;
		break;
	case MUCOM_DRIVER_MUCOMDOTNET:
	case MUCOM_DRIVER_UNKNOWN:
	default:
		break;
	}
	original_mode = orig;
	original_ver = ver;
}


void CMucom::AddExtraInfo(char *mmlsource)
{
	//	infobufに追加情報を書き込む
	//
	char mml_md5[128];
	if (infobuf == NULL) return;

	GetMD5(mml_md5, mmlsource, strlen(mmlsource));
	infobuf->PutStr("#mucver " VERSION );
	infobuf->PutCR();
	infobuf->PutStr("#mmlhash ");
	infobuf->PutStr(mml_md5);
	infobuf->PutCR();
	if (user_uuid[0] != 0) {
		infobuf->PutStr("#uuid ");
		infobuf->PutStr(user_uuid);
		infobuf->PutCR();
	}
}


int CMucom::SaveMusic(const char *fname,int start, int length, int option)
{
	//		音楽データファイルを出力(コンパイルが必要)
	//		filename     = 出力される音楽データファイル
	//		option : 1   = #タグによるvoice設定を無視
	//		         2   = PCM埋め込みをスキップ
	//		         8   = メモリバッファに結果を出力
	//		(戻り値が0以外の場合はエラー)
	//
	int res;
	MUBHED hed;
	char *header;
	char *footer;
	char *pcmdata;
	const char *pcmname;
	int hedsize;
	int footsize;
	int pcmsize;
	int i;
	if (fname == NULL) return -1;

	header = (char *)&hed;
	hedsize = sizeof(MUBHED);
	memset(header,0,hedsize);
	header[0] = 'M';
	header[1] = 'U';
	header[2] = 'B';	// 'C'=1.0 Header, 'B'=2.0 Header
	header[3] = '8';

	footer = NULL;
	pcmdata = NULL;
	footsize = 0;
	pcmsize = 0;
	if (infobuf) {
		infobuf->Put((int)0);
		footer = infobuf->GetBuffer();
		footsize = infobuf->GetSize();
	}

	hed.dataoffset = hedsize;
	hed.datasize = length;
	hed.tagdata = hedsize + length;
	hed.tagsize = footsize;
	hed.jumpcount = jumpcount;
	hed.jumpline = jumpline;

	//	2.0 Header option
#ifdef MUCOM88UTF8
	hed.ext_flags = MUCOM_FLAG_UTF8TAG;
#else
	hed.ext_flags = MUCOM_FLAG_SJISTAG;
#endif	

	hed.ext_system = MUCOM_SYSTEM_PC88;
	hed.ext_target = MUCOM_TARGET_YM2608;
	hed.ext_channel_num = maxch;
	hed.ext_fmvoice_num = fmvoice;
	hed.ext_player = 0;					// not use (reserved)

	for (i = 0; i < fmvoice; i++) {
		hed.ext_fmvoice[i] = (unsigned char)vm->Peek(MUCOM_ADDRESS_DEFVOICE + i);
	}

	if ((option & MUCOM_COMPILE_IGNOREPCM) == 0) {
		pcmname = GetInfoBufferByName("pcm");
		if (pcmname[0] != 0) {
			pcmdata = vm->LoadAlloc(pcmname, &pcmsize);
			if (pcmdata != NULL) {
				hed.pcmdata = hed.tagdata + footsize;
				hed.pcmsize = pcmsize;
			}
		}
	}

	if (option & MUCOM_COMPILE_TO_MUSBUFFER) {

		int num = 0;
		CMemBuf *buf = new CMemBuf();
		ChangeMemoryToSong();
		res = vm->StoreMemExpand(buf, start, length, header, hedsize, footer, footsize, pcmdata, pcmsize);
		RestoreMemory();
		if (res) {
			PRINTF("#Memory write error.\r\n");
			return -2;
		}
		if (musbuf[num] != NULL) {
			delete musbuf[num];
		}
		musbuf[num] = buf;
		NoticePlugins(MUCOM88IF_NOTICE_LOADMUB);
	}
	else {
		ChangeMemoryToSong();
		//res = vm->SaveMem(filename, start, length);
		res = vm->SaveMemExpand(fname, start, length, header, hedsize, footer, footsize, pcmdata, pcmsize);
		RestoreMemory();

		if (res) {
			PRINTF("#File write error [%s].\r\n", fname);
			return -2;
		}
	}

	if (pcmdata != NULL) vm->LoadAllocFree(pcmdata);

	PRINTF("#Saved [%s].\r\n", fname);
	return 0;
}



int CMucom::MUBGetHeaderVersion(MUBHED *hed)
{
	char *p;
	p = (char *)hed;
	if ((p[0] == 'M') && (p[1] == 'U') && (p[2] == 'C') && (p[3] == '8')) return MUCOM_HEADER_VERSION1;
	if ((p[0] == 'M') && (p[1] == 'U') && (p[2] == 'B') && (p[3] == '8')) return MUCOM_HEADER_VERSION2;
	return -1;
}


char *CMucom::MUBGetData(MUBHED *hed, int &size)
{
	char *p;
	p = (char *)hed;
	p += hed->dataoffset;
	size = hed->datasize;
	return p;
}


char *CMucom::MUBGetTagData(MUBHED *hed, int &size)
{
	char *p;
	p = (char *)hed;
	p += hed->tagdata;
	if (hed->tagdata == 0) return NULL;
	size = hed->tagsize;
	if (size == 0) return NULL;
	return p;
}


char *CMucom::MUBGetPCMData(MUBHED *hed, int &size)
{
	char *p;
	p = (char *)hed;
	p += hed->pcmdata;
	if (hed->pcmdata == 0) return NULL;
	size = hed->pcmsize;
	if (size == 0) return NULL;
	return p;
}


int CMucom::ConvertADPCM(const char *fname, const char *sname)
{
	int res;
	res = vm->ConvertWAVtoADPCMFile(fname, sname);
	if (res < 0) return -1;
	return 0;
}


void CMucom::GetMD5(char *res, char *buffer, int size)
{
	int di;
	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16 * 2 + 1];

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)buffer, size);
	md5_finish(&state, digest);

	for (di = 0; di < 16; ++di) {
		sprintf(hex_output + di * 2, "%02x", digest[di]);
	}
	strcpy(res, hex_output);
}


void CMucom::SetVMOption(int option, int mode)
{
	switch (mode) {
	case 0:
		vm->SetOption(option);
		break;
	case 1:
		vm->SetOption( vm->GetOption() | option);
		break;
	case 2:
		vm->SetOption(vm->GetOption() & ~option);
		break;
	default:
		break;
	}
}


void CMucom::SetVolume(int fmvol, int ssgvol)
{
	vm->SetVolume(fmvol, ssgvol);
}


void CMucom::SetFastFW(int value)
{
	vm->SetFastFW(value);
}


int CMucom::GetChannelData(int ch, PCHDATA *result)
{
	//		指定されたチャンネルのリアルタイム演奏情報を取得する
	//			ch = チャンネルNo (0～maxch)(A～Kチャンネルの順)
	//			result = 結果を出力するPCHDATA形式のポインタ(あらかじめ確保が必要)
	//
	int i;
	int size;
	int *ptr;
	unsigned char *src;
	unsigned char *srcp;
	bool ready = playflag;
	if ((ch < 0) || (ch >= MUCOM_MAXCH)) ready = false;
	if (!ready){
		memset(result, 0, sizeof(PCHDATA));
		return -1;
	}

	size = MUCOM_PCHDATA_PC88_SIZE;
	src = (unsigned char *)vm->GetChData(ch);
	srcp = src;
	ptr = (int *)result;

	if (src == NULL) return -1;

	for (i = 0; i < size; i++){
		*ptr++ = (int)*src++;
	}

	result->wadr = srcp[2] + (srcp[3] << 8);
	result->tadr = srcp[4] + (srcp[5] << 8);
	result->detune = srcp[9] + (srcp[10] << 8);
	result->lfo_diff = srcp[23] + (srcp[24] << 8);

	//	Check data (チャンネルごとに値の加工が必要)
	int pan = 0;
	int v_orig = 0;
	int vol_org = 0;
	unsigned char chwork;
	switch (ch) {
	case MUCOM_CH_PSG:
	case MUCOM_CH_PSG+1:
	case MUCOM_CH_PSG+2:
		vol_org = result->volume & 15;
		pan = 3;
		break;
	case MUCOM_CH_RHYTHM:
		vol_org = result->volume;
		pan = 3;
		break;
	case MUCOM_CH_ADPCM:
		chwork = vm->GetChWork(15);
		pan = chwork & 3;
		vol_org = result->volume - 4;
		if (vol_org < 0) vol_org = 0;
		break;
	default:
		if (ch >= MUCOM_CH_FM2) {
			chwork = vm->GetChWork(8 + 4 + (ch - MUCOM_CH_FM2));
		}
		else {
			chwork = vm->GetChWork(8 + ch);
		}
		vol_org = result->volume - 4;
		if (vol_org < 0) vol_org = 0;
		pan = (int)(chwork & 0xc0);
		pan = pan >> 6;
		if (mubver == MUCOM_HEADER_VERSION2) {
			v_orig = (int)hedmusic->ext_fmvoice[result->vnum] - 1;
			if (v_orig < 0) v_orig = 0;
		}
		break;
	}
	result->pan = pan;
	result->vnum_org = v_orig;
	result->vol_org = vol_org;

	return 0;
}

/*------------------------------------------------------------*/
/*
plugin interface
*/
/*------------------------------------------------------------*/

int MUCOM88IF_VM_COMMAND(void *ifptr, int cmd, int prm1, int prm2, void *prm3, void *prm4)
{
	//		VM用プラグインコマンド
	Mucom88Plugin *plg = (Mucom88Plugin *)ifptr;
	mucomvm *vm = plg->vm;				// vmインスタンスを得る
	CMucom *mucom = plg->mucom;			// mucomインスタンスを得る
	switch (cmd) {
	case MUCOM88IF_MUCOMVM_CMD_FMWRITE:			// prm1=reg, prm2=data
		vm->FMRegDataOut( prm1, prm2 );
		return 0;
	case MUCOM88IF_MUCOMVM_CMD_FMREAD:
		break;
	case MUCOM88IF_MUCOMVM_CMD_GETCHDATA:		// prm1=ch, prm3=PCHDATA出力先
		return mucom->GetChannelData(prm1, (PCHDATA *)prm3);
	case MUCOM88IF_MUCOMVM_CMD_CHDATA:
		break;
	case MUCOM88IF_MUCOMVM_CMD_TAGDATA:			// prm3=タグ名, prm4=出力先(max255chr)
		{
		char *result= (char *)prm4;
		const char *src = (const char *)prm3;
		const char *p = mucom->GetInfoBufferByName( src );
		if (p == NULL) return -1;
		strncpy(result,p,255 );
		return 0;
		}
	case MUCOM88IF_MUCOMVM_CMD_VOICEUPDATE:		// prm1=音色No. , prm3=MUCOM88_VOICEFORMATポインタ
		{
		return mucom->UpdateFMVoice(prm1, (MUCOM88_VOICEFORMAT *)prm3);
		}
	case MUCOM88IF_MUCOMVM_CMD_VOICESAVE:		// パラメーターなし
		{
		return mucom->SaveFMVoice();
		}
	case MUCOM88IF_MUCOMVM_CMD_GETVOICENUM:		// (返値はテーブルの要素数) , prm3=番号出力先(int*)
		{
		int *pp = (int *)prm3;
		int max = mucom->GetUseVoiceMax();
		for (int i = 0; i < max; i++) {
			// ベクターに追加する
			pp[i] = mucom->GetUseVoiceNum(i);
		}
		return max;
		}
	case MUCOM88IF_MUCOMVM_CMD_GETVOICEDATA:	// prm3,prm4=MUCOM88_VOICEFORMATポインタ出力用のポインタ (prm3はオリジナル、prm4は編集中の音色)
		{
		MUCOM88_VOICEFORMAT **pp3 = (MUCOM88_VOICEFORMAT **)prm3;
		MUCOM88_VOICEFORMAT **pp4 = (MUCOM88_VOICEFORMAT **)prm4;
		*pp3 = mucom->GetVoiceDataOrg();
		*pp4 = mucom->GetVoiceData();
		return 0;
		}
	case MUCOM88IF_MUCOMVM_CMD_GETVOICENAME:	// 音色ファイル名を取得 prm3=char *ポインタ出力用のポインタ
		{
		char **pp = (char **)prm3;
		*pp = (char *)mucom->GetVoiceFileName();
		return 0;
		}
	case MUCOM88IF_MUCOMVM_CMD_GETVMMEMMAP:		// VMののZ80メモリマップを取得 prm3=char *ポインタ出力用のポインタ
		{
		char **pp = (char **)prm3;
		*pp = (char *)vm->GetMemoryMap();
		return 0;
		}
	}
	return -1;
}


int MUCOM88IF_EDITOR_COMMAND(void *ifptr, int cmd, int prm1, int prm2, void *prm3, void *prm4)
{
	//		エディタ用プラグインコマンド
	Mucom88Plugin *plg = (Mucom88Plugin *)ifptr;
	CMucom *mucom = plg->mucom;			// mucomインスタンスを得る
	switch (cmd) {
	case MUCOM88IF_EDITOR_CMD_GETTEXTSIZE:	// パラメーターなし/サイズを返す
		return strlen(mucom->GetEditorMML());
	case MUCOM88IF_EDITOR_CMD_GETTEXT:	// prm3=テキスト書き込み先
		strcpy( (char *)prm3, mucom->GetEditorMML());
		return 0;
	case MUCOM88IF_EDITOR_CMD_UPDATETEXT:	// prm3=書き換えテキストデータ
		return mucom->RequestEditorMML( (const char *)prm3 );
	}
	return -1;
}

