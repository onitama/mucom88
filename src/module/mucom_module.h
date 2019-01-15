// MucomModule 
// BouKiCHi 2019

#ifndef _MUCOM_MODULE_H_
#define _MUCOM_MODULE_H_

#include "cmucom.h"

class MucomModule  {
public:
    MucomModule();
    ~MucomModule();
    bool Open(const char *workingDirectory, const char *songFilename);
    void SetRate(int rate);
    void SetPCM(const char *file);
    void SetVoice(const char *file);
    void SetVolume(double vol);
    void Mix(short *buffer, int samples);
    void Volume(double vol);
    bool Play();
    void Close();
    const char *GetResult();
private:
    double volume;
    const char *GetMucomMessage();
    void FreeMucom();
    void FreeResultBuffer();
    void AddResultBuffer(const char *text);
	CMucom *mucom;
    char *resultText;
    const char *pcmfile;
    const char *voicefile;
    const char *outfile;
    int audioRate;
};

#endif
