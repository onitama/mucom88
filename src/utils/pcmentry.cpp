#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pcmentry.h"

PcmEntry::PcmEntry() {
	adrl = 0;
	adrh = 0;
	pcmopt = 0;
	pcmstart = 0;
	data = NULL;
}

PcmEntry::~PcmEntry() {
    if (data != NULL) delete[] data;
}

void PcmEntry::SetEntry(unsigned char *entry) {
	memset(entry, 0x00, 0x20);

	int len = strlen(name);
	strncpy((char*)entry, name, 16);
	if (len < 16) memset(entry + len, 0x20, 16 - len);

	int whl = adrh - adrl;

	WriteWord(entry + 0x10, adrl);
	WriteWord(entry + 0x12, adrh);
	WriteWord(entry + 0x1a, pcmopt);
	WriteWord(entry + 0x1c, pcmstart);
	WriteWord(entry + 0x1e, whl);
}

int PcmEntry::GetLength() {
	return filesize;
}

void PcmEntry::WriteWord(unsigned char *data, int value) {
	data[0] = value & 0xff;
	data[1] = (value >> 8) & 0xff;
}

void PcmEntry::SetStart(int start) {
	int end = start + filesize;
	adrl = (start >> 2);
	adrh = (end >> 2);
	pcmstart = (start >> 2);
}

bool PcmEntry::SetData(const char *name, const char *binfile) {
	strcpy(this->name, name);

    FILE *fp = fopen(binfile, "rb");
    if (fp == NULL) { return false; }

    fseek(fp,0,SEEK_END);
    filesize = (int)ftell(fp);
    fseek(fp,0,SEEK_SET);

    data = new unsigned char[filesize];
    fread(data, filesize, 1, fp);
    fclose(fp);

    return true;
}

