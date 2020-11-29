// OsDependent SDL
// BouKiCHi 2019

#include <SDL.h>
#include <stdio.h>
#include "osdep_sdl.h"

#define AUDIO_BUFFER_BLOCK 2048
#define AUDIO_BUFFER_SIZE (AUDIO_BUFFER_BLOCK * 8)
#define AUDIO_CHANNELS 2

// Win32では10ms以下にはならないので注意
#define TIMER_INTERVAL 10

static void SdlAudioCallback(void *param, Uint8 *data, int len);
static Uint32 SdlTimerCallback(Uint32 interval, void *param);

OsDependentSdl::OsDependentSdl() {
    Time = new AudioTimeInfo();
    Buffer = new AudioBuffer(AUDIO_CHANNELS, AUDIO_BUFFER_SIZE, AUDIO_BUFFER_BLOCK);
    AudioOpenFlag = false;

	UserTimerCallback = new TimerCallback;
	UserAudioCallback = new AudioCallback;

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf("Failed to Initialize SDL!!\n");
    }
}

OsDependentSdl::~OsDependentSdl() {
    delete Buffer;
    delete Time;
	if (UserTimerCallback) delete UserTimerCallback;
	if (UserAudioCallback) delete UserAudioCallback;
}

bool OsDependentSdl::CoInitialize() {
	return true;
}

// オーディオ
bool OsDependentSdl::InitAudio(void *hwnd, int Rate, int BufferSize) {
    Buffer->SendBuffer = false;
    Buffer->Reset();


    SDL_AudioSpec af;
    af.freq     = Rate;
    af.format   = AUDIO_S16;
    af.channels = AUDIO_CHANNELS;
    af.samples  = AUDIO_BUFFER_BLOCK / 2;
    af.callback = SdlAudioCallback;
    af.userdata = this;

    // 1msあたりのサンプル数
    Buffer->SetRate(Rate);

    if (SDL_OpenAudio(&af, NULL) < 0) {
        printf("Audio Error!!\n");
        return false;
    }

    AudioOpenFlag = true;

    SDL_PauseAudio(0);
    return true;
}

void OsDependentSdl::FreeAudio() {
    if (AudioOpenFlag) SDL_CloseAudio();
    SDL_Quit();
    AudioOpenFlag = false;
}

// タイマー
static Uint32 SdlTimerCallback(Uint32 interval, void *param) {
    OsDependentSdl *inst = (OsDependentSdl*)param;
    inst->UpdateTimer();
    return(interval);
}

bool OsDependentSdl::SendAudio(int ms) {

    int s = Buffer->GetLeft();

    //　バッファ送出を開始
    if (s == 0) {
        Buffer->SendBuffer = true;
        return true;
    }

    int UpdateTick = ms;
    int u = Buffer->TickToSamples(UpdateTick);

    if (u < s) s = u;
    if (AUDIO_BUFFER_BLOCK < s) { s = AUDIO_BUFFER_BLOCK; }
    if (s > 0) {
        Buffer->ClearTick();
        int smp[AUDIO_BUFFER_BLOCK];
        memset(smp,0,sizeof(int) * AUDIO_BUFFER_BLOCK);
        UserAudioCallback->mix = smp;
        UserAudioCallback->size = s/2;
        UserAudioCallback->Run();

        Buffer->Write(smp,s/2);
    }
    return true;
}

// オーディオコールバック
static void SdlAudioCallback(void *param, Uint8 *data, int len) {
    OsDependentSdl *inst = (OsDependentSdl *)param;
    inst->AudioMain((short *)data, len / 4);
}


// オーディオ処理メイン
void OsDependentSdl::AudioMain(short *buffer, int frames) {
    int Samples = frames * AUDIO_CHANNELS;
    if (!Buffer->SendBuffer) {
        memset(buffer, 0, Samples * sizeof(short));
        return;
    }

    // 出力
    int count = Buffer->WriteCount;
    int pos = Buffer->ReadPosition;
    short *input = Buffer->AudioData;

    bool Under = false;

    for(int i = 0; i < frames * 2; i++) {
        if (count <= 0) { Under = true; buffer[i] = 0; continue; }

        buffer[i] = input[pos++];
        count--;
        if (pos >= AUDIO_BUFFER_SIZE) pos = 0;
    }

    if (Under) Buffer->UnderCount++;

    Buffer->ReadPosition = pos;
    Buffer->WriteCount = count;
}



void OsDependentSdl::WaitSendingAudio() {
}

// 実チップ
bool OsDependentSdl::InitRealChip() {
	return true;
}

void OsDependentSdl::FreeRealChip() {
}


int OsDependentSdl::CheckRealChip() {
	return 0;
}

int OsDependentSdl::CheckRealChipSB2() {
	return 1;
}

void OsDependentSdl::ResetRealChip() {
}


void OsDependentSdl::OutputRealChip(unsigned int Register, unsigned int Data) {
}

void OsDependentSdl::OutputRealChipAdpcm(void *pData, int size) {
}

// タイマー
bool OsDependentSdl::InitTimer() {
    Time->ResetTick();
    TimerId = SDL_AddTimer(TIMER_INTERVAL, SdlTimerCallback, this);
	return true;
}

void OsDependentSdl::FreeTimer() {
    SDL_RemoveTimer(TimerId);
}

// オーディオ更新用タイマー
// tickはミリ秒*1024の単位
void OsDependentSdl::UpdateTimer() {
    // int UpdateTick = 10;
    int UpdateTick = Time->GetUpdateTick();
    int tick = UpdateTick * 1024;
	UserTimerCallback->tick = tick;
	UserTimerCallback->Run();
}

// タイマー初期化時に呼ばれる
void OsDependentSdl::ResetTime() {

}

// ミリ秒*1024の単位の経過時間
int OsDependentSdl::GetElapsedTime() {
	return 0;
}

// ミリ秒の経過時間
int OsDependentSdl::GetMilliseconds() {
	return 0;
}

// ミリ秒待つ(idle)
void OsDependentSdl::Delay(int ms) {
    SDL_Delay(ms);
}

// プラグイン拡張
int OsDependentSdl::InitPlugin(Mucom88Plugin *plg, const char *filename, int bootopt) {
	return 0;
}

void OsDependentSdl::FreePlugin(Mucom88Plugin *plg) {
}


int OsDependentSdl::ExecPluginVMCommand(Mucom88Plugin *plg, int, int, int, void *, void *)
{
	//		OS依存のプラグインVMコマンド処理
	//
	return 0;
}

int OsDependentSdl::ExecPluginEditorCommand(Mucom88Plugin *plg, int, int, int, void *, void *)
{
	//		OS依存のプラグインエディタコマンド処理
	//
	return 0;
}


int OsDependentSdl::GetDirectory(char *buf, int size)
{
	return 0;
}

int OsDependentSdl::ChangeDirectory(const char *dir)
{
	return 0;
}

int OsDependentSdl::KillFile(const char *filename)
{
	return 0;
}


bool OsDependentSdl::SetBreakHook()
{
	return true;
}

bool OsDependentSdl::GetBreakStatus()
{
	return false;
}

int OsDependentSdl::GetStatus(int option)
{
	// OsDep内部パラメーター読み込みhub
	//
	return 0;
}


