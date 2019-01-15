// MucomModule 
// BouKiCHi 2019

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "mucom_module.h"

#define MUCOM_DEFAULT_PCMFILE "mucompcm.bin"
#define DEFAULT_OUTFILE "mucom88.mub"

MucomModule::MucomModule() {
  audioRate = 44100;
  pcmfile = MUCOM_DEFAULT_PCMFILE;
  outfile = DEFAULT_OUTFILE;
  voicefile = NULL;
  resultText = NULL;
  volume = 1.0f;
}

MucomModule::~MucomModule() {
  Close();
}

void MucomModule::SetRate(int rate) {
  audioRate = rate;
}

void MucomModule::SetPCM(const char *file) {
  pcmfile = file;
}

void MucomModule::SetVoice(const char *file) {
  voicefile = file;
}

void MucomModule::SetVolume(double vol) {
  volume = vol;
}


bool MucomModule::Open(const char *workingDirectory, const char *songFilename) {
  chdir(workingDirectory);
  mucom = new CMucom();
  int cmpopt = MUCOM_CMPOPT_COMPILE;
  mucom->Init(NULL,cmpopt,audioRate);
  mucom->Reset(cmpopt);
  if (pcmfile) mucom->LoadPCM(pcmfile);
  if (voicefile) mucom->LoadFMVoice(voicefile);
  int cr = mucom->CompileFile(songFilename, outfile);

  AddResultBuffer(GetMucomMessage());
  FreeMucom();

  if (cr != 0) return false; 

  // 再生用に再度準備
  mucom = new CMucom();
  cmpopt = MUCOM_CMPOPT_STEP;
  mucom->Init(NULL,cmpopt,audioRate);
  mucom->Reset(0);
  if (mucom->LoadMusic(outfile) < 0) {
      AddResultBuffer(GetMucomMessage());
    return false;
  }
  return true;
}

const char *MucomModule::GetMucomMessage() {
  mucom->PrintInfoBuffer();
  return mucom->GetMessageBuffer();
}

void MucomModule::AddResultBuffer(const char *text) {
  int len = strlen(text);
  if (resultText != NULL) len += strlen(resultText);
  char *ptr = new char[len+1];
  if (resultText != NULL) strcpy(ptr, resultText); else  ptr[0] = 0x00;
  strcat(ptr, text);
  FreeResultBuffer();
  resultText = ptr;
}

bool MucomModule::Play() {
    if (!mucom) return false;
    if (mucom->Play(0) < 0) return false;
    return true;
}


void MucomModule::Close() {
  FreeMucom();
  FreeResultBuffer();
}

void MucomModule::FreeMucom() {
  if (!mucom) return;
  delete mucom;
  mucom = NULL;
}

void MucomModule::FreeResultBuffer() {
  if (!resultText) return;
  delete resultText;
  resultText = NULL;
}

const char *MucomModule::GetResult() {
  if (!resultText) return "";
  return resultText;
}

void MucomModule::Mix(short *data, int samples) {
	int buf[128];

  int index = 0;
  while(samples > 0) {
      int s = samples < 16 ? samples : 16;
      mucom->RenderAudio(buf, s);

      for(int i=0; i < s*2; i++) {
          int v=(buf[i]*volume);

          data[index] = v > 32767 ? 32767 : (v < -32768 ? -32768 : v);
          index++;
      }
      samples -= s;
  }
}
