#pragma once

class PcmEntry
{
public:
	PcmEntry();
	~PcmEntry();

    char name[16];

    int adrl;
    int adrh;
	int pcmopt;
	int pcmstart;

    int filesize;
    unsigned char *data;
    bool SetData(const char *name, const char *binfile);
	void SetEntry(unsigned char* entry);
	void WriteWord(unsigned char* data, int value);

	void SetStart(int start);

	int GetLength();
};

