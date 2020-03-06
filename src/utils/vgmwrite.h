#pragma once

#include <stdio.h>
#include "logwrite.h"

class VGMWrite : public ILogWrite
{
public:
	VGMWrite();
	~VGMWrite();

	bool Open(const char *filename);
	void Close();

	void SetLoopPoint();
	void Wait(double Seconds);
	void WriteData(int Device, int Address, int Value);

	void WriteAdpcmMemory(void* pData, int size);

private:
	FILE *fp;
	bool Loop;
	bool HeaderOut;

	double BaseTick;
	double SyncBufferTicks;

	int TotalSyncCount;
	int FileSize;

	int LoopPoint;
	int DataLength;


	void WriteEndMark();

	void WriteDword(unsigned char* buf, int value);
	void WriteWord(unsigned char* buf, int value);

	void WriteValue(int value);
	void WriteDwordValue(unsigned int value);
	void WriteSync();
	void WriteSync1();
	void WriteSync2(int Count);

	void WriteHeader();

	void WriteVgmAdpcm(unsigned int transSize, unsigned char *AdpcmBuffer);

};

