#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wavwrite.h"

WavWriter::WavWriter()
{
  fp = NULL;
  HeaderOut = false;

  TotalSamples = 0;
  Frequency = 0;
  Channels = 0;
  Bits = 0;

  SetFormat();
}

WavWriter::~WavWriter()
{
  Close();
}

void WavWriter::SetFormat(int rate, int bits, int channels)
{
  this->Frequency = rate;
  this->Bits = bits;
  this->Channels = channels;
  this->Bytes = (Bits / 8);
}

bool WavWriter::Open(const char *filename)
{
  fp = fopen(filename, "wb");
  return fp != NULL;
}

void WavWriter::Close(void)
{
  if (fp == NULL) return;
  WriteHeader();
  fclose(fp);
  fp = NULL;
}

void WavWriter::WriteHeader()
{
  HeaderOut = true;
  if (fp == NULL) return;

  unsigned char hdr[0x80];

  long PcmByteSize = TotalSamples * Channels * Bytes;

  memcpy(hdr, "RIFF", 4);
  WriteDword(hdr + 4, PcmByteSize + 44);
  memcpy(hdr + 8, "WAVEfmt ", 8);
  WriteDword(hdr + 16, 16);                           // chunk length
  WriteWord(hdr + 20, 1);                             // pcm id
  WriteWord(hdr + 22, Channels);                      // ch
  WriteDword(hdr + 24, Frequency);                    // freq
  WriteDword(hdr + 28, Frequency * Channels * Bytes); // bytes per sec
  WriteWord(hdr + 32, Channels * Bytes);              // bytes per frame
  WriteWord(hdr + 34, Bits);                          // bits

  memcpy(hdr + 36, "data", 4);
  WriteDword(hdr + 40, PcmByteSize); // pcm size

  // ヘッダを更新
  fseek(fp, 0, SEEK_SET);
  fwrite(hdr, 44, 1, fp);

  // ファイルポインタを末尾に戻す
  fseek(fp, 0, SEEK_END);
}

void WavWriter::WriteData(short *buf, int length)
{
  if (!HeaderOut) WriteHeader();
  TotalSamples += length;

  fwrite(buf, length * Channels * Bytes, 1, fp);
}

void WavWriter::WriteData(int *buf, int length)
{
  short out[2048];
  if (!HeaderOut) WriteHeader();

  TotalSamples += length;

  while(0 < length) {
    int block_size = 512 <= length ? 512 : length;
    
    // convert int to short
    for (int i = 0; i < block_size * 2; i++) {
      int v = *(buf++);
      out[i] = v > 32767 ? 32767 : (v < -32768 ? -32768 : v);
    }
    fwrite(out, block_size * Channels * Bytes, 1, fp);

    length -= block_size;
  }

}

void WavWriter::WriteWord(unsigned char *p, unsigned short v)
{
  p[0] = (v & 0xff);
  p[1] = ((v >> 8) & 0xff);
}

void WavWriter::WriteDword(unsigned char *p, unsigned long v)
{
  p[0] = (v & 0xff);
  p[1] = ((v >> 8) & 0xff);
  p[2] = ((v >> 16) & 0xff);
  p[3] = ((v >> 24) & 0xff);
}
