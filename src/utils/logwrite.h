#pragma once

#include <stdio.h>

class ILogWrite {

public:
	virtual ~ILogWrite() {}

	virtual bool Open(const char* filename) = 0;
	virtual void Close() = 0;

	virtual void SetLoopPoint() = 0;
	virtual void Wait(double Seconds) = 0;
	virtual void WriteData(int Device, int Address, int Value) = 0;

	virtual void WriteAdpcmMemory(void* pData, int size) = 0;
};
