#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "s98write.h"

S98Write::S98Write() {
	BaseTick = 0;
	SyncBufferTicks = 0;
	LoopPoint = 0;
	DataLength = 0;

	Numerator = 0;
	Denominator = 0;
	DeviceCount = 0;

	HeaderOut = false;

	BaseTick = (double)10 / 1000;
	Loop = false;
}

S98Write::~S98Write() {
	Close();
}

bool S98Write::Open(const char *filename) {
	fp = fopen(filename,"wb");
	return fp != NULL;
}

void S98Write::Close(void) {
	WriteSync();
	WriteEnd();
	if (fp != NULL) fclose(fp);
	fp = NULL;
}

void S98Write::WriteHeader() {
	HeaderOut = true;
	if (fp == NULL) return;
	fseek(fp, 0, SEEK_SET);

	unsigned char header[0x20];
	memset(header, 0, 0x20);
	memcpy(header, "S983",4);
	WriteDword(header + 0x04, Numerator);
	WriteDword(header + 0x08, Denominator);
	WriteDword(header + 0x0c, 0); // Reserved

	WriteDword(header + 0x10, 0); // Tag

	int DumpStart = 0x20 + (DeviceCount * 0x10);
	WriteDword(header + 0x14, DumpStart);
	WriteDword(header + 0x18, LoopPoint);
	WriteDword(header + 0x1c, DeviceCount);

	fwrite(header, 0x20, 1, fp);
}

void S98Write::WriteDword(unsigned char *data, int value) {
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
	data[2] = (value >> 16) & 0xff;
	data[3] = (value >> 24) & 0xff;
}

void S98Write::WriteWord(unsigned char *data, int value) {
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
}

void S98Write::WriteData(int Device, int Address, int Value) {
	WriteSync();
	int d = (int)(Device * 2 + (Address >= 0x100 ? 1 : 0));
	int a = (int)(Address & 0xff);
	int v = (int)(Value & 0xff);
	WriteValue(d);
	WriteValue(a);
	WriteValue(v);
}

void S98Write::WriteAdpcmMemory(void* pData, int size) {

	unsigned char AdpcmBuffer[0x40000];

	// メモリからコピーする
	memcpy(AdpcmBuffer, pData, size);
	// 半端分？はゼロ生めする
	memset(AdpcmBuffer + size, 0x00, 0x40000 - size);
	// 転送サイズを計算する（パディングを考慮）
	unsigned transSize = size + ((0x20 - (size & 0x1f)) & 0x1f);

	WriteData(0, 0x100, 0x20);
	WriteData(0, 0x100, 0x21);
	WriteData(0, 0x100, 0x00);
	WriteData(0, 0x110, 0x00);
	WriteData(0, 0x110, 0x80);

	WriteData(0, 0x100, 0x61);
	WriteData(0, 0x100, 0x68);
	WriteData(0, 0x101, 0x00);

	// アドレス
	WriteData(0, 0x102, 0x00);
	WriteData(0, 0x103, 0x00);
	WriteData(0, 0x104, 0xff);
	WriteData(0, 0x105, 0xff);
	WriteData(0, 0x10c, 0xff);
	WriteData(0, 0x10d, 0xff);
	// PCM転送
	unsigned cnt = 0;
	for (cnt = 0; cnt < transSize; cnt++) {
		WriteData(0, 0x108, AdpcmBuffer[cnt]);
	}
	// 終了
	WriteData(0, 0x100, 0x00);
	Wait(0.016); // 16ms?
	WriteData(0, 0x110, 0x80);
	Wait(0.016);
}



void S98Write::WriteValue(int value) {
	if (!HeaderOut) WriteHeader();
	if (fp != NULL) fputc(value, fp);
	DataLength++;
}

void S98Write::WriteSync() {
	if (SyncBufferTicks < BaseTick) return;

	int SyncCount = (int)(SyncBufferTicks / BaseTick);
	SyncBufferTicks -= BaseTick * SyncCount;
	if (SyncCount > 1) WriteSync2(SyncCount); else WriteSync1();
}

void S98Write::WriteSync2(int Count) {
	Count -= 2;
	WriteValue(0xfe);
	while (true) {
		int v = (Count & 0x7f);
		Count = Count >> 7;
		bool Next = Count > 0;
		v = v | (Next ? 0x80 : 0x00);

		WriteValue(v & 0xff);
		if (!Next) break;
	}
}

void S98Write::WriteSync1() {
	WriteValue(0xff);
}

void S98Write::WriteEnd() {
	WriteValue(0xfd);
}

void S98Write::SetLoopPoint() {
	Loop = true;
	LoopPoint = DataLength;
}

void S98Write::Wait(double seconds) {
	SyncBufferTicks += seconds;
}

