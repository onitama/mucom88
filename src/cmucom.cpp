
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

#include "cmucom.h"
#include "mucomvm.h"
#include "mucomerror.h"
#include "md5.h"

#include "plugin.h"

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

static int htoi_sub(char hstr)
{
	//	exchange hex to int

	char a1;
	a1 = tolower(hstr);
	if ((a1 >= '0') && (a1 <= '9')) return a1 - '0';
	if ((a1 >= 'a') && (a1 <= 'f')) return a1 - 'a' + 10;
	return 0;
}


static int htoi(char *str)
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

static void strcase(char *target)
{
	//		strをすべて小文字に(全角対応版)
	//
	unsigned char *p;
	unsigned char a1;
	p = (unsigned char *)target;
	while (1) {
		a1 = *p; if (a1 == 0) break;
		*p = tolower(a1);
		p++;							// 検索位置を移動
		if (a1 >= 129) {					// 全角文字チェック
			if ((a1 <= 159) || (a1 >= 224)) p++;
		}
	}
}

static int strpick_spc(char *target,char *dest,int strmax)
{
	//		strの先頭からspaceまでを小文字として取り出す
	//
	unsigned char *p;
	unsigned char *dst;
	unsigned char a1;
	int len = 0;
	p = (unsigned char *)target;
	dst = (unsigned char *)dest;
	while (1) {
		if (len >= strmax) break;
		a1 = *p++; if (a1 == 0) return 0;
		if (a1 == 32) break;

		*dst++ = tolower(a1);
		len++;
		if (a1 >= 129) {					// 全角文字チェック
			if ((a1 <= 159) || (a1 >= 224)) {
				p++; len++;
			}
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
	user_uuid[0] = 0;
}


CMucom::~CMucom( void )
{
	Stop(1);
	Mucom88Plugin_Term();
	DeleteInfoBuffer();
	MusicBufferTerm();
	if (vm != NULL) delete vm;
}


void CMucom::Init(void *window, int option)
{
	//		MUCOM88の初期化(初回だけ呼び出してください)
	//		window : 0   = ウインドウハンドル(HWND)
	//		               (NULLの場合はアクティブウインドウが選択される)
	//		option : 0   = 1:FMをミュート  2:SCCIを使用
	//
	vm = new mucomvm;
	flag = 1;

	if (window != NULL) {
		vm->SetWindow(window);
	}
	else {
		//OleInitialize(NULL);
	}
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) {
		return;
	}
	vm->SetOption(option);
	vm->InitSoundSystem();
	MusicBufferInit();

	Mucom88Plugin_Init((HWND)window,vm,this);
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

	if (option & 2) {
		if (option & 1) {
			//	コンパイラをファイルから読む
			vm->LoadMem("expand", 0xab00, 0);
			vm->LoadMem("errmsg", 0x8800, 0);
			vm->LoadMem("msub", 0x9000, 0);
			vm->LoadMem("muc88", 0x9600, 0);
			vm->LoadMem("ssgdat", 0x5e00, 0);
			vm->LoadMem("time", 0xe400, 0);
			vm->LoadMem("smon", 0xde00, 0);
			vm->LoadMem(MUCOM_DEFAULT_VOICEFILE, 0x6000, 0);
		}
		else {
			//	内部のコンパイラを読む
			vm->SendMem(bin_expand, 0xab00, expand_size);
			vm->SendMem(bin_errmsg, 0x8800, errmsg_size);
			vm->SendMem(bin_msub, 0x9000, msub_size);
			vm->SendMem(bin_muc88, 0x9600, muc88_size);
			vm->SendMem(bin_ssgdat, 0x5e00, ssgdat_size);
			vm->SendMem(bin_time, 0xe400, time_size);
			vm->SendMem(bin_voice_dat, 0x6000, voice_dat_size);
			vm->SendMem(bin_smon, 0xde00, smon_size);
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
	if (option & 1) {
		//	プレイヤーをファイルから読む
		vm->LoadMem("music", 0xb000, 0);
	}
	else {
		//	内部のプレイヤーを読む
		vm->SendMem(bin_music2, 0xb000, music2_size);
	}

	vm->SetPlayFlag(true);

	DeleteInfoBuffer();

	//vm->LoadPcm("mucompcm.bin");
	//vm->LoadMem("of7.muc", 0xc200, 0);
	//vm->LoadMem("sampl1.muc", 0xc200, 0);
	//vm->LoadMem("sampl2.muc", 0xc200, 0);
	//vm->LoadMem("sampl3.muc", 0xc200, 0);
	//vm->LoadMem("sc012.muc", 0xc200, 0);
}

void CMucom::SetUUID(char *uuid)
{
	if (uuid == NULL) {
		user_uuid[0] = 0;
		return;
	}
	strncpy(user_uuid,uuid,64);
}


char *CMucom::GetMessageBuffer(void)
{
	return vm->GetMessageBuffer();
}

int CMucom::Play(int num)
{
	//		MUCOM88音楽再生
	//		num : 0   = 音楽No. (0～15)
	//		(戻り値が0以外の場合はエラー)
	//
	MUBHED *hed;
	char *data;
	char *pcmdata;
	char *pcmname;
	int datasize;
	int pcmsize;

	if ((num < 0) || (num >= MUCOM_MUSICBUFFER_MAX)) return -1;

	if (playflag) Stop();

	LoadTagFromMusic(num);

	CMemBuf *buf = musbuf[num];
	if (buf == NULL) return -1;

	hed = (MUBHED *)buf->GetBuffer();
	data = MUBGetData(hed, datasize);
	if (data == NULL) return -1;

	if (hed->pcmdata) {
		int skippcm = 0;
		pcmname = GetInfoBufferByName("pcm");
		if (pcmname[0] != 0) {
			//	既に同名のPCMが読み込まれているか?
			if (strcmp(pcmname, pcmfilename) == 0) skippcm = 1;
		}
		if (skippcm==0) {
			//	埋め込みPCMを読み込む
			pcmdata = MUBGetPCMData(hed, pcmsize);
			vm->LoadPcmFromMem(pcmdata, pcmsize);
		}
	}

	vm->SendMem((const unsigned char *)data, 0xc200, datasize);

	PRINTF("#Play[%d]\r\n", num);
	vm->CallAndHalt(0xb000);
	//int vec = vm->Peekw(0xf308);
	//PRINTF("#INT3 $%x.\r\n", vec);

	vm->StartIN3();
	vm->SetIntCount(0);

	int jcount = hed->jumpcount;
	vm->SkipPlay(jcount);

	playflag = true;

	return 0;
}


int CMucom::LoadTagFromMusic(int num)
{
	//		MUCOM88音楽データからタグ一覧を取得する
	//		num : 0   = 音楽No. (0～15)
	//		(戻り値が0以外の場合はエラー)
	//
	MUBHED *hed;
	int tagsize;

	if ((num < 0) || (num >= MUCOM_MUSICBUFFER_MAX)) return -1;

	CMemBuf *buf = musbuf[num];
	if (buf == NULL) return -1;

	hed = (MUBHED *)buf->GetBuffer();

	if (hed->tagdata) {
		DeleteInfoBuffer();
		infobuf = new CMemBuf();
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
		vm->SetINT3Flag(false);
		vm->CallAndHalt(0xb003);
		vm->ResetFM();
	}
	else {
		vm->SetINT3Flag(false);
		vm->CallAndHalt(0xb003);
	}
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


int CMucom::LoadFMVoice(const char *fname)
{
	//		FM音色データ読み込み
	//		fname = FM音色データファイル (デフォルトはMUCOM_DEFAULT_VOICEFILE)
	//		(戻り値が0以外の場合はエラー)
	//
	if (vm->LoadMem(fname, 0x6000, 0) >= 0) return 0;
	PRINTF("#Voice file not found [%s].\r\n", fname);
	return -1;
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
	musbuf[num] = buf;
	//if (vm->LoadMem(fname, 0xc200, 0) >= 0) return 0;
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
		return vm->GetPassTick();
	case MUCOM_STATUS_MAJORVER:
		return MAJORVER;
	case MUCOM_STATUS_MINORVER:
		return MINORVER;

	case MUCOM_STATUS_COUNT:
		i = vm->GetIntCount();
		if (maxcount) {
			if (i >= maxcount) i = maxcount - 1;
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
Compiler support
*/
/*------------------------------------------------------------*/

char *CMucom::GetTextLine(char *text)
{
	//	1行分のデータを格納
	//
	unsigned char *p = (unsigned char *)text;
	unsigned char a1;
	int mptr = 0;

	while (1) {
		a1 = *p++;
		if (a1 == 0) {
			p = NULL;  break;			// End of text
		}
		if (a1 == 9) {					// TAB->space
			a1 = 32;
		}
		if (a1 == 10) break;			// LF
		if (a1 == 13) {
			if (*p == 10) p++;
			break;						// CR/LF
		}

		if (a1 >= 129) {				// 全角文字チェック
			if ((a1 <= 159) || (a1 >= 224)) {
				linebuf[mptr++] = a1;
				a1 = *p++;
			}
		}
		linebuf[mptr++] = a1;
	}
	linebuf[mptr++] = 0;
	return (char *)p;
}


char *CMucom::GetInfoBuffer(void)
{
	if (infobuf == NULL) return "";
	return infobuf->GetBuffer();
}


char *CMucom::GetInfoBufferByName(char *name)
{
	//		infobuf内の指定タグ項目を取得
	//		name = タグ名(英小文字)
	//		(結果が""の場合は該当項目なし)
	//
	int len;
	char *src = GetInfoBuffer();
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
	//		fname = MML書式のテキストデータファイル(SJIS)
	//		(結果は、infobufに入ります)
	//		(戻り値が0以外の場合はエラー)
	//
	int sz;
	char *mml;
	mml = vm->LoadAlloc(fname, &sz);
	if (mml == NULL) {
		PRINTF("#File not found [%s].\r\n", fname);
		return -1;
	}
	ProcessHeader( mml );
	free(mml);
	return 0;
}


int CMucom::ProcessHeader(char *text)
{
	//		MUCOM88 MMLソース内のタグを処理
	//		text         = MML書式のテキストデータ(SJIS)(終端=0)
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
	char *src = text;
	DeleteInfoBuffer();
	infobuf = new CMemBuf();
	while (1) {
		if (src == NULL) break;
		src = GetTextLine(src);
		if (linebuf[0] == '#') {
			//printf("[%s]\n", linebuf);
			infobuf->PutStr((char *)linebuf);
			infobuf->PutCR();
		}
	}
	return 0;
}


int CMucom::StoreBasicSource(char *text, int line, int add)
{
	//	BASICソースの形式でリストを作成
	//
	char *src = text;
	int ln = line;
	int mptr = 1;
	int linkptr;
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
			a1 = linebuf[i++];
			vm->Poke(mptr++, a1);
			if (a1 == 0) break;
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
	//		text         = MML書式のテキストデータ(SJIS)(終端=0)
	//		filename     = 出力される音楽データファイル
	//		option : 1   = #タグによるvoice設定を無視
	//		         2   = PCM埋め込みをスキップ
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
	if ((option & 1) == 0) {
		char voicefile[MUCOM_FILE_MAXSTR];
		strncpy(voicefile, GetInfoBufferByName("voice"), MUCOM_FILE_MAXSTR);
		if (voicefile[0]) {
			LoadFMVoice(voicefile);
		}
	}

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

	return SaveMusic(filename,start,length,option| pcmflag);
}


int CMucom::CompileFile(const char *fname, const char *sname, int option)
{
	//		MUCOM88コンパイル(Resetが必要)
	//		fname     = MML書式のテキストファイル(SJIS)
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
	free(mml);

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
	char *pcmname;
	int hedsize;
	int footsize;
	int pcmsize;
	if (fname == NULL) return -1;

	header = (char *)&hed;
	hedsize = sizeof(MUBHED);
	memset(header,0,hedsize);
	header[0] = 'M';
	header[1] = 'U';
	header[2] = 'C';
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

	if ((option & 2) == 0) {
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

	if (pcmdata != NULL) free(pcmdata);

	PRINTF("#Saved [%s].\r\n", fname);
	return 0;
}


char *CMucom::MUBGetData(MUBHED *hed, int &size)
{
	char *p;
	p = (char *)hed;
	if ((p[0] != 'M') || (p[1] != 'U') || (p[2] != 'C') || (p[3] != '8')) return NULL;
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
