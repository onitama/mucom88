#include <stdio.h>
#include <string.h>

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

void RecordWav(const char *outputFilename, MucomModule *m,int seconds) {
    FILE *fp = fopen(outputFilename,"wb");
    if (fp == NULL) return;

    int rate = 44100;
    int bits = 16;
    int channels = 2;
    long totalSamples = 0;

    WriteWavHeader(fp, rate, bits, channels, totalSamples);

    long ms = 0;
    int samples = 128;
    short out[512];
    
    while(totalSamples < rate * seconds) {
        m->Mix(out, samples);
        fwrite(out, samples*4, 1, fp);
        totalSamples += samples;
    }

	WriteWavHeader(fp, rate, bits, channels, totalSamples);
    fclose(fp);
}

int CompileWav(const char *filename, const char *wavFilename) {
    MucomModule *module = new MucomModule();
    // module->SetVolume(1.0);
    printf("Input File:%s\n", filename);
    printf("Output File:%s\n", wavFilename);

    bool r = module->Open(".", filename);
    printf("open\n");
    puts(module->GetResult());
    if (!r) return -1;
    r = module->Play();
    if (!r) return -1;
    printf("RecordWav\n");
    RecordWav(wavFilename, module, 30);
    printf("close\n");
    module->Close();
    delete module;
}

int main(int argc, char *argv[])
{
    char *inputMuc;
    char outputFilename[512];
    printf("MucomModule Compile Test\n");
    if (argc < 2) {
        printf("Usage compile input.muc\n");
        return 0;
    }

    inputMuc = argv[1];
    strcpy(outputFilename, inputMuc);
    char *pos = strrchr(outputFilename,'.');
    const char *extWav = ".wav";
    if (pos != NULL) strcpy(pos,extWav); else strcat(outputFilename,extWav);
    if (CompileWav(argv[1], outputFilename) < 0) return -1; 
    return 0;
}


