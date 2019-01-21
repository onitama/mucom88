// OsDependent SDL
// BouKiCHi 2019

#ifndef _OS_DEP_SDL_H_
#define _OS_DEP_SDL_H_


#include <stdint.h>
#include "../osdep.h"
#include "audiotime.h"
#include "audiobuffer.h"

class OsDependentSdl : public OsDependent {
public:
	OsDependentSdl();
	~OsDependentSdl();

	// COMの初期化(最初の１回のみ)
	bool CoInitialize();

	// サウンド
	bool InitAudio(void *hwnd, int Rate, int BufferSize);
	void FreeAudio();
	bool SendAudio(int ms);
	void WaitSendingAudio();
	void AudioMain(short *buffer, int size);


	// 実チップ
	bool InitRealChip();
	void FreeRealChip();
	void ResetRealChip();
	int CheckRealChip();
	void OutputRealChip(unsigned int Register, unsigned int Data);
	void OutputRealChipAdpcm(void *pData, int size);

	// タイマー
	bool InitTimer();
	void FreeTimer();
	void UpdateTimer();
	void ResetTime();
	int GetElapsedTime();

	// 時間
	int GetMilliseconds();
	void Delay(int ms);

	// プラグイン拡張
	int InitPlugin(Mucom88Plugin *plg, const char *filename, int bootopt);
	void FreePlugin(Mucom88Plugin *plg);
	int ExecPluginVMCommand(Mucom88Plugin *plg, int, int, int, void *, void *);
	int ExecPluginEditorCommand(Mucom88Plugin *plg, int, int, int, void *, void *);

    AudioBuffer *Buffer;
    AudioTimeInfo *Time;
    AudioTimeInfo *TimeRender;

    bool AudioOpenFlag;

private:
	void *TimerId;
};

#endif
