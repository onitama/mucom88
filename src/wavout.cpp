// WAV file record
// BouKiCHi 2019

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cmucom.h"
#include "mucomvm.h"

static void WriteWORD(unsigned char *p, unsigned short v) {
    p[0] = (v & 0xff);
    p[1] = ((v>>8) & 0xff);
}

static void WriteDWORD(unsigned char *p, unsigned long v) {
    p[0] = (v & 0xff);
    p[1] = ((v>>8) & 0xff);
    p[2] = ((v>>16) & 0xff);
    p[3] = ((v>>24) & 0xff);
}


// WAVヘッダ出力
static void WriteWavHeader(FILE *fp, int frequency, int bits, int channels, long samples) {
  if (!fp) return;
  unsigned char hdr[0x80];
  int bytes = (bits/8);
  long pcm_bytesize = samples * channels * bytes;

  memcpy(hdr,"RIFF", 4);
  WriteDWORD(hdr + 4, pcm_bytesize + 44);
  memcpy(hdr + 8,"WAVEfmt ", 8);
  WriteDWORD(hdr + 16, 16); // chunk length
  WriteWORD(hdr + 20, 01); // pcm id
  WriteWORD(hdr + 22, channels); // ch
  WriteDWORD(hdr + 24, frequency); // freq
  WriteDWORD(hdr + 28, frequency * channels * bytes); // bytes per sec
  WriteWORD(hdr + 32, channels * bytes); // bytes per frame
  WriteWORD(hdr + 34, bits); // bits

  memcpy(hdr + 36, "data",4);
  WriteDWORD(hdr + 40, pcm_bytesize); // pcm size

  // 先頭のヘッダを更新
  fseek(fp, 0, SEEK_SET);
  fwrite(hdr, 44, 1, fp);

  // ファイルポインタを末尾に戻す
  fseek(fp, 0, SEEK_END);
}

void RecordWave(CMucom *m, const char *fname, int rate, int seconds) {
	int buf[512];
	short out[512];

	int bits = 16;
	int channels = 2;
	long total_samples = 0;

	m->SetVMOption(VM_OPTION_STEP, 1);		// オプションを設定
	m->SetAudioRate(rate);

	FILE *fp = fopen(fname, "wb");
	if (fp == NULL) return;

	WriteWavHeader(fp, rate, bits, channels, total_samples);

	while (total_samples < rate * seconds) {
		int samples = 16;
		m->RenderAudio(buf, samples);

		for(int i=0; i < samples*2; i++) {
			int v=buf[i];
			out[i] = v > 32767 ? 32767 : (v < -32768 ? -32768 : v);
		}
		fwrite(out, samples*4, 1, fp);
		total_samples += samples;
	}

	WriteWavHeader(fp, rate, bits, channels, total_samples);
	fclose(fp);

	m->SetVMOption(VM_OPTION_STEP, 2);		// オプションを解除
}

/*----------------------------------------------------------*/
