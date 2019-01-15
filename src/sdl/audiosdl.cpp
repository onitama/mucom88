// audiosdl.cpp 
// BouKiCHi 2019
// SDL使用

#include <SDL.h>
#include <string.h>
#include "audiosdl.h"

#define PULSE_MAX 100
#define PULSE_VALUE 10000

#define AUDIO_BUFFER_BLOCK 2048
#define AUDIO_BUFFER_SIZE (AUDIO_BUFFER_BLOCK * 8)
#define AUDIO_CHANNELS 2

// Win32では10ms以下にはならないので注意
#define TIMER_INTERVAL 10

static void SdlAudioCallback(void *param, Uint8 *data, int len);
static Uint32 SdlTimerCallback(Uint32 interval, void *param);

AudioSdl::AudioSdl() {
    Time = new AudioTimeInfo();
    Buffer = new AudioBuffer(AUDIO_CHANNELS, AUDIO_BUFFER_SIZE, AUDIO_BUFFER_BLOCK);
    UserAudioCallback = new AudioCallback;
    AudioOpenFlag = false;
}

AudioSdl::~AudioSdl() {
    delete Buffer;
    delete Time;
    delete UserAudioCallback;
}

// オーディオ開始
bool AudioSdl::Open(int rate) {
    Buffer->SendBuffer = false;
    Buffer->Reset();

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        printf("Failed to Initialize!!\n");
        return false;
    }

    SDL_AudioSpec af;
    af.freq     = rate;
    af.format   = AUDIO_S16;
    af.channels = AUDIO_CHANNELS;
    af.samples  = AUDIO_BUFFER_BLOCK / 2;
    af.callback = SdlAudioCallback;
    af.userdata = this;

    // 1msあたりのサンプル数
    Buffer->SetRate(rate);

    if (SDL_OpenAudio(&af, NULL) < 0) {
        printf("Audio Error!!\n");
        return false;
    }

    AudioOpenFlag = true;
    InitAudioTimer();

    SDL_PauseAudio(0);
    return true;
}

void AudioSdl::Close() {
    if (AudioOpenFlag) SDL_CloseAudio();
    SDL_Quit();
    AudioOpenFlag = false;
}


// タイマー初期化
void AudioSdl::InitAudioTimer() {
    Time->ResetTick();

    SDL_AddTimer(TIMER_INTERVAL, SdlTimerCallback, this);
}

// タイマー
static Uint32 SdlTimerCallback(Uint32 interval, void *param) {
    AudioSdl *inst = (AudioSdl*)param;
    inst->UpdateAudioTimer();
    return(interval);
}


// オーディオ更新
void AudioSdl::UpdateAudioTimer() {

    int s = Buffer->GetLeft();

    //　バッファ送出を開始
    if (s == 0) {
        Buffer->SendBuffer = true;
        return;
    }

    int UpdateTick = Time->GetUpdateTick();
    int u = Buffer->TickToSamples(UpdateTick);

    if (s < u) s = u;
    if (AUDIO_BUFFER_BLOCK < s) s = AUDIO_BUFFER_BLOCK;

    UpdateSamples(s);
}

// オーディオデータ作成後に更新
void AudioSdl::UpdateSamples(int Samples) {
    // short型です
    short buf[AUDIO_BUFFER_BLOCK];

    UserAudioCallback->mix = buf;
    UserAudioCallback->size = Samples/2;
    UserAudioCallback->Run();

    // バッファ書き込み
    Buffer->Write(buf,Samples/2);
} 

// オーディオコールバック
static void SdlAudioCallback(void *param, Uint8 *data, int len) {
    AudioSdl *inst = (AudioSdl *)param;
    inst->AudioMain((short *)data, len / 4);
}

// オーディオ処理メイン
void AudioSdl::AudioMain(short *buffer, int frames) {
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


