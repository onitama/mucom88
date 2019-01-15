#ifndef _AUDIO_TIME_H_
#define _AUDIO_TIME_H_

class AudioTimeInfo {
public:
    AudioTimeInfo();

    void ResetTick();
    int GetUpdateTick();
    
    int LastTick;
};

#endif