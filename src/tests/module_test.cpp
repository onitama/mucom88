#include <stdio.h>
#include <string.h>

// mainをSDL_mainにするために必要
#ifdef USE_SDL
#include <SDL.h>
#endif


#include "mucom_module.h"

void WriteWORD(unsigned char *p, unsigned short v) {
    p[0] = (v & 0xff);
    p[1] = ((v>>8) & 0xff);
}

void WriteDWORD(unsigned char *p, unsigned long v) {
    p[0] = (v & 0xff);
    p[1] = ((v>>8) & 0xff);
    p[2] = ((v>>16) & 0xff);
    p[3] = ((v>>24) & 0xff);
}


// WAVヘッダ出力
void WriteWavHeader(FILE *fp, int frequency, int bits, int channels, long samples) {
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

void recordWav(const char *outputFilename, MucomModule *m,int seconds) {
    FILE *fp = fopen(outputFilename,"wb");
    if (fp == NULL) return;

    int rate = 44100;
    int bits = 16;
    int channels = 2;
    long total_samples = 0;

    WriteWavHeader(fp, rate, bits, channels, total_samples);

    long ms = 0;
    int samples = 128;
    short out[512];
    
    while(total_samples < rate * seconds) {
        m->Mix(out, samples);
        fwrite(out, samples*4, 1, fp);
        total_samples += samples;
    }

	WriteWavHeader(fp, rate, bits, channels, total_samples);
    fclose(fp);
}

int compileWav(const char *filename, const char *wavFilename) {
    MucomModule *module = new MucomModule();
    // module->SetVolume(1.0);
    printf("File:%s\n", filename);
    bool r = module->Open(".", filename);
    printf("open\n");
    puts(module->GetResult());
    if (!r) return -1;
    r = module->Play();
    if (!r) return -1;
    printf("recordWav\n");
    recordWav(wavFilename, module, 30);
    printf("close\n");
    module->Close();
    delete module;
}

int main(int argc, char *argv[])
{
#if defined(USE_SDL) && defined(_WIN32)
    freopen( "CON", "w", stdout );
    freopen( "CON", "w", stderr );
#endif

    printf("MucomModule Test\n");
    if (compileWav("sampl1.muc","sampl1.wav") < 0) return -1; 
    if (compileWav("sampl2.muc","sampl2.wav") < 0) return -1; 
    if (compileWav("sampl3.muc","sampl3.wav") < 0) return -1; 
    return 0;
}


