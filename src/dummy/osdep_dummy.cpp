// OsDependent 
// BouKiCHi 2019

#include <stdio.h>
#include "osdep_dummy.h"

OsDependentDummy::OsDependentDummy()
{
	UserTimerCallback = new TimerCallback;
	UserAudioCallback = new AudioCallback;
}

OsDependentDummy::~OsDependentDummy()
{
	if (UserTimerCallback) delete UserTimerCallback;
	if (UserAudioCallback) delete UserAudioCallback;
}

bool OsDependentDummy::CoInitialize() {
	return true;
}

// オーディオ
bool OsDependentDummy::InitAudio(void *hwnd, int Rate, int BufferSize) {
	return true;
}

void OsDependentDummy::FreeAudio() {
}

bool OsDependentDummy::SendAudio(int ms) {
	return true;
}

void OsDependentDummy::WaitSendingAudio() {
}

// 実チップ
bool OsDependentDummy::InitRealChip() {
	return true;
}

void OsDependentDummy::FreeRealChip() {
}


int OsDependentDummy::CheckRealChip() {
	return 0;
}

int OsDependentDummy::CheckRealChipSB2() {
	return 1;
}

void OsDependentDummy::ResetRealChip() {
}


void OsDependentDummy::OutputRealChip(unsigned int Register, unsigned int Data) {
}

void OsDependentDummy::OutputRealChipAdpcm(void *pData, int size) {
}

// タイマー
bool OsDependentDummy::InitTimer() {
	return false;
}

void OsDependentDummy::FreeTimer() {
}

void OsDependentDummy::UpdateTimer() {
}

void OsDependentDummy::ResetTime() {
}


int OsDependentDummy::GetElapsedTime() {
	return 0;
}

// 時間
int OsDependentDummy::GetMilliseconds() {
	return 0;
}

void OsDependentDummy::Delay(int ms) {
}

// プラグイン拡張
int OsDependentDummy::InitPlugin(Mucom88Plugin *plg, const char *filename, int bootopt) {
	return 0;
}
void OsDependentDummy::FreePlugin(Mucom88Plugin *plg) {
}
int OsDependentDummy::ExecPluginVMCommand(Mucom88Plugin *plg, int, int, int, void *, void *) {
	return 0;
}
int OsDependentDummy::ExecPluginEditorCommand(Mucom88Plugin *plg, int, int, int, void *, void *) {
	return 0;
}

int OsDependentDummy::GetDirectory(char *buf, int size)
{
	return 0;
}

int OsDependentDummy::ChangeDirectory(const char *dir)
{
	return 0;
}

int OsDependentDummy::KillFile(const char *filename)
{
	return 0;
}

