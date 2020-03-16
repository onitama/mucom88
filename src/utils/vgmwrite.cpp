#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "vgmwrite.h"

VGMWrite::VGMWrite() {
	BaseTick = 0;
	SyncBufferTicks = 0;
	LoopPoint = 0;
	DataLength = 0;

	TotalSyncCount = 0;
	FileSize = 0x100;

	HeaderOut = false;

	BaseTick = (double)1 / 44100;
	Loop = false;
}

VGMWrite::~VGMWrite() {
	Close();
}

bool VGMWrite::Open(const char *filename) {
	fp = fopen(filename,"wb");
	return fp != NULL;
}

void VGMWrite::Close(void) {
	if (fp == NULL) return;

	WriteSync();
	WriteEndMark();

	FileSize = (int)ftell(fp);
	WriteHeader();
	fclose(fp);
	fp = NULL;
}

void VGMWrite::WriteHeader() {
	HeaderOut = true;
	if (fp == NULL) return;
	fseek(fp, 0, SEEK_SET);

	unsigned char header[0x100];
	memset(header, 0, 0x100);
	memcpy(header, "Vgm ",4);

	WriteDword(header + 0x04, FileSize - 4);
	WriteDword(header + 0x08, 0x170); // Ver 1.70

	WriteDword(header + 0x18, TotalSyncCount);

	WriteDword(header + 0x34, 0x100 - 0x34); // Offset to data

	WriteDword(header + 0x48, 8000000);

	fwrite(header, 0x100, 1, fp);
}

void VGMWrite::WriteDword(unsigned char *data, int value) {
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
	data[2] = (value >> 16) & 0xff;
	data[3] = (value >> 24) & 0xff;
}

void VGMWrite::WriteWord(unsigned char *data, int value) {
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
}

void VGMWrite::WriteData(int Device, int Address, int Value) {
	WriteSync();

	int cmd = 0x56 + ((Address >> 8) & 0x1);
	int a = (int)(Address & 0xff);
	int v = (int)(Value & 0xff);
	WriteValue(cmd);
	WriteValue(a);
	WriteValue(v);
}

void VGMWrite::WriteAdpcmMemory(void* pData, int size) {

	unsigned char AdpcmBuffer[0x40000];

	// メモリからコピーする
	memcpy(AdpcmBuffer, pData, size);
	// 半端分？はゼロ生めする
	memset(AdpcmBuffer + size, 0x00, 0x40000 - size);
	// 転送サイズを計算する（パディングを考慮）
	unsigned transSize = size + ((0x20 - (size & 0x1f)) & 0x1f);

	WriteVgmAdpcm(transSize, AdpcmBuffer);
}

void VGMWrite::WriteVgmAdpcm(unsigned int transSize, unsigned char *AdpcmBuffer)
{
	// VGM転送
	WriteValue(0x67); // VGM DataBlock Command
	WriteValue(0x66); // 
	WriteValue(0x81); // DataType: YM2608 ADPCM
	WriteDwordValue(transSize + 8); // BlockSize
	WriteDwordValue(transSize); // size of the entire ROM
	WriteDwordValue(0); // start address of data

						// PCM転送
	unsigned cnt = 0;
	for (cnt = 0; cnt < transSize; cnt++) {
		WriteValue(AdpcmBuffer[cnt]);
	}
}

void VGMWrite::WriteDwordValue(unsigned int value) {
	WriteValue(value & 0xff);
	WriteValue((value >> 8) & 0xff);
	WriteValue((value >> 16) & 0xff);
	WriteValue((value >> 24) & 0xff);

}

void VGMWrite::WriteValue(int value) {
	if (!HeaderOut) WriteHeader();
	if (fp != NULL) fputc(value, fp);
	DataLength++;
}

void VGMWrite::WriteSync() {
	if (SyncBufferTicks < BaseTick) return;

	int SyncCount = (int)(SyncBufferTicks / BaseTick);
	SyncBufferTicks -= BaseTick * SyncCount;

	TotalSyncCount += SyncCount;
	if (SyncCount > 1) WriteSync2(SyncCount); else WriteSync1();
}

void VGMWrite::WriteSync2(int Count) {
	Count -= 1;
	if (Count < 0x10) {
		WriteValue(0x70 + Count);
		return;
	}

	WriteValue(0x61);
	WriteValue(Count & 0xff);
	WriteValue((Count>>8) & 0xff);
}

void VGMWrite::WriteSync1() {
	WriteValue(0x70);
}

void VGMWrite::WriteEndMark() {
	WriteValue(0x66);
}

void VGMWrite::SetLoopPoint() {
	Loop = true;
	LoopPoint = TotalSyncCount;
}

void VGMWrite::Wait(double seconds) {
	SyncBufferTicks += seconds;
}

