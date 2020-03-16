
//
//		PC-8801 virtual machine
//		(PC-8801FA相当のCPUとOPNAのみをエミュレーションします)
//			onion software/onitama 2018/11
//			Z80 emulation by Yasuo Kuwahara 2002-2018(C)
//			FM Sound Generator by cisc 1998, 2003(C)
//

#include <stdio.h>
#include <stdio.h>
#include "mucomvm.h"

#include "adpcm.h"
#include "mucomerror.h"

#define BUFSIZE 200			// Stream Buffer 200ms
#define baseclock 7987200		// Base Clock

#include "mucomvm_os.h"
#include "voiceformat.h"

/*------------------------------------------------------------*/
/*
interface
*/
/*------------------------------------------------------------*/

mucomvm::mucomvm(void)
{
	m_flag = VMFLAG_NONE;
	m_option = 0;
	m_fastfw = 4;
	m_fmvol = 0;
	m_ssgvol = -3;

	p_log = NULL;
	p_wav = NULL;

	opn = NULL;
	membuf = NULL;
	osd = NULL;
	master_window = NULL;

	original_mode = true;

	extram_bank_mode = 0;
	extram_bank_no = 0;

	channel_max = 0;
	channel_size = 0;
	pchdata = NULL;

	playflag = false;
	trace_fp = NULL;

	ResetMessageBuffer();
}

mucomvm::~mucomvm(void)
{
	FreePlugins();

	if (osd) {
		osd->FreeTimer();
		osd->FreeAudio();
		osd->FreeRealChip();
		osd = NULL;
	}

	if (opn) {
		delete opn;
		opn = NULL;
	}
	if (membuf) delete membuf;

	if (pchdata) {
		free(pchdata);
	}

}

void mucomvm::SetLogWriter(ILogWrite *log)
{
	p_log = log;
}

void mucomvm::SetWavWriter(WavWriter* wav) {
	p_wav = wav;
}

void mucomvm::GetFMRegMemory(unsigned char* data, int address, int length)
{
	if (address < 0 || OPNAREG_MAX <= address) {
		memset(data, 0, length);
		return;
	}
	if (address + length >= OPNAREG_MAX) {
		int actual_size = OPNAREG_MAX - address;
		memcpy(data, regmap + address, actual_size);
		memset(data + actual_size, 0, length - actual_size);
		return;
	}
	memcpy(data, regmap + address, length);
}


void mucomvm::GetMemory(unsigned char *data, int address, int length)
{
	if (address < 0 || 0x10000 <= address) {
		memset(data, 0, length);
		return;
	}
	if (address + length >= 0x10000) {
		int actual_size = 0x10000 - address;
		RecvMem(data, address, actual_size);
		memset(data + actual_size, 0, length - actual_size);
		return;
	}

	RecvMem(data, address, length);
}

void mucomvm::GetMainMemory(unsigned char* data, int address, int length)
{
	if (address < 0 || 0x10000 <= address) {
		memset(data, 0, length);
		return;
	}
	if (address + length >= 0x10000) {
		int actual_size = 0x10000 - address;
		memcpy(data, mem + address, actual_size);
		memset(data + actual_size, 0, length - actual_size);
		return;
	}

	memcpy(data, mem + address, length);
}

void mucomvm::GetExtMemory(unsigned char* data, int bank, int address, int length)
{
	if (address < 0 || 0x8000 <= address) {
		memset(data, 0, length);
		return;
	}

	if (address + length >= 0x8000) {
		int actual_size = 0x8000 - address;
		memcpy(data, extram[bank] + address, actual_size);
		memset(data + actual_size, 0, length - actual_size);
		return;
	}
	memcpy(data, extram[bank] + address, length);
}

void mucomvm::SetMainMemory(unsigned char* data, int address, int length)
{
	if (address < 0 || 0x10000 <= address) {
		return;
	}

	if (address + length >= 0x10000) {
		int actual_size = 0x10000 - address;
		memcpy(mem + address, data, actual_size);
		return;
	}

	memcpy(mem + address, data, length);
}

void mucomvm::SetMemory(unsigned char* data, int address, int length)
{
}

void mucomvm::SetExtMemory(unsigned char* data, int bank, int address, int length)
{
	if (address < 0 || 0x8000 <= address) {
		return;
	}

	if (address + length >= 0x8000) {
		int actual_size = 0x8000 - address;
		memcpy(extram[bank] + address, data, actual_size);
		return;
	}
	memcpy(extram[bank] + address, data, length);
}



void mucomvm::SetMucomInstance(CMucom *mucom)
{
	p_cmucom = mucom;
}


void mucomvm::SetOption(int option)
{
	m_option = option;
}


void mucomvm::SetVolume(int fmvol, int ssgvol)
{
	if (opn) {
		m_fmvol = fmvol;
		m_ssgvol = ssgvol;
		opn->SetVolumeFM(m_fmvol);
		opn->SetVolumePSG(m_ssgvol);
	}
}

void mucomvm::SetFastFW(int value)
{
	if (opn) {
		m_fastfw = value;
	}
}


static void RunAudioCallback(void *CallbackInstance, void *MethodInstance);
static void RunTimerCallback(void *CallbackInstance, void *MethodInstance);

// オーディオコールバック
static void RunAudioCallback(void *CallbackInstance, void *MethodInstance) {
	AudioCallback *acb = (AudioCallback *)CallbackInstance;
	((mucomvm *)MethodInstance)->AudioCallback(acb->mix, acb->size);
}

// タイマーコールバック
static void RunTimerCallback(void *CallbackInstance, void *MethodInstance) {
	TimerCallback *tcb = (TimerCallback *)CallbackInstance;
	((mucomvm *)MethodInstance)->UpdateCallback(tcb->tick);
}


void mucomvm::ResetFM(void)
{
	//	VMのリセット(FM部のみ)
	//
	if (playflag) {
		osd->WaitSendingAudio();
	}

	int i;
	for (i = 0; i < OPNACH_MAX; i++) {
		chmute[i] = 0; chstat[i] = 0;
	}
	memset(regmap, 0, OPNAREG_MAX);

	if (opn) {
		int3mask = 0;
		int3flag = false;
		opn->Reset();
		opn->SetRate(baseclock, Rate, false);
		opn->SetReg(0x2d, 0);
		opn->SetVolumeFM(m_fmvol);
		opn->SetVolumePSG(m_ssgvol);
	}

	if (m_option & VM_OPTION_SCCI) {
		osd->ResetRealChip();
		osd->OutputRealChip(0x2d, 0);
	}
}

void mucomvm::InitSoundSystem(int rate)
{
	// レート設定
	Rate = rate;

	//	サウンドの初期化(初回のみでOK)
	//
	playflag = false;
	predelay = 0;

	// OS依存部分
	osd = new OSDEP_CLASS();
	if (osd == NULL) return;

	//		COM初期化
	//
	osd->CoInitialize();

	//		SCCI対応
	//
	if (m_option & VM_OPTION_SCCI) {
		osd->InitRealChip();
	}

	//		オーディオ
	if (!(m_option & VM_OPTION_STEP)) {
		osd->UserAudioCallback->Set(this, &RunAudioCallback);
		osd->UserTimerCallback->Set(this, &RunTimerCallback);
		osd->InitAudio(master_window, rate, BUFSIZE);
	}

	//		タイマー初期化
	//
	ResetTimer();

	if (!(m_option & VM_OPTION_STEP)) {
		osd->InitTimer();
		osd->ResetTime();
	}

	opn = new FM::OPNA;
	if (opn) {
		opn->Init(baseclock, 8000, 0);
#if 0
		// PSG TEST
		opn->SetReg(0x07, 8 + 16 + 32);
		opn->SetReg(0x08, 0x8);
		opn->SetReg(0x00, 0x0);
		opn->SetReg(0x01, 0x4);
#endif
		ResetFM();
	}

	playflag = true;
	//printf("#Stream update %dms.\n", time_stream);
}

void mucomvm::Reset(void)
{
	//	VMのリセット(FMResetを含む)
	//
	uint8_t *p;

	Z80::Reset();
	ResetFM();
	SetIntVec(0xf3);

	p = &mem[0];
	memset(p, 0, 0x10000);		// CLEAR

	ResetExtRam();

	ClearBank();
	bankprg = VMPRGBANK_MAIN;

	// バンクをリセット
	int i = 0;
	for (i = 0; i < 0x10; i++) {
		membank[i] = &mem[i * 0x1000];
		membank_wr[i] = &mem[i * 0x1000];
	}

	m_flag = VMFLAG_EXEC;
	sound_reg_select = 0;

#if 0
	*p++ = 0x3c;	    // LABEL1:INC A
	*p++ = 0xd3;
	*p++ = 0x01;		// OUT (0),A
	*p++ = 0x76;		// HALT
	*p++ = 0x18;
	*p++ = 0xfb;		// JR LABEL1
#endif
	ResetMessageBuffer();
}

void mucomvm::ResetExtRam()
{
	int i;
	for (i = 0; i < 4; i++) {
		memset(extram[i], 0, 0x8000);
	}
}

void mucomvm::ResetMessageBuffer(void) {
	if (membuf) delete membuf;
	membuf = new CMemBuf;
}


int mucomvm::DeviceCheck(void)
{
	//	デバイスの正常性をチェック
	//
	if (m_option & VM_OPTION_SCCI) {
		return osd->CheckRealChip();
	}
	return 0;
}


bool mucomvm::GetSB2Present(void)
{
	if (m_option & VM_OPTION_SCCI) {
		return ( osd->CheckRealChipSB2() != 0 );
	}
	return true;
}


int32_t mucomvm::loadpc(uint16_t adr)
{
	if (bankprg == VMPRGBANK_SHADOW) {
		if ((adr < 0xde00)||(adr >=0xe300)) {
			return (int32_t)membank[adr>>12][adr&0xfff];
		}
		return (int32_t)memprg[adr];
	}
	return (int32_t)membank[adr>>12][adr&0xfff];
}


int32_t mucomvm::load(uint16_t adr)
{
	return (int32_t)membank[adr>>12][adr & 0xfff];
}


void mucomvm::store(uint16_t adr, uint8_t data)
{
	membank_wr[adr>>12][adr&0xfff] = data;
}


int32_t mucomvm::input(uint16_t adr)
{
	//printf("input : %x\n", adr);
	int port = adr & 0xff;

	// キーボードはOFF状態
	if (port < 0x10) return 0xff;
	switch (port)
	{
	case 0x32:
		return int3mask;
	case 0x45:
		return FMInData();
	case 0x47:
		return FMInData2();
	default:
		break;
	}
	return 0;
}


void mucomvm::output(uint16_t adr, uint8_t data)
{
	int port = adr & 0xff;
	switch (port)
	{
	case 0x32:
		int3mask = (int)data;
		break;
	case 0x44:
		sound_reg_select = (int)data;
		break;
	case 0x45:
		FMOutData((int)data);
		break;
	case 0x46:
		sound_reg_select2 = (int)data;
		break;
	case 0x47:
		FMOutData2((int)data);
		break;
	case 0x5c:
	case 0x5d:
	case 0x5e:
	case 0x5f:
		ChangeBank( port - 0x5c );
		break;
	case 0xe2:
		ChangeExtRamMode(data);
		break;
	case 0xe3:
		ChangeExtRamBank(data);
		break;
	default:
		break;
	}
}



int mucomvm::GetExtRamBank()
{
	return extram_bank_no;
}

void mucomvm::SetExtRamBank(uint8_t bank)
{
	extram_bank_no = bank;
}

int mucomvm::GetExtRamMode()
{
	return extram_bank_mode;
}

// 拡張RAM切り替え
void mucomvm::ChangeExtRam(uint8_t mode, uint8_t bank)
{
	extram_bank_no = bank;
	ChangeExtRamMode(mode);
}


// 拡張RAM切り替え
void mucomvm::ChangeExtRamMode(uint8_t mode)
{
	extram_bank_mode = mode;
	ChangeExtRamBank(extram_bank_no);
}

// 拡張RAM切り替え
void mucomvm::ChangeExtRamBank(uint8_t bank)
{
	extram_bank_no = bank & 0x3;
	uint8_t* wrp;
	uint8_t* rdp;

	// bit4 = 書き込み
	if (extram_bank_mode & 0x10) {
		wrp = extram[extram_bank_no];
	}
	else {
		wrp = mem;
	}

	// bit0 = 読み込み
	if (extram_bank_mode & 0x01) {
		rdp = extram[extram_bank_no];
	}
	else {
		rdp = mem;
	}

	int i;
	for (i = 0; i < 0x8; i++) {
		membank[i] = &rdp[i * 0x1000];
		membank_wr[i] = &wrp[i * 0x1000];
	}
}

// ドライバの種類を設定
void mucomvm::SetOrignalMode( bool mode )
{
	original_mode = mode;
}

void mucomvm::Halt(void)
{
	//if (m_flag == VMFLAG_EXEC) printf("Halt.\n");
	m_flag = VMFLAG_HALT;
}


void mucomvm::SendMemoryToShadow(void)
{
	//	メモリのシャドーコピーを作成する
	//
	memcpy(memprg, mem, 0x10000);
}


/*------------------------------------------------------------*/
/*
status
*/
/*------------------------------------------------------------*/

void mucomvm::ClearBank(void)
{
	int i;
	uint8_t *p;

	for (i = 0; i < VMBANK_MAX; i++) {
		p = &vram[i][0];
		memset(p, 0, VMBANK_SIZE);		// CLEAR
	}
	bankmode = VMBANK_MAX-1;
}


void mucomvm::ChangeBank(int bank)
{
	uint8_t *p;
	if (bankmode == bank) return;
	//	現在のメモリ内容を退避
	p = &vram[bankmode][0];
	memcpy(p, mem+0xc000, VMBANK_SIZE);
	//	あたらしいバンクデータをコピー
	p = &vram[bank][0];
	memcpy(mem + 0xc000,p, VMBANK_SIZE);
	bankmode = bank;
}


void mucomvm::BackupMem(uint8_t *mem_bak)
{
	memcpy( mem_bak, mem, 0x10000 );
}

void mucomvm::RestoreMem(uint8_t *mem_bak)
{
	memcpy(mem, mem_bak, 0x10000);
}

int mucomvm::Peek(uint16_t adr)
{
	return (int)membank[adr>>12][adr&0xfff];
}

int mucomvm::Peekw(uint16_t adr)
{
	return ((Peek(adr+1))<<8) + Peek(adr);
}


void mucomvm::Poke(uint16_t adr, uint8_t data)
{
	membank[adr >> 12][adr & 0xfff] = data;
}

void mucomvm::FillMem(uint16_t adr, uint8_t value, uint16_t length)
{
	int i;
	for (i = 0; i < length; i++) {
		Poke(adr + i, value);
	}
}


void mucomvm::Pokew(uint16_t adr, uint16_t data)
{
	Poke(adr, data & 0xff);
	Poke(adr+1, (data>>8) & 0xff);
}


void mucomvm::PeekToStr(char *out, uint16_t adr, uint16_t length)
{
	int i;
	for (i = 0; i < 80; i++) {
		out[i] = Peek(adr + i);
	}
	out[i] = 0;
}


int mucomvm::ExecUntilHalt(int times)
{
    if (m_flag == VMFLAG_NONE) {
        return 0;
    }

	bool adrmap[0x10000];
	memset(adrmap, 0, 0x10000);

	int last_pc = 0x0;
	int cnt=0;
	int id = 0;
	msgid = 0;
	while (1) {
#ifdef DEBUGZ80_TRACE
		//membuf->Put( (int)pc );
		Msgf("%06x PC=%04x HL=%04x A=%02x\r\n", cnt, pc, GetHL(), GetA());
#endif
		//printf("PC=%04x HL=%04x (%d):\n", pc, GetHL(), Peek(0xa0f5));
		if (pc < 0x8000) {
			if (pc == 0x5550) {
				int i;
				uint8_t a1;
				uint16_t hl = GetHL();
				char stmp[256];
				i = 0;
				while (1) {
					if (i >= 250) break;
					a1 = mem[hl++];
					if (a1 == 0) break;
					stmp[i++] = (char)a1;
				}
				stmp[i++] = 0;
				Poke(pc, 0xc9);
				id = mucom_geterror(stmp);
				if (id > 0) {
					msgid = id;
					//Msgf("#%s\r\n", mucom_geterror(msgid),msgid);
				}
				else {
					Msgf("#Unknown message [%s].\r\n", stmp);
				}
			}
			else if (pc == 0x18) {
				//Msgf("#RST 18H Trap at $%04x.\r\n", GetA());
				return -1;
			}
			else if (pc == 0x3b3) {
				Msgf("#Error trap at $%04x.\r\n", pc);
				//DumpBin(0, 0x100);
				return -1;
			}
			else {
				Msgf("#Unknown ROM called $%04x.\r\n", pc);
				//return -2;
			}
		}

		if (original_mode) ExecuteCLUC(); else ExecuteModCLUC();

#if 1
		ConvertVoice();
#endif

		if (!adrmap[pc]) {
			adrmap[pc] = true;
			if (verbose) printf("run:pc:%04x\n", pc);
		}

		last_pc = pc;
		Execute(times);
		if (m_flag == VMFLAG_HALT) break;
		cnt++;
	}
#ifdef DEBUGZ80_TRACE
	membuf->SaveFile("trace.txt");
#endif
	//Msgf( "#CPU halted.\n" );
	return 0;
}

// アドレスを元に設定
#define CONVERT 0xde06

// 音色変換 em版ではスキップ
void mucomvm::ConvertVoice()
{
	if (!original_mode) return;

	if (pc == CONVERT) {
		// CONVERT(音色定義コンバート)
		//	出力データが大きい場合、DE06H～とかぶるので代替コードで実行する
		// $6001～ 38byteの音色データを25byteに圧縮する->$6001から書き込む

		// 音色は表メモリでの処理になる
		MUCOM88_VOICEFORMAT* v = (MUCOM88_VOICEFORMAT*)(memprg + 0x6000);
		unsigned char* src = mem + 0x6001;
		v->ar_op1 = *src++;  v->dr_op1 = *src++; v->sr_op1 = *src++; v->rr_op1 = *src++; v->sl_op1 = *src++; v->tl_op1 = *src++; v->ks_op1 = *src++; v->ml_op1 = *src++; v->dt_op1 = *src++;
		v->ar_op2 = *src++;  v->dr_op2 = *src++; v->sr_op2 = *src++; v->rr_op2 = *src++; v->sl_op2 = *src++; v->tl_op2 = *src++; v->ks_op2 = *src++; v->ml_op2 = *src++; v->dt_op2 = *src++;
		v->ar_op3 = *src++;  v->dr_op3 = *src++; v->sr_op3 = *src++; v->rr_op3 = *src++; v->sl_op3 = *src++; v->tl_op3 = *src++; v->ks_op3 = *src++; v->ml_op3 = *src++; v->dt_op3 = *src++;
		v->ar_op4 = *src++;  v->dr_op4 = *src++; v->sr_op4 = *src++; v->rr_op4 = *src++; v->sl_op4 = *src++; v->tl_op4 = *src++; v->ks_op4 = *src++; v->ml_op4 = *src++; v->dt_op4 = *src++;
		v->fb = *src++; v->al = *src++;
		memcpy(mem + 0x6000, memprg + 0x6000, 26);

		SetHL(0x6001);
		Poke(CONVERT, 0xc9);
		// pc = 0xafb3;				// retの位置まで飛ばす($c9のコードなら何でもいい)
	}
}

void mucomvm::ExecuteCLUC()
{
	if (pc == 0xaf80) {				// expand内のCULC: を置き換える
		int amul = GetA();
		int val = Peekw(0xAF93); // 0xAF93 = CULLP2 + 1
		int frq = Peekw(0xAFFA); // 0xAFFA = FRQBEF
		int ans, count;
		float facc;
		float frqbef = (float)frq;
		if (val == 0x0A1BB) {
			facc = 0.943874f;
		}
		else {
			facc = 1.059463f;
		}
		for (count = 0; count < amul; count++) {
			frqbef = frqbef * facc;
		}
		ans = int(frqbef);
		SetHL(ans);
		//Msgf("#CULC A=%d : %d * %f =%d.\r\n", amul, frq, facc, ans);

		Poke(0xaf80, 0xc9); // RETにする

		//pc = 0xafb3;				// retの位置まで飛ばす
	}
}


// MUCOM88em版
void mucomvm::ExecuteModCLUC()
{
	if (pc != MUCOM_EM_CULC) { return; }

	int amul = GetA();
	int val = Peekw(MUCOM_EM_CULLP2 + 1);
	int frq = Peekw(MUCOM_EM_FRQBEF);
	int ans, count;
	float facc;
	float frqbef = (float)frq;
	facc = (val == 0x0A1BB) ? 0.943874f : 1.059463f;
		
	for (count = 0; count < amul; count++) {
		frqbef = frqbef * facc;
	}
	ans = int(frqbef);
	SetHL(ans);
	//Msgf("#CULC A=%d : %d * %f =%d.\r\n", amul, frq, facc, ans);

	Poke(MUCOM_EM_CULC, 0xc9); // RETにする	
}


void mucomvm::DumpBin(uint16_t adr, uint16_t length)
{
	int i;
	uint16_t p = adr;
	uint16_t ep = adr+length;
	while (1) {
		if (p >= ep) break;
		Msgf("#$%04x", p);
		for (i = 0; i < 16; i++) {
			Msgf(" %02x", Peek(p++));
		}
		Msgf("\r\n");
	}
}


void mucomvm::Msgf(const char *format, ...)
{
	char textbf[4096];
	va_list args;
	va_start(args, format);
	vsprintf(textbf, format, args);
	va_end(args);
	membuf->PutStr(textbf);
}


void mucomvm::SetPC(uint16_t adr)
{
	pc = adr;
	m_flag = VMFLAG_EXEC;
}


void mucomvm::CallAndHalt(uint16_t adr)
{
    if (m_flag == VMFLAG_NONE) {
        return;
    }
	uint16_t tempadr = 0xf000;
	uint8_t *p = mem + tempadr;
	*p++ = 0xcd;				// Call
	*p++ = (adr & 0xff);
	*p++ = ((adr>>8) & 0xff);
	*p++ = 0x76;				// Halt
	SetPC(tempadr);
	ExecUntilHalt();
}


int mucomvm::CallAndHalt2(uint16_t adr,uint8_t code)
{
	uint16_t tempadr = 0xf000;
	uint8_t *p = mem + tempadr;

	*p++ = 0x21;				// ld hl
	*p++ = 0x10;
	*p++ = 0xf0;
	*p++ = 0xcd;				// Call
	*p++ = (adr & 0xff);
	*p++ = ((adr >> 8) & 0xff);
	*p++ = 0x76;				// Halt
	p = mem + tempadr + 16;
	*p++ = code;
	*p++ = 0;
	*p++ = 0;

	SetPC(tempadr);
	return ExecUntilHalt(1);
}


int mucomvm::CallAndHaltWithA(uint16_t adr, uint8_t areg)
{
	uint16_t tempadr = 0xf000;
	uint8_t *p = mem + tempadr;
	*p++ = 0x3e;				// ld a
	*p++ = areg;
	*p++ = 0xcd;				// Call
	*p++ = (adr & 0xff);
	*p++ = ((adr >> 8) & 0xff);
	*p++ = 0x76;				// Halt
	SetPC(tempadr);
	ExecUntilHalt();
	return (int)GetIX();
}

int mucomvm::CallAndHaltWithB(uint16_t adr, uint8_t val)
{
	uint16_t tempadr = 0xf000;
	uint8_t* p = mem + tempadr;
	*p++ = 0x06;				// ld b
	*p++ = val;
	*p++ = 0xcd;				// Call
	*p++ = (adr & 0xff);
	*p++ = ((adr >> 8) & 0xff);
	*p++ = 0x76;				// Halt
	SetPC(tempadr);
	ExecUntilHalt();
	return (int)GetIX();
}

int mucomvm::LoadMem(const char *fname, int adr, int size)
{
	//	VMメモリにファイルをロード
	//
	FILE *fp;
	int flen,sz;
	fp = fopen(fname, "rb");
	if (fp == NULL) return -1;
	sz = size; if (sz == 0) sz = 0x10000;
	flen = (int)fread(mem+adr, 1, sz, fp);
	fclose(fp);
	Msgf("#load:%04x-%04x : %s (%d)\r\n", adr, adr + flen, fname, flen);
	return flen;
}


char *mucomvm::LoadAlloc(const char *fname, int *sizeout)
{
	//	メモリにファイルをロード
	//
	FILE *fp;
	char *buf;
	int sz;

	*sizeout = 0;
	fp = fopen(fname, "rb");
	if (fp == NULL) return NULL;
	fseek(fp, 0, SEEK_END);
	sz = (int)ftell(fp);			// normal file size
	if (sz <= 0) return NULL;
	buf = (char *)malloc(sz+16);
	if (buf) {
		fseek(fp, 0, SEEK_SET);
		fread(buf, 1, sz, fp);
		fclose(fp);
		buf[sz] = 0;
		Msgf("#load:%s (%d)\r\n", fname, sz);
		*sizeout = sz;
		return buf;
	}
	fclose(fp);
	return NULL;
}


void mucomvm::LoadAllocFree(char *ptr)
{
	//	LoadAllocをロードしたメモリを返却する
	//
	free(ptr);
}

// ADPCM読み出し VMのメモリを経由せずにロードする
int mucomvm::LoadPcmFromMem(const char *buf, int sz, int maxpcm)
{
	//	PCMデータをOPNAのRAMにロード(メモリから)
	//
	char *pcmdat;
	char *pcmmem;
	int infosize;
	int i;
	int pcmtable;
	int inftable;
	int adr, whl, eadr;
	char pcmname[17];
	const unsigned char *table = (const unsigned char*)buf;


	infosize = 0x400;
	inftable = 0xd000;
	//SendMem((const unsigned char *)buf, inftable, infosize);
	pcmtable = 0xe300;
	for (i = 0; i < maxpcm; i++) {
		//adr = Peekw(inftable + 28);
		//whl = Peekw(inftable + 30);
		adr = PeekTableWord(table, 28);
		whl = PeekTableWord(table, 30);
		eadr = adr + (whl >> 2);
		if (buf[i * 32] != 0) {
			Pokew(pcmtable, adr);
			Pokew(pcmtable + 2, eadr);
			Pokew(pcmtable + 4, 0);
			//Pokew(pcmtable + 6, Peekw(inftable + 26));
			Pokew(pcmtable + 6, PeekTableWord(table,26));
			memcpy(pcmname, buf + i * 32, 16);
			pcmname[16] = 0;
			Msgf("#PCM%d $%04x $%04x %s\r\n", i + 1, adr, eadr, pcmname);
		}
		pcmtable += 8;
		inftable += 32;
		table += 32;
	}
	pcmdat = (char *)buf + infosize;
	pcmmem = (char *)opn->GetADPCMBuffer();
	memcpy(pcmmem, pcmdat, sz - infosize);

	//if (p_log) p_log->WriteAdpcmMemory(pcmdat, sz - infosize);

	if (m_option & VM_OPTION_SCCI) {
		osd->OutputRealChipAdpcm(pcmdat, sz - infosize);
	}

	return 0;
}

int mucomvm::PeekTableWord(const unsigned char* table, int adr)
{
	return (table[adr] | (table[adr + 1] << 8));
}


int mucomvm::LoadPcm(const char *fname,int maxpcm)
{
	//	PCMデータをOPNAのRAMにロード
	//	(別途、MUCOM88のPCMデータをパッキングしたデータが必要)
	//
	int sz;
	char *buf;
	buf = LoadAlloc( fname, &sz );
	if (buf) {
		LoadPcmFromMem( buf,sz,maxpcm );
		LoadAllocFree(buf);
	}
	return 0;
}


int mucomvm::SendMem(const unsigned char *src, int adr, int size)
{
	//	VMメモリにデータを転送
	//
	CopyMemToVm(src, adr, size);
	return 0;
}

int mucomvm::SendExtMem(const unsigned char* src, int bank, int adr, int size)
{
	CopyMemToExtRam(src, bank, adr, size);
	return 0;
}


int mucomvm::RecvMem(unsigned char* mem, int adr, int size)
{
	CopyMemFromVm(mem, adr, size);
	return 0;
}

int mucomvm::SaveMem(const char *fname, int adr, int size)
{
	//	VMメモリの内容をファイルにセーブ
	//

	uint8_t* rdbuf = new uint8_t[size];
	CopyMemFromVm(rdbuf, adr, size);
	int ret = SaveToFile(fname, rdbuf, size);
	delete[] rdbuf;
	return ret;
}


int mucomvm::SaveToFile(const char *fname, const unsigned char *src, int size)
{
	//	バイナリファイルを保存
	//
	FILE *fp;
	int flen;
	fp = fopen(fname, "wb");
	if (fp == NULL) return -1;
	flen = (int)fwrite(src, 1, size, fp);
	fclose(fp);
	return 0;
}

void mucomvm::CopyMemToVm(const uint8_t * src, int address, int length) 
{
	int i = 0;
	int left = length;
	int start = address & 0xfff;
	int bank = address >> 12;

	while(0 < left) {
		int block_size = start + left < 0x1000 ? left : 0x1000 - start;
		memcpy(membank_wr[bank + i] + start, src, block_size);
		i++;
		start = 0;
		src += block_size;
		left -= block_size;
	}
}

void mucomvm::CopyMemToExtRam(const uint8_t* src, int bank, int address, int length)
{
	if (bank < 0 || 4 <= bank) return;
	if (address < 0 || 0x8000 <= address) return;

	int block_size = address + length < 0x8000 ? length : 0x8000 - address;
	memcpy(extram[bank] + address, src, block_size);
}


void mucomvm::CopyMemFromVm(uint8_t *dest, int address, int length) 
{
	int i = 0;
	int left = length;
	int start = address & 0xfff;
	int bank = address >> 12;

	while (0 < left) {
		int block_size = start + left < 0x1000 ? left : 0x1000 - start;
		memcpy(dest, membank[bank + i] + start, block_size);
		i++;
		start = 0;
		dest += block_size;
		left -= block_size;
	}
}


int mucomvm::SaveMemExpand(const char *fname, int adr, int size, char *header, int hedsize, char *footer, int footsize, char *pcm, int pcmsize)
{
	//	VMメモリの内容をファイルにセーブ(ヘッダとフッタ付き)
	//
	FILE *fp;
	fp = fopen(fname, "wb");
	if (fp == NULL) return -1;
	if (header) fwrite(header, 1, hedsize, fp);

	uint8_t* rdbuf = new uint8_t[size];
	CopyMemFromVm(rdbuf, adr, size);	
	fwrite(rdbuf, 1, size, fp);
	delete[] rdbuf;

	if (footer) fwrite(footer, 1, footsize, fp);
	if (pcm) fwrite(pcm, 1, pcmsize, fp);
	fclose(fp);
	return 0;
}


int mucomvm::StoreMemExpand(CMemBuf *buf, int adr, int size, char *header, int hedsize, char *footer, int footsize, char *pcm, int pcmsize)
{
	//	VMメモリの内容をmembufに保存(ヘッダとフッタ付き)
	//
	if ( buf==NULL ) return -1;
	if (header) buf->PutData(header, hedsize);

	uint8_t* rdbuf = new uint8_t[size];
	CopyMemFromVm(rdbuf, adr, size);
	buf->PutData(rdbuf, size);
	delete[] rdbuf;

	if (footer) buf->PutData(footer, footsize);
	if (pcm) buf->PutData(pcm, pcmsize);
	return 0;
}


int mucomvm::KillFile(const char *fname)
{
	//		ファイルの削除
	//
	return osd->KillFile(fname);
}


int mucomvm::GetDirectory(char *buf, int size)
{
	//		ファイルの削除
	//
	return osd->GetDirectory(buf,size);
}


int mucomvm::ChangeDirectory(const char *dir)
{
	//		ファイルの削除
	//
	return osd->ChangeDirectory(dir);
}


void mucomvm::SkipPlay(int count)
{
	//		再生のスキップ
	//
	if (count <= 0) return;

	int jcount = count;
	int vec = Peekw(0xf308);		// INT3 vectorを取得

	SetChMuteAll(true);
	busyflag = true;				// Z80VMの2重起動を防止する
	while (1) {
		if (jcount <= 0) break;
		CallAndHalt(vec);
		jcount--;
		time_intcount++;
		if (m_option & VM_OPTION_SCCI) {
			if ((jcount & 3) == 0) osd->Delay(1);	// ちょっと待ちます
		}
	}
	busyflag = false;
	SetChMuteAll(false);
}


/*------------------------------------------------------------*/
/*
Timer
*/
/*------------------------------------------------------------*/

void mucomvm::ResetTimer(void)
{
	busyflag = false;
	predelay = 0;

	time_master = 0;
	time_scount = 0;
	time_intcount = 0;

	pass_tick = 0;
	last_tick = 0;
	audio_output_ms = 0;

	osd->ResetTime();
}


void mucomvm::UpdateTime(int base)
{
	//		1msごとの割り込み
	//		base値は、1024=1msで経過時間が入る
	//
	if (playflag == false) {
		return;
	}

	if (p_log != NULL) p_log->Wait(((double)base)/(1024*1000));

	bool stream_event = false;
	bool int3_mode = int3flag;

	time_master += base;
	time_scount += base;

	if (tmflag) {
		return;
	}
	tmflag = true;

	if (int3mask & 128) int3_mode = false;		// 割り込みマスク

	if (opn->Count(base)) {
		if (int3_mode) {
			stream_event = true;
			if (predelay == 0) {
				int times = 1;
				if (m_option & VM_OPTION_FASTFW) times = m_fastfw;
				busyflag = true;				// Z80VMの2重起動を防止する
				while (1) {
					if (times <= 0) break;
					int vec = Peekw(0xf308);	// INT3 vectorを呼び出す
					CallAndHalt(vec);
					times--;
					time_intcount++;
				}
				busyflag = false;
				ProcessChData();
				NoticePlugins(MUCOM88IF_NOTICE_INTDONE, NULL, NULL);		// プラグインに通知する
			}
			else {
				predelay--;
			}
		}
	}

	if ((stream_event == false) && (int3_mode == false)) {
		// INT3が無効な場合もストリーム再生は続ける
		if (time_scount > (10<< TICK_SHIFT)) {
			stream_event = true;
		}
	}

	if (stream_event) {
		stream_event = false;
		//pass_tick = ( opn->GetNextEvent() + 1023 ) >> TICK_SHIFT;
		pass_tick = time_scount >> TICK_SHIFT;
		time_scount = ( time_scount & ((int)TICK_FACTOR - 1) );
		osd->SendAudio(pass_tick);
		audio_output_ms += pass_tick;
	}
	tmflag = false;
}

// Z80の処理を待つ
void mucomvm::WaitReady(void)
{
	while(busyflag) osd->Delay(10);
}


int mucomvm::GetDriverStatus(int option)
{
	//		ドライバー固有の値を取得
	//
	return osd->GetStatus(option);
}


void mucomvm::StartINT3(void)
{
	//		INT3割り込みを開始
	//
	if (int3flag) {
		osd->WaitSendingAudio();
		checkThreadBusy();
	}
	osd->ResetTime();
	SetIntCount(0);
	predelay = 4;
	int3flag = true;
	tmflag = false;
}


void mucomvm::RestartINT3(void)
{
	//		INT3割り込みを再開
	//
	if (int3flag) {
		return;
	}
	int3mask &= 0x7f;
	FMRegDataOut( 0x32, int3mask);
	osd->WaitSendingAudio();
	checkThreadBusy();
	osd->ResetTime();
	int3flag = true;
	tmflag = false;
}


void mucomvm::StopINT3(void)
{
	//		INT3割り込みを停止
	//
	osd->WaitSendingAudio();
	int3flag = false;
	checkThreadBusy();
	//SetIntCount(0);
	predelay = 0;
}


void mucomvm::checkThreadBusy(void)
{
	//		スレッドがVMを使用しているかチェック
	//
	for (int i = 0; i < 300 && busyflag; i++) osd->Delay(10);
}


void mucomvm::PlayLoop() {
	osd->SetBreakHook();
	while (!osd->GetBreakStatus()) {
		osd->Delay(20);
	}
}

void mucomvm::RenderAudio(void *mix, int size) {
	opn->Mix((FM::Sample *)mix, size);
	if (p_wav != NULL) p_wav->WriteData((int*)mix, size);
}

void mucomvm::UpdateCallback(int tick) {
	if (m_option & VM_OPTION_STEP) return;
	UpdateTime(tick);
}


void mucomvm::AudioCallback(void *mix, int size) {
	if (m_option & VM_OPTION_FMMUTE || m_option & VM_OPTION_STEP) return;
	RenderAudio(mix, size);
}


/*------------------------------------------------------------*/
/*
FM synth
*/
/*------------------------------------------------------------*/

//	レジスタデータ出力
void mucomvm::FMRegDataOut(int reg, int data)
{
	regmap[reg] = (uint8_t)data;			// 内部レジスタ保持用
	opn->SetReg(reg, data);
	if (p_log != NULL) p_log->WriteData(0, reg, data);

	if (m_option & VM_OPTION_SCCI) {
		// リアルチップ出力
		osd->OutputRealChip(reg, data);
	}
}

//	レジスタデータ取得
int mucomvm::FMRegDataGet(int reg)
{
	return (int)regmap[reg];
}

void mucomvm::FMOutData(int data)
{
	//printf("FMReg: %04x = %02x\n", sound_reg_select, data);
	//TraceLog(data);

	switch (sound_reg_select) {
	case 0x28:
	{
		//		FM KeyOn
		int ch = data & 7;
		int slot = data & 0xf0;
		if (chmute[ch]) return;
		if (slot) chstat[ch] = 1; else chstat[ch] = 0;
		break;
	}
	case 0x10:
	{
		//		Rhythm KeyOn
		if (chmute[OPNACH_RHYTHM]) return;
		if (data & 0x80) chstat[OPNACH_ADPCM] = 1; else chstat[OPNACH_ADPCM] = 0;
		break;
	}
	default:
		break;
	}

	FMRegDataOut(sound_reg_select, data);
}

void mucomvm::TraceLog(int data)
{
	if (trace_fp == NULL) {
		trace_fp = fopen("tracelog.txt","w");
	}
	if (trace_fp != NULL) 
		fprintf(trace_fp,"FMReg: %04x = %02x\n", sound_reg_select, data);
}

//	データ出力(OPNA側)
void mucomvm::FMOutData2(int data)
{
	//if (verbose) printf("FMReg2: %04x = %02x\n", sound_reg_select, data);

	if (sound_reg_select2 == 0x00) {
		if (chmute[OPNACH_ADPCM]) return;
		if (data & 0x80) chstat[OPNACH_ADPCM] = 1; else chstat[OPNACH_ADPCM] = 0;
	}

	FMRegDataOut(sound_reg_select2 | 0x100, data);
}

//	データ入力
int mucomvm::FMInData(void)
{
	return (int)opn->GetReg(sound_reg_select);
}


//	データ入力(OPNA側)
int mucomvm::FMInData2(void)
{
	return (int)opn->GetReg(sound_reg_select2|0x100);
}


void mucomvm::SetChMute(int ch, bool sw)
{
	chmute[ch] = (uint8_t)sw;
}


bool mucomvm::GetChMute(int ch)
{
	return (chmute[ch] != 0);
}


int mucomvm::GetChStatus(int ch)
{
	return (int)chstat[ch];
}


void mucomvm::SetChMuteAll(bool sw)
{
	int i;
	for (i = 0; i < OPNACH_MAX; i++) {
		SetChMute(i, sw);
	}
}


/*------------------------------------------------------------*/
/*
ADPCM
*/
/*------------------------------------------------------------*/

int mucomvm::ConvertWAVtoADPCMFile(const char *fname, const char *sname)
{
	Adpcm adpcm;
	DWORD dAdpcmSize;
	BYTE *dstbuffer;
	int sz, res;

	char *buf;
	buf = LoadAlloc(fname, &sz);
	if (buf == NULL) return -1;


	dstbuffer = adpcm.waveToAdpcm(buf, sz, dAdpcmSize, 16000 );
	LoadAllocFree(buf);

	if (dstbuffer == NULL) return -1;
	res = (int)dAdpcmSize;

	FILE *fp;
	fp = fopen(sname, "wb");
	if (fp != NULL) {
		fwrite(dstbuffer, 1, res, fp);
		fclose(fp);
	}
	else {
		res = -1;
	}
	delete[] dstbuffer;
	return res;
}


/*------------------------------------------------------------*/
/*
Channel Data
*/
/*------------------------------------------------------------*/

void mucomvm::InitChData(int chmax, int chsize)
{
	if (pchdata) {
		free(pchdata);
	}
	channel_max = chmax;
	channel_size = chsize;
	pchdata = (uint8_t *)malloc(channel_max*channel_size);
	memset(pchdata, 0, channel_max*channel_size);
}


void mucomvm::SetChDataAddress(int ch, int adr)
{
	if ((ch < 0) || (ch >= channel_max)) return;
	pchadr[ch] = (uint16_t)adr;
}


uint8_t *mucomvm::GetChData(int ch)
{
	uint8_t *p = pchdata;
	if ((ch < 0) || (ch >= channel_max)) return NULL;
	if (pchdata) {
		return p + (ch*channel_size);
	}
	return NULL;
}


uint8_t mucomvm::GetChWork(int index)
{
	if ((index < 0) || (index >= 16)) return 0;
	return pchwork[index];
}


void mucomvm::ProcessChData(void)
{
	int i;
	uint8_t *src;
	uint8_t *dst;
	uint8_t code;
	uint8_t keyon;
	if (pchdata==NULL) return;
	if (channel_max == 0) return;
	for (i = 0; i < channel_max; i++){
		src = mem + pchadr[i];
		dst = pchdata + (channel_size*i);
		if (src[0] > dst[0]) {						// lebgthが更新された
			keyon = 0;
			code = src[32];
			if (i == 10) {
				keyon = src[1];						// PCMchは音色No.を入れる
			}
			else if ( i == 6) {
				keyon = mem[ pchadr[10] + 38 ];		// rhythmのワーク値を入れる
			}
			else {
				if (code != dst[32]) {
					keyon = ((code >> 4) * 12) + (code & 15);	// 正規化したキーNo.
				}

			}
			src[37] = keyon;						// keyon情報を付加する
		}
		else {
			if (src[0] < src[18]) {				// quantize切れ
				src[37] = 0;						// keyon情報を付加する
			}
		}
		memcpy(dst, src , channel_size);
	}
	memcpy( pchwork, mem + pchadr[0] - 16, 16 );	// CHDATAの前にワークがある
}


/*------------------------------------------------------------*/
/*
plugin interface
*/
/*------------------------------------------------------------*/

int MUCOM88IF_VM_COMMAND(void *ifptr, int cmd, int prm1, int prm2, void *prm3, void *prm4);
int MUCOM88IF_EDITOR_COMMAND(void *ifptr, int cmd, int prm1, int prm2, void *prm3, void *prm4);

int mucomvm::AddPlugins(const char *filename, int bootopt)
{
	//		プラグインを追加する
	//		filename = プラグインDLL名
	//		bootopt = 起動オプション(未使用)
	//		終了コード : 0=OK
	//
#ifndef USE_SDL
	int res;
	Mucom88Plugin *plg = new Mucom88Plugin;
	plg->if_mucomvm = (MUCOM88IF_COMMAND)MUCOM88IF_VM_COMMAND;
	plg->if_editor = (MUCOM88IF_COMMAND)MUCOM88IF_EDITOR_COMMAND;
	plg->mucom = p_cmucom;
	plg->vm = this;
	plg->hwnd = master_window;
	strncpy( plg->filename, filename, MUCOM88IF_FILENAME_MAX-1 );
	plugins.push_back(plg);
	res = osd->InitPlugin(plg, filename, bootopt);
	if (res) return res;
	plg->if_notice(plg, MUCOM88IF_NOTICE_BOOT,NULL,NULL);				// 初期化を通知する
#endif
	return 0;
}


void mucomvm::FreePlugins(void)
{
	//		プラグインをすべて破棄する
	//
// C++11ではOS X 10.6用ビルドが通らないので…。
#ifndef USE_SDL
	Mucom88Plugin *plg;
	for (auto it = begin(plugins); it != end(plugins); ++it) {
		plg = *it;
		plg->if_notice(plg, MUCOM88IF_NOTICE_TERMINATE, NULL, NULL);	// 破棄する前に通知する
		osd->FreePlugin(plg);
	}
	plugins.clear();			// すべて削除
#endif
}


void mucomvm::NoticePlugins(int cmd, void *p1, void *p2)
{
#ifndef USE_SDL
	Mucom88Plugin *plg;
	for (auto it = begin(plugins); it != end(plugins); ++it) {
		plg = *it;
		plg->if_notice(plg, cmd, p1, p2);
	}
#endif
}


