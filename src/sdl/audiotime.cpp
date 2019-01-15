// audiotime.cpp 
// BouKiCHi 2019
// SDL依存

#include <SDL.h>
#include "audiotime.h"

AudioTimeInfo::AudioTimeInfo() {
    ResetTick();
}

// 経過時間のリセット
void AudioTimeInfo::ResetTick() {
    LastTick = SDL_GetTicks();
}

// 経過時間(ms)の取得
int AudioTimeInfo::GetUpdateTick() {
    int CurrentTick = SDL_GetTicks();
    int UpdateTick = CurrentTick - LastTick;
    LastTick = CurrentTick;
    return UpdateTick;
}
