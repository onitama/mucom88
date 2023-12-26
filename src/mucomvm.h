
//
//	cmucomvm.cpp structures
//
#ifndef __mucomvm_h
#define __mucomvm_h

#include <stdio.h>
#include "Z80/Z80.h"
#include "fmgen/opna.h"

#include "mucom88config.h"
#include "osdep.h"
#include "membuf.h"

#include "utils/logwrite.h"
#include "utils/wavwrite.h"

#include "codeconv.h"

//#define DEBUGZ80_TRACE

enum {
	VMFLAG_NONE, VMFLAG_EXEC, VMFLAG_HALT
};

#define VM_OPTION_FMMUTE 1
#define VM_OPTION_SCCI 2
#define VM_OPTION_FASTFW 4
#define VM_OPTION_STEP 8

#define VMBANK_MAX 4
#define VMBANK_SIZE 0x4000

#define EXTRAM_SIZE 0x8000

#define VMPRGBANK_MAIN 0
#define VMPRGBANK_SHADOW 1

#define OPNACH_MAX 16
#define OPNACH_FM 0
#define OPNACH_PSG 6
#define OPNACH_RHYTHM 9
#define OPNACH_ADPCM 10
#define OPNAREG_MAX 0x200

// CULC関連のアドレス
#define MUCOM_EM_CULC 0xBC92
#define MUCOM_EM_CULLP2 0xBCA4
#define MUCOM_EM_FRQBEF 0xBD0C

class mucomvm : public Z80 {
public:
	mucomvm();
	~mucomvm();
	void ResetMessageBuffer(void);

	//		Z80コントロール
	void InitSoundSystem(int Rate);
	void SetOption(int option);
	int GetOption(void) { return m_option; }
	void SetPC(uint16_t adr);
	void CallAndHalt(uint16_t adr);
	int CallAndHalt2(uint16_t adr, uint8_t code);
	int CallAndHaltWithA(uint16_t adr, uint8_t areg);
	int CallAndHaltWithB(uint16_t adr, uint8_t breg);
	int ExecUntilHalt(int times = 0x10000);
	void ExecuteCLUC();
	void ExecuteModCLUC();
	void SendMemoryToShadow(void);

	//		仮想マシンコントロール
	void Reset(void);
	void ResetExtRam();
	void ResetFM(void);
	int LoadPcm(const char *fname, int maxpcm = 32);
	int LoadPcmFromMem(const char *buf, int sz, int maxpcm = 32);
	int PeekTableWord(const unsigned char* table, int adr);
	int LoadMem(const char *fname, int adr, int size);
	int SendMem(const unsigned char *src, int adr, int size);
	int SendExtMem(const unsigned char* src, int bank, int adr, int size);
	int RecvMem(unsigned char* mem, int adr, int size);
	int SaveMem(const char *fname,int adr, int size);
	int SaveMemExpand(const char *fname, int adr, int size, char *header, int hedsize, char *footer, int footsize, char *pcm, int pcmsize);
	int StoreMemExpand(CMemBuf *buf, int adr, int size, char *header, int hedsize, char *footer, int footsize, char *pcm, int pcmsize);
	char *LoadAlloc(const char *fname, int *sizeout);
	void LoadAllocFree(char *ptr);
	int SaveToFile(const char *fname, const unsigned char *src, int size);
	void SetVolume(int fmvol, int ssgvol);
	void SetFastFW(int value);
	void SkipPlay(int count);
	void PlayLoop(void);

	//		ファイルコントロール
	int GetDirectory(char *buf, int size);
	int ChangeDirectory(const char *dir);
	int KillFile(const char *fname);

	//		仮想マシンステータス
	uint8_t *GetMemoryMap(void) { return mem; }
	int GetFlag(void) { return m_flag; }
	int Peek(uint16_t adr);
	int Peekw(uint16_t adr);
	void Poke(uint16_t adr, uint8_t data);
	void FillMem(uint16_t adr, uint8_t value, uint16_t length);
	void Pokew(uint16_t adr, uint16_t data);
	void PeekToStr(char *out, uint16_t adr, uint16_t length);
	void BackupMem(uint8_t *mem_bak);
	void RestoreMem(uint8_t *mem_bak);

	void ResetTimer(void);
	void StartINT3(void);
	void StopINT3(void);
	void RestartINT3(void);
	void SetWindow(void *window) { master_window = window; }
	void SetIntCount(int value) { time_intcount = value; }
	int GetIntCount(void) { return time_intcount; }
	int GetMasterCount(void) { return time_master; }
	int GetPassTick(void) { return pass_tick; }
	int GetAudioOutputMs(void) { return audio_output_ms; }

	void WaitReady(void);
	int GetDriverStatus(int option);

	//		YM2608ステータス
	void FMRegDataOut(int reg, int data);
	int FMRegDataGet(int reg);
	void SetChMuteAll(bool sw);
	void SetChMute(int ch, bool sw);
	bool GetChMute(int ch);
	int GetChStatus(int ch);
	uint8_t *GetRegisterMap(void) { return regmap;  }
	bool GetSB2Present(void);

	//		デバッグ用
	void Msgf(const char *format, ...);
	void MsgfNoConvert(const char *format, ...);
	char *GetMessageBuffer(void) { return membuf->GetBuffer(); }
	int GetMessageBufferSize(void) { return membuf->GetSize(); }
	void DumpBin(uint16_t adr, uint16_t length);
	int DeviceCheck(void);
	int GetMessageId() { return msgid; }

	//		ADPCM用
	int ConvertWAVtoADPCMFile(const char *fname, const char *sname);

	//		BANK切り替え
	void ClearBank(void);
	void ChangeBank(int bank);

	void CopyMemToVm(const uint8_t * src, int address, int length);
	void CopyMemToExtRam(const uint8_t* src, int bank, int address, int length);
	void CopyMemFromVm(uint8_t * dest, int address, int length);

	// 拡張RAMのバンク番号
	int GetExtRamBank();

	// 拡張RAMのバンク番号の設定
	void SetExtRamBank(uint8_t bank);

	// 拡張RAMの現在のモード
	int GetExtRamMode();

	// バンク切り替え 
	// mode : 0x11 = 読み書き 0x00 = 無効(メインメモリになる)
	void ChangeExtRam(uint8_t mode, uint8_t bank);
	void ChangeExtRamMode(uint8_t mode);

	// オリジナル/拡張モードの設定
	void SetOrignalMode(bool mode=true);

	// その他処理
	void ConvertVoice();

	//		CHDATA用
	void InitChData(int chmax, int chsize);
	void SetChDataAddress(int ch, int adr);
	uint8_t *GetChData(int ch);
	uint8_t GetChWork(int index);
	void ProcessChData(void);

	//		オーディオ書き込み
	void RenderAudio(void *mix, int size);
	void AudioCallback(void *mix, int size);
	void UpdateTime(int tick);
	void UpdateCallback(int tick);

	//		プラグインコントロール
	void SetMucomInstance(CMucom *mucom);
	int AddPlugins(const char *filename, int bootopt);
	int DeletePlugins(const char* filename);
	void FreePlugins(void);
	void NoticePlugins(int cmd, void *p1 = NULL, void *p2=NULL);

	// 音源ログ設定
	void SetLogWriter(ILogWrite *log);
	void SetWavWriter(WavWriter* wav);

	// データ取得
	void GetFMRegMemory(unsigned char* data, int address, int length);
	void GetMemory(unsigned char *data, int address, int length);
	void GetMainMemory(unsigned char* data, int address, int length);
	void GetExtMemory(unsigned char* data, int bank, int address, int length);

	void SetMainMemory(unsigned char* data, int address, int length);
	void SetMemory(unsigned char* data, int address, int length);
	void SetExtMemory(unsigned char* data, int bank, int address, int length);

	CodeConvert *Conv;

private:
	//		Z80
	int32_t load(uint16_t adr);
	int32_t loadpc(uint16_t adr);
	void store(uint16_t adr, uint8_t data);
	int32_t input(uint16_t adr);
	void output(uint16_t adr, uint8_t data);

	void Halt(void);

	//		メモリ(64K)
	int m_flag;
	int m_option;						// 動作オプション
	int m_fastfw;						// 早送りカウント
	int m_fmvol, m_ssgvol;				// ボリューム
	int bankmode;						// BANK(3=MAIN/012=BGR)
	int bankprg;						// BANK(0=MAIN/1=シャドーコピー側)
	uint8_t mem[0x10000];				// メインメモリ
	uint8_t vram[VMBANK_MAX][0x4000];	// GVRAM(3:退避/012=BRG)
	uint8_t memprg[0x10000];			// メインメモリ(プログラム実行用のシャドーコピー)
	uint8_t extram[4][0x8000];			// 拡張メモリ

	// バンク機能
	uint8_t *membank[0x10];
	uint8_t* membank_wr[0x10];

	// 拡張RAM関連
	bool original_mode;
	uint8_t extram_bank_mode;
	uint8_t extram_bank_no;
	void ChangeExtRamBank(uint8_t bank);


	//		音源
	FM::OPNA *opn;

	FILE *trace_fp;

	int sound_reg_select;
	int sound_reg_select2;
	void FMOutData(int data);
	void TraceLog(int data);
	void FMOutData2(int data);
	int FMInData();
	int FMInData2();

	//		OPNA情報スタック
	int Rate;
	uint8_t chmute[OPNACH_MAX];
	uint8_t chstat[OPNACH_MAX];
	uint8_t regmap[OPNAREG_MAX];

	//		CH情報スタック
	int channel_max, channel_size;
	uint16_t pchadr[64];
	uint8_t *pchdata;
	uint8_t pchwork[16];

	//		割り込みタイマー関連
	int time_master;
	int time_scount;
	int time_intcount;
	int pass_tick;
	int last_tick;
	int audio_output_ms;

	bool playflag;
	bool busyflag;
	bool int3flag;
	bool tmflag;
	int predelay;
	int int3mask;
	int msgid;

	void checkThreadBusy(void);

	//		メッセージバッファ
	CMemBuf *membuf;

	//		OS依存部分
	OsDependent *osd;
	void *master_window;

	// プラグイン拡張(内部用)
	std::vector<Mucom88Plugin *> plugins;

	//		親のインスタンス(参照)
	CMucom *p_cmucom;

	ILogWrite *p_log;

	WavWriter *p_wav;
};



#endif
