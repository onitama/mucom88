
//
//		MUCOM88 access class
//		(PC-8801のVMを介してMUCOM88の機能を提供します)
//			MUCOM88 by Yuzo Koshiro Copyright 1987-2019(C) 
//			Windows version by onion software/onitama 2018/11
//

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cmucom.h"
#include "mucomvm.h"
#include "mucomerror.h"
#include "md5.h"

#include "bin_music2.h"

#include "bin_expand.h"
#include "bin_errmsg.h"
#include "bin_msub.h"
#include "bin_muc88.h"
#include "bin_ssgdat.h"
#include "bin_time.h"
#include "bin_voice.h"
#include "bin_smon.h"

#define PRINTF vm->Msgf

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
	flag = 0;
	vm = NULL;
	infobuf = NULL;
	edit_status = MUCOM_EDIT_STATUS_NONE;
	user_uuid[0] = 0;
}


CMucom::~CMucom( void )
{
	Stop(1);
	DeleteInfoBuffer();
	MusicBufferTerm();
	if (vm != NULL) delete vm;
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

	// レート設定
	int myrate = rate;
	if (rate == 0) myrate = MUCOM_AUDIO_RATE;
	SetAudioRate(myrate);

	if (window != NULL) {
		vm->SetWindow(window);
	}
	vm->SetOption(option);
	vm->InitSoundSystem(myrate);
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
	vm->Reset();
	PRINTF("#OpenMucom88 Ver.%s Copyright 1987-2019(C) Yuzo Koshiro\r\n",VERSION);
	pcmfilename[0] = 0;

	devres = vm->DeviceCheck();
	if (devres) {
		PRINTF("#Device error(%d)\r\n", devres);
	}

	if (option & MUCOM_CMPOPT_COMPILE) {
		if (option & MUCOM_CMPOPT_USE_EXTROM) {
			//	コンパイラをファイルから読む
			vm->LoadMem("expand", 0xab00, 0);
			vm->LoadMem("errmsg", 0x8800, 0);
			vm->LoadMem("msub", 0x9000, 0);
			vm->LoadMem("muc88", 0x9600, 0);
			vm->LoadMem("ssgdat", 0x5e00, 0);
			vm->LoadMem("time", 0xe400, 0);
			vm->LoadMem("smon", 0xde00, 0);
			LoadFMVoice(MUCOM_DEFAULT_VOICEFILE,true);
		}
		else {
			//	内部のコンパイラを読む
			vm->SendMem(bin_expand, 0xab00, expand_size);
			vm->SendMem(bin_errmsg, 0x8800, errmsg_size);
			vm->SendMem(bin_msub, 0x9000, msub_size);
			vm->SendMem(bin_muc88, 0x9600, muc88_size);
			vm->SendMem(bin_ssgdat, 0x5e00, ssgdat_size);
			vm->SendMem(bin_time, 0xe400, time_size);
			vm->SendMem(bin_smon, 0xde00, smon_size);
			//vm->SendMem(bin_voice_dat, 0x6000, voice_dat_size);
			StoreFMVoice((unsigned char *)bin_voice_dat);
		}

		int i;
		// music側のダミールーチン
		for (i = 0; i < 0x40; i++) {
			vm->Poke(0xb000 + i, 0xc9);
		}
		// basic rom側のダミールーチン
		for (i = 0; i < 0x4000; i++) {
			vm->Poke(0x1000 + i, 0xc9);
		}
		// ワーク設定用ルーチン
		i = 0xb036;
		vm->Poke(i++, 0xdd);	// LD IX,$c9bf
		vm->Poke(i++, 0x21);
		vm->Poke(i++, 0x0c);
		vm->Poke(i++, 0xbf);
		vm->Poke(i++, 0xc9);

		return;
	}
	if (option & MUCOM_CMPOPT_USE_EXTROM) {
		//	プレイヤーをファイルから読む
		vm->LoadMem("music", 0xb000, 0);
	}
	else {
		//	内部のプレイヤーを読む
		vm->SendMem(bin_music2, 0xb000, music2_size);
	}

	//	実行用メモリをシャドーコピーとして保存しておく
	vm->SendMemoryToShadow();

	DeleteInfoBuffer();

	int i,adr;
	vm->InitChData(MUCOM_MAXCH,MUCOM_CHDATA_SIZE);
	for (i = 0; i < MUCOM_MAXCH; i++){
		adr = vm->CallAndHaltWithA(0xb00c, i);
		vm->SetChDataAddress( i,adr );
	}

	NoticePlugins(MUCOM88IF_NOTICE_RESET);
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

int CMucom::Play(int num)
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

	vm->SendMem((const unsigned char *)data, 0xc200, datasize);

	StoreFMVoiceFromEmbed();

	NoticePlugins(MUCOM88IF_NOTICE_PREPLAY);

	PRINTF("#Play[%d]\r\n", num);
	vm->CallAndHalt(0xb000);
	//int vec = vm->Peekw(0xf308);
	//PRINTF("#INT3 $%x.\r\n", vec);

	NoticePlugins(MUCOM88IF_NOTICE_PLAY);

	int jcount = hedmusic->jumpcount;
	vm->SkipPlay(jcount);
	vm->StartINT3();

	playflag = true;

	return 0;
}


void CMucom::PlayLoop() {
	vm->PlayLoop();
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
	playflag = false;
	if (option & 1) {
		vm->StopINT3();
		vm->CallAndHalt(0xb003);
		vm->ResetFM();
	}
	else {
		vm->StopINT3();
		vm->CallAndHalt(0xb003);
	}
	NoticePlugins(MUCOM88IF_NOTICE_STOP);
	return 0;
}


int CMucom::Restart(void)
{
	//		MUCOM88音楽再生の再開(停止後)
	//		(戻り値が0以外の場合はエラー)
	//
	NoticePlugins(MUCOM88IF_NOTICE_PLAY);
	vm->RestartINT3();
	playflag = true;
	return 0;
}


int CMucom::Fade(void)
{
	//		MUCOM88音楽フェードアウト
	//		(戻り値が0以外の場合はエラー)
	//
	if (playflag == false) return -1;
	vm->CallAndHalt(0xb006);
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
		return mubsize * 100 / MUCOM_MUBSIZE_MAX;
	case MUCOM_STATUS_BASICSIZE:
		return basicsize;
	case MUCOM_STATUS_BASICRATE:
		return basicsize * 100 / MUCOM_BASICSIZE_MAX;

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
		vm->SendMem((unsigned char *)fmvoice_internal, MUCOM_FMVOICE_ADR, MUCOM_FMVOICE_SIZE);
		res = vm->SaveMem(voicefilename.c_str(), MUCOM_FMVOICE_ADR, MUCOM_FMVOICE_SIZE);
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
	vm->SendMem(voice, MUCOM_FMVOICE_ADR, MUCOM_FMVOICE_SIZE);
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
		vm->SendMem((unsigned char *)fmvoice_internal, MUCOM_FMVOICE_ADR, MUCOM_FMVOICE_SIZE);
		return vm->SaveMem((char *)tempfilename.c_str(), MUCOM_FMVOICE_ADR, MUCOM_FMVOICE_SIZE);
	}
	vm->ChangeDirectory(curdir);

	return 0;
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

	int vdata = vm->Peekw(0xc201) + 0xc200;
	int result = vm->Peek(vdata++);
	fmvoice_usemax = result;

	for (i = 0; i < result; i++) {
		no = (int)hedmusic->ext_fmvoice[i] - 1;
		fmvoice_use[i] = (unsigned char)no;
		v = (char *)GetFMVoice(no);
		vmmem = no * sizeof(MUCOM88_VOICEFORMAT) + MUCOM_FMVOICE_ADR;
		vm->SendMem( (unsigned char *)v, vmmem, sizeof(MUCOM88_VOICEFORMAT));
		v++;
		for (j = 0; j < 25; j++) {			// 25byteの音色データをコピー
			*v++ = vm->Peek(vdata++);
		}
	}
	return result;
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
	if (a1 >= 128) {				// 全角文字チェック(UTF8)
		int utf8cnt = 0;
		if ((a1 >= 192) && (p[1] != 0)) utf8cnt++;
		if ((a1 >= 224) && (p[2] != 0)) utf8cnt++;
		if ((a1 >= 240) && (p[3] != 0)) utf8cnt++;
		if ((a1 >= 248) && (p[4] != 0)) utf8cnt++;
		if ((a1 >= 252) && (p[5] != 0)) utf8cnt++;
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
	mml = vm->LoadAlloc(fname, &sz);
	if (mml == NULL) {
		PRINTF("#File not found [%s].\r\n", fname);
		return -1;
	}
	ProcessHeader( mml );
	vm->LoadAllocFree(mml);
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
			if (a1 == 0) {
				vm->Poke(mptr++, 0);
				break;
			}
			mulchr = GetMultibyteCharacter(linebuf+i);
			i += mulchr;
			if ( mulchr==1) {
				vm->Poke(mptr++, a1);	// 半角の文字のみ登録する
			}
		}
		vm->Pokew(linkptr, mptr);		// 次のポインタを保存する
		ln += add;
	}

	vm->Pokew(mptr, 0);					// End pointer
	return mptr;
}


int CMucom::Compile(char *text, const char *filename, int option)
{
	//		MUCOM88コンパイル(Resetが必要)
	//		text         = MML書式のテキストデータ(UTF8)(終端=0)
	//		filename     = 出力される音楽データファイル
	//		option : 1   = #タグによるvoice設定を無視
	//		         2   = PCM埋め込みをスキップ
	//		         4   = 音色の一時バッファ作成を許可する
	//		(戻り値が0以外の場合はエラー)
	//
	int i, res;
	int start, length;
	char *adr_start;
	char *adr_length;
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

	basicsize = StoreBasicSource( text, 1, 1 );

	vm->CallAndHalt(0x9600);
	int vec = vm->Peekw(0x0eea8);
	PRINTF("#poll a $%x.\r\n", vec);

	int loopst = 0xf25a;
	vm->Pokew( loopst, 0x0101 );		// ループ情報スタックを初期化する(ループ外の'/'でエラーを出すため)

	res = vm->CallAndHalt2(vec, 'A');
	if (res){
		int line = vm->Peekw(0x0f32e);
		int msgid = vm->GetMessageId();
		if (msgid>0) {
			PRINTF("#error %d in line %d.\r\n-> %s (%s)\r\n", msgid, line, mucom_geterror_j(msgid), mucom_geterror(msgid));
		}
		else {
			PRINTF("#unknown error in line %d.\r\n", line);
		}
		return -1;
	}
	
	char stmp[128];
	vm->PeekToStr(stmp,0xf3c8,80);		// 画面最上段のメッセージ
	PRINTF("%s\r\n", stmp);

	workadr = 0xf320;
	fmvoice = vm->Peek(workadr + 50);
	pcmflag = 0;
	maxcount = 0;
	mubsize = 0;

	jumpcount = vm->Peekw(0x8c90);		// JCLOCKの値(Jコマンドのタグ位置)
	jumpline = vm->Peekw(0x8c92);		// JPLINEの値(Jコマンドの行番号)

	PRINTF("Used FM voice:%d", fmvoice);

	if (jumpcount > 0) {
		PRINTF("  Jump to line:%d", jumpline);
	}

	PRINTF("\r\n");
	PRINTF("[ Total count ]\r\n");

	for (i = 0; i < maxch; i++){
		int tcnt,lcnt;
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

	for (i = 0; i < maxch; i++){
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

	PRINTF("#Data Buffer $%04x-$%04x ($%04x)\r\n", start, start + length - 1,length);
	PRINTF("#MaxCount:%d Basic:%04x Data:%04x\r\n", maxcount, basicsize, mubsize);

	if (tcount[maxch-1]==0) pcmflag = 2;	// PCM chが使われてなければPCM埋め込みはスキップ

	NoticePlugins(MUCOM88IF_NOTICE_COMPEND);

	return SaveMusic(filename,start,length,option| pcmflag);
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
		hed.ext_fmvoice[i] = (unsigned char)vm->Peek(0x8c50+i);
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

	//res = vm->SaveMem(filename, start, length);
	res = vm->SaveMemExpand(fname, start, length, header, hedsize, footer, footsize, pcmdata, pcmsize);
	if (res){
		PRINTF("#File write error [%s].\r\n", fname);
		return -2;
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

