// audiobuffer.cpp 
// BouKiCHi 2019

#include <string.h>
#include "audiobuffer.h"

AudioBuffer::AudioBuffer(int channels, int bufferSize, int blockSize) {
    SendBuffer = false;

    UnderCount = 0;
    WritePosition = 0;
    WriteCount = 0;
    ReadPosition = 0;

    SamplePerTick = 0;
    UpdateSamples = 0;

    Channels = channels;
    BufferSize = bufferSize;
    BlockSize = blockSize;

    AudioData = new short[BufferSize];
    memset(AudioData, 0, sizeof(short) * BufferSize);
}

AudioBuffer::~AudioBuffer() {
    delete[] AudioData;
}

// リセット
void AudioBuffer::Reset() {
    WritePosition = 0;
    WriteCount = 0;
    ReadPosition = 0;
}

// バッファサイズ - 再生領域
int AudioBuffer::GetLeft() {
    return (BufferSize - BlockSize) - WriteCount;
}

// ミリ秒からサンプル数を作成する
int AudioBuffer::TickToSamples(int ms) {
    UpdateSamples += (ms * SamplePerTick * Channels);
    int s = (int)UpdateSamples;
    return s;
}

void AudioBuffer::ClearTick() {
    UpdateSamples -= (int)UpdateSamples;
}


// 1msあたりのサンプル数を計算する
void AudioBuffer::SetRate(int rate) {
    SamplePerTick = ((double)rate/1000);
}

// 事前に宣言しないと生成されません…。
template void AudioBuffer::Write<int*>(int *input,int frames);
template void AudioBuffer::Write<short*>(short *input,int frames);

// サンプルをバッファを書き込み
template <typename T> void AudioBuffer::Write(T input,int frames) {
    short *output = AudioData;

    int count = WriteCount;
    int pos = WritePosition;

    // int -> short
    for(int i=0; i < frames*2; i++) {
        int v=input[i];

        output[pos] = v > 32767 ? 32767 : (v < -32768 ? -32768 : v);
        pos++;
        count++;
        if (pos >= BufferSize) pos = 0;
    }

    WriteCount = count;
    WritePosition = pos;
}
