#pragma once

#include <stdio.h>

class WavWriter
{
public:
  WavWriter();
  ~WavWriter();

  bool Open(const char *filename);
  void Close();

  void SetFormat(int rate = 44100, int bits = 16, int channels = 2);

  // length: shortの数
  void WriteData(short *buf, int length);

  // length: intの数
  void WriteData(int *buf, int length);


private:
  FILE *fp;
  bool HeaderOut;

  int TotalSamples;
  int Frequency;
  int Channels;
  int Bits;
  int Bytes;

  void WriteDword(unsigned char *buf, unsigned long value);
  void WriteWord(unsigned char *buf, unsigned short value);

  void WriteHeader();
};
