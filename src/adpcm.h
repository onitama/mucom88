#pragma once

#include	<windows.h>

typedef struct {
	BYTE	bID[4];		// ヘッダ
	DWORD	dSize;		// サイズ
} RIFF_HED;

typedef struct {
	BYTE	bID[4];
	DWORD	dChunkSize;
} CHUNK_HED;

typedef struct {
	BYTE	bFMT[4];
	DWORD	dChunkSize;
	WORD	wFmt;
	WORD	wChannels;
	DWORD	dRate;
	DWORD	dDataRate;
	WORD	wBlockSize;
	WORD	wSample;
} WAVE_CHUNK;

typedef struct {
	BYTE	bID[4];
	DWORD	dSize;
	BYTE	bData[1];
} DATA_CHUNK;

class	Adpcm{
private:
	RIFF_HED	*m_pRiffHed;
	WAVE_CHUNK	*m_pWaveChunk;
	DATA_CHUNK	*m_pDataChunk;

	// ADPCM-A
	int acc;
	int decstep;
	int diff;
	int step;
	int predsample;
	int index;
	int prevsample;
	int previndex;
	static short step_size[49];
	static int step_adj[16];
	int jedi_table[16 * 49];
public:
	Adpcm();
	~Adpcm();
	BYTE* waveToAdpcm(void *pData,DWORD dSize,DWORD &dAdpcmSize,DWORD dRate,DWORD dPadSize = 32);
	short* resampling(DWORD &dSize,DWORD dRate,DWORD dPadSize);
	int encode(short *pSrc,unsigned char *pDis,DWORD iSampleSize);

};
