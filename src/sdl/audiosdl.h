#ifndef _AUDIO_SDL_H_
#define _AUDIO_SDL_H_

#include <stdio.h>
#include "audiobuffer.h"
#include "audiotime.h"
#include "callback.h"

class AudioSdl {
public:
    AudioSdl();
    ~AudioSdl();

    bool Open(int rate);
    void Close();

    void AudioMain(short *buffer, int size);

    void InitAudioTimer();
    void UpdateAudioTimer();
    int GetUpdateSamples(int tick);
    void UpdateSamples(int Samples);

    AudioBuffer *Buffer;
    AudioTimeInfo *Time;
    
    bool AudioOpenFlag;

    AudioCallback *UserAudioCallback;

private:
};

#endif