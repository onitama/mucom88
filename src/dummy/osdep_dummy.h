// OsDependent 
// BouKiCHi 2019

#ifndef _OS_DEP_DUMMY_H_
#define _OS_DEP_DUMMY_H_

#include <stdint.h>
#include "../osdep.h"

class OsDependentDummy : public OsDependent {
public:
	OsDependentDummy();
	~OsDependentDummy();

	// COMの初期化(最初の１回のみ)
	bool CoInitialize();

	// サウンド
	bool InitAudio(void *hwnd, int Rate, int BufferSize);
	void FreeAudio();
	bool SendAudio(int ms);
	void WaitSendingAudio();

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

	// ファイル操作関連
	int GetDirectory(char *buf, int size);
	int ChangeDirectory(const char *dir);
	int KillFile(const char *filename);
};

#endif
