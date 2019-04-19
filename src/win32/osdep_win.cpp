
//
//		OsDependent win32
//		BouKiCHi 2019
//		(OS依存のルーチンをまとめます)
//		onion software/onitama 2019/1
//

#include <stdio.h>
#include <windows.h>
#include "osdep_win.h"
#include "realchip.h"
#include "soundds.h"

#define USE_SCCI
#define USE_HIGH_LEVEL_COUNTER

/*-------------------------------------------------------------------------------*/

//
//		windows debug support
//
void Alertf(const char *format, ...)
{
	char textbf[4096];
	va_list args;
	va_start(args, format);
	vsprintf(textbf, format, args);
	va_end(args);
	MessageBox(NULL, textbf, "error", MB_ICONINFORMATION | MB_OK);
}

/*-------------------------------------------------------------------------------*/

OsDependentWin32::OsDependentWin32(void)
{
	snddrv = NULL;
	master_window = NULL;
	RealChipInstance = NULL;

	MuteAudio = false;
	sending = false;
	threadflag = false;

	UserTimerCallback = new TimerCallback;
	UserAudioCallback = new AudioCallback;
}

OsDependentWin32::~OsDependentWin32(void)
{
	if (UserTimerCallback) delete UserTimerCallback;
	if (UserAudioCallback) delete UserAudioCallback;
	if (snddrv != NULL) delete snddrv;
}

// COMの初期化
bool OsDependentWin32::CoInitialize() {
	if (FAILED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED))) return false;
	return true;
}


// 実チップ
bool OsDependentWin32::InitRealChip() {
	realchip *rc = new realchip();
	rc->Initialize();
	if (!rc->IsRealChip()) {
		RealChipInstance = NULL;			// 初期化に失敗したら使用しない
		return false;
	}
	RealChipInstance = rc;
	return true;
}

void OsDependentWin32::FreeRealChip() {
	if (!RealChipInstance) return;
	realchip *rc = (realchip *)RealChipInstance;
	rc->UnInitialize();
	delete rc;
	RealChipInstance = NULL;
}


int OsDependentWin32::CheckRealChip() {
	if (!RealChipInstance) return -1;
	return 0;
}

void OsDependentWin32::ResetRealChip() {
	if (!RealChipInstance) return;
	realchip *rc = (realchip *)RealChipInstance;
	rc->Reset();
}


void OsDependentWin32::OutputRealChip(unsigned int Register, unsigned int Data) {
	if (!RealChipInstance) return;
	realchip *rc = (realchip *)RealChipInstance;

	rc->SetRegister(Register, Data);
}

void OsDependentWin32::OutputRealChipAdpcm(void *pData, int size)
{
	if (!RealChipInstance) return;
	if (CheckRealChip()) return;
	realchip *rc = (realchip *)RealChipInstance;

	rc->SendAdpcmData(pData, (DWORD)size);
}


// タイマー
bool OsDependentWin32::InitTimer() {

	timerid = 0;

	TIMECAPS caps;
	if (timeGetDevCaps(&caps, sizeof(TIMECAPS)) == TIMERR_NOERROR) {
		// マルチメディアタイマーのサービス精度を最大に
		HANDLE myth = GetCurrentThread();
		SetThreadPriority(myth, THREAD_PRIORITY_HIGHEST);

		timer_period = caps.wPeriodMin;
		timeBeginPeriod(timer_period);
		timerid = timeSetEvent(timer_period, caps.wPeriodMin, TimeProc, reinterpret_cast<DWORD_PTR>(this), (UINT)TIME_PERIODIC);
		if (!timerid)
		{
			timeEndPeriod(timer_period);
		}
	}
	else {
		//	失敗した時
		timer_period = -1;
		timerid = 0;
		MessageBox(NULL, "Unable to start timer.", "Error", 0);
	}

	return false;
}

void OsDependentWin32::FreeTimer() {

	if (timerid)
	{
		timeKillEvent(timerid);
		timeEndPeriod(timer_period);
		timerid = 0;
	}
	//	スレッドを停止する
	StopThread();
}

static void CALLBACK TimerProc(UINT uid, UINT, DWORD_PTR user, DWORD_PTR, DWORD_PTR) {
	if (!user) return;
	OsDependent* inst = (OsDependent *)(user);
	inst->UpdateTimer();
}

void OsDependentWin32::UpdateTimer()
{
	int tick = GetElapsedTime();
	UserTimerCallback->tick = tick;
	UserTimerCallback->Run();
}

// 時間
int OsDependentWin32::GetMilliseconds() {
	return timeGetTime();
}

void OsDependentWin32::Delay(int ms) {
	Sleep(ms);
}

int OsDependentWin32::StartThread(void)
{
	// ストリームスレッドを開始する

	// イベントオブジェクトを作成する
	hevent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// スレッドを生成する
	hthread = CreateThread(NULL, 0x40000, (LPTHREAD_START_ROUTINE)OsDependentWin32::vThreadFunc, (LPVOID)this, 0, &threadid);
	if (hthread == NULL) {
		return -1;
	}
	// スレッドの優先順位を変更する
	SetThreadPriority(hthread, THREAD_PRIORITY_TIME_CRITICAL);
	return 0;
}


DWORD WINAPI OsDependentWin32::vThreadFunc(LPVOID pParam) {
	OsDependentWin32 *pThis = (OsDependentWin32 *)pParam;
	pThis->ThreadFunc();
	return 0;
}

int OsDependentWin32::StopThread(void)
{
	// ストリームスレッドを停止する
	if (threadflag == FALSE) return -1;

	// イベントオブジェクト破棄
	threadflag = FALSE;
	SetEvent(hevent);
	CloseHandle(hevent);
	hevent = NULL;
	// スレッド停止を待つ
	DWORD	dActive = 0;
	GetExitCodeThread(hthread, &dActive);
	if (dActive == STILL_ACTIVE) {
		WaitForSingleObject(hthread, INFINITE);
	}
	// スレッド破棄
	CloseHandle(hthread);

	hthread = NULL;
	threadid = 0;

	return 0;
}


void OsDependentWin32::ResetTime()
{
#ifdef USE_HIGH_LEVEL_COUNTER
	GetSystemTimeAsFileTime((FILETIME *)&last_ft);
#else
	last_tick = timeGetTime();
#endif
}


int OsDependentWin32::GetElapsedTime()
{
#ifdef USE_HIGH_LEVEL_COUNTER

	int64_t cur_ft;		// 100ナノ秒単位の時間
	int64_t pass_ft;
	double ms;
	GetSystemTimeAsFileTime((FILETIME *)&cur_ft);
	pass_ft = cur_ft - last_ft;
	last_ft = cur_ft;

	ms = ((double)pass_ft) * (0.0001 * TICK_FACTOR);	// 1ミリ秒=1024 単位に直す
	//printf( "(%f)\n",ms );

	return (int)(ms);

#else
	int curtime;
	curtime = timeGetTime();
	pass_tick = curtime - last_tick;
	last_tick = curtime;
	return 1024 * pass_tick;
#endif
}


void OsDependentWin32::ThreadFunc() {
	// ストリームスレッドループ
	threadflag = true;
	while (threadflag) {
		Sleep(20);
		//if (WaitForSingleObject(hevent, 20) == WAIT_TIMEOUT) {
		//	continue;
		//}
		//StreamSend(16);
		snddrv->PrepareSend();
		snddrv->Send();
	}
}

//  TimeProc
//
void CALLBACK OsDependentWin32::TimeProc(UINT uid, UINT, DWORD_PTR user, DWORD_PTR, DWORD_PTR)
{
	OsDependentWin32* inst = reinterpret_cast<OsDependentWin32*>(user);
	if (inst){
		//SetEvent(inst->hevent);
		inst->UpdateTimer();
	}
}

// オーディオ
bool OsDependentWin32::InitAudio(void *hwnd, int Rate, int BufferSize) {

	snddrv = new WinSoundDriver::DriverDS;
	if (hwnd) snddrv->SetHWND((HWND)hwnd);
	if (!snddrv->Init(Rate, 2, BufferSize)) return false;

	SamplePerTick = ((double)Rate / 1000);
	UpdateSamples = 0.0;
	TotalTick = 0;

	//		先行するサウンドバッファを作っておく
	//
	presize = Rate * 40 / 1000;
	snddrv->GetSoundBuffer()->PrepareBuffer(presize*2);
	snddrv->GetSoundBuffer()->UpdateBuffer(presize*2);
	//pooltime = snddrv->GetSoundBuffer()->GetPoolSize();

	//		タイマー初期化
	//
	//	ストリーム用スレッド
	if (RealChipInstance == NULL) {
		StartThread();
	}

	return true;
}

void OsDependentWin32::FreeAudio() {
	if (!snddrv) return;
	WaitSendingAudio();
	delete snddrv;
	snddrv = NULL;
}

void OsDependentWin32::WaitSendingAudio() {
	for (int i = 0; i < 300 && sending; i++) Sleep(10);
}

bool OsDependentWin32::SendAudio(int ms)
{
	StreamSend(ms);
	//SetEvent(hevent);
	return true;
}

void OsDependentWin32::StreamSend(int ms)
{
	if (!snddrv) return;

	// 0以外はスレッドが重複しているので続行しない。
	int ret = InterlockedExchange(&sending, 1);
	if (ret != 0) {
		return;
	}

	int writelength;
	int needlength;

	UpdateSamples = (ms * SamplePerTick );
	writelength = ((int)UpdateSamples);
	needlength =  (( snddrv->GetSoundBuffer()->GetReadSize() >>1 )+ writelength + presize) - (snddrv->GetSoundBuffer()->GetWriteSize() >> 1);
	if (writelength < needlength) writelength = needlength;

	//UpdateSamples -= writelength;
	TotalTick += writelength;

	//UpdateSamples = (ms * SamplePerTick * sizeof(short) * 2 );
	//writelength = (int)UpdateSamples;

	if (writelength) {
		int32 *smp;
		int size = writelength;
		smp = snddrv->GetSoundBuffer()->PrepareBuffer(size*2);

		if (!MuteAudio) {
			UserAudioCallback->mix = smp;
			UserAudioCallback->size = size;
			UserAudioCallback->Run();
		}

		snddrv->GetSoundBuffer()->UpdateBuffer(size*2);
	}
	//writelength = snddrv->PrepareSend();
	//snddrv->Send();

	// 終了
	InterlockedExchange(&sending, 0);
	return;
}



//	MUCOM88Winプラグイン処理用
//
int OsDependentWin32::InitPlugin(Mucom88Plugin *plg, const char *filename, int bootopt)
{
	//		DLLをリンクして初期化する
	//
	HMODULE	hInst = NULL;
	plg->instance = NULL;

	//	DLL読み込み
	hInst = ::LoadLibrary(filename);
	if (hInst == NULL) {
		return GetLastError();
	}
	//	初期化関数アドレス取得
	MUCOM88IF_STARTUP initalize = (MUCOM88IF_STARTUP)(::GetProcAddress(hInst, "InitalizePlugin"));

	if (initalize == NULL) {
		::FreeLibrary(hInst);
		return GetLastError();
	}

	//	初期化する
	return initalize(plg,bootopt);
}


void OsDependentWin32::FreePlugin(Mucom88Plugin *plg)
{
	//		DLLを解放する
	//
	HMODULE	hInst = (HMODULE)plg->instance;
	if (hInst == NULL) return;

	::FreeLibrary(hInst);
	plg->instance = NULL;
}


int OsDependentWin32::ExecPluginVMCommand(Mucom88Plugin *plg, int, int, int, void *, void *)
{
	//		OS依存のプラグインVMコマンド処理
	//
	return 0;
}


int OsDependentWin32::ExecPluginEditorCommand(Mucom88Plugin *plg, int, int, int, void *, void *)
{
	//		OS依存のプラグインエディタコマンド処理
	//
	return 0;
}


int OsDependentWin32::GetDirectory(char *buf, int size)
{
	//		OS依存のフォルダ名取得
	//
	if (GetCurrentDirectory(size, buf) == 0) return -1;
	return 0;
}


int OsDependentWin32::ChangeDirectory(const char *dir)
{
	//		OS依存のフォルダ移動
	//
	if (SetCurrentDirectory(dir) == 0) return -1;
	return 0;
}


int OsDependentWin32::KillFile(const char *filename)
{
	//		OS依存のファイル削除
	//
	if (remove(filename)) return -1;
	return 0;
}
