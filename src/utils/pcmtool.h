#pragma once

#include "../cmucom.h"
#include "pcmentry.h"

class PcmTool
{
public:
    PcmTool();
    ~PcmTool();

    int Convert(const char *infile);
    bool ConvertList(FILE *fp);
	int NextAddress(int len);
	bool WriteBinary(const char *outfile);

    CMucom *cmucom;
    PcmEntry entry[32];
    int entryCount;
	int pcmBodySize;

	unsigned char *header;
	unsigned char *body;
};
