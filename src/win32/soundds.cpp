
//
//		DirectSound based driver
//		(ストリーム再生用DirectSoundドライバ)
//			Based on soundds.cpp,v 1.10 2002/05/31 09:45:21 cisc
//			onion software/onitama 2018/11
//

#include "../mucomvm.h"
#include "soundds.h"
#include "../fmgen/opna.h"

#if defined( _MSC_VER )
#pragma comment(lib,"winmm.lib")
#pragma comment(lib, "lib/lib/d3d8.lib")
#pragma comment(lib, "lib/lib/d3dx8.lib")
#pragma comment(lib, "lib/lib/d3dxof.lib")
#pragma comment(lib, "lib/lib/dxguid.lib")
#endif

using namespace WinSoundDriver;

// ---------------------------------------------------------------------------

const uint DriverDS::num_blocks = 5;

// ---------------------------------------------------------------------------
//	構築・破棄 ---------------------------------------------------------------

DriverDS::DriverDS()
{
	playing = false;
	mixalways = false;
	lpds = 0;
	lpdsb = 0;
	lpdsb_primary = 0;
	sndbuf = NULL;
	master_hwnd = NULL;
}

DriverDS::~DriverDS()
{
	Cleanup();
}

// ---------------------------------------------------------------------------
//  初期化 -------------------------------------------------------------------

bool DriverDS::Init(uint rate, uint ch, uint buflen)
{
	if (playing)
		return false;

	buffer_length = buflen;
	sampleshift = 1 + (ch == 2 ? 1 : 0);

	// 計算
	buffersize = (rate * ch * sizeof(Sample) * buffer_length / 1000) & ~7;

	// DirectSound object 作成
	HRESULT result = CoCreateInstance(CLSID_DirectSound, 0, CLSCTX_ALL, IID_IDirectSound, (void**) &lpds);
	if (FAILED(result))
		return false;
	if (FAILED(lpds->Initialize(0)))
		return false;

	// 協調レベル設定
	HWND hwnd = master_hwnd;
	if (hwnd == NULL) hwnd = GetDesktopWindow();
	if (DS_OK != lpds->SetCooperativeLevel(hwnd, DSSCL_PRIORITY))
	{
		if (DS_OK != lpds->SetCooperativeLevel(hwnd, DSSCL_NORMAL))
			return false;
	}

	DSBUFFERDESC dsbd;
    memset(&dsbd, 0, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
    dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
    dsbd.dwBufferBytes = 0; 
    dsbd.lpwfxFormat = 0;
	if (DS_OK != lpds->CreateSoundBuffer(&dsbd, &lpdsb_primary, 0))
		return false;

	// 再生フォーマット設定
	WAVEFORMATEX wf;
    memset(&wf, 0, sizeof(WAVEFORMATEX));
    wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = ch;
    wf.nSamplesPerSec = rate;
	wf.wBitsPerSample = 16;
	wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;

	lpdsb_primary->SetFormat(&wf);

	// セカンダリバッファ作成
    memset(&dsbd, 0, sizeof(DSBUFFERDESC));
    dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_LOCSOFTWARE | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;

    dsbd.dwBufferBytes = buffersize; 
    dsbd.lpwfxFormat = &wf;
    
	HRESULT res = lpds->CreateSoundBuffer(&dsbd, &lpdsb, NULL); 
	if (DS_OK != res)
		return false;

	// 再生
	lpdsb->SetVolume(0);
	lpdsb->SetPan(0);
	lpdsb->Play(0, 0, DSBPLAY_LOOPING);

	nextwrite = 1 << sampleshift;
	
	//	サウンドバッファ
	sndbuf = new SoundBuf;
	sndbuf->Reset(rate * ch);

	playing = true;
	return true;
}


// ---------------------------------------------------------------------------
//  後片付け -----------------------------------------------------------------

bool DriverDS::Cleanup()
{
	playing = false;
	if (sndbuf) {
		delete sndbuf;
		sndbuf = NULL;
	}
	if (lpdsb)
	{
		lpdsb->Stop();
		lpdsb->Release(), lpdsb = 0;
	}
	if (lpdsb_primary)
		lpdsb_primary->Release(), lpdsb_primary = 0;
	if (lpds)
		lpds->Release(), lpds = 0;
	return true;
}

// ---------------------------------------------------------------------------
//  ブロック送る -------------------------------------------------------------

int DriverDS::PrepareSend()
{
	//		ストリーム送信準備(PrepareSend->Sendの順番で呼び出すこと)
	//
	DWORD status;

	restored = false;
	writelength = 0;

	lpdsb->GetStatus(&status);
	if (DSBSTATUS_BUFFERLOST & status)
	{
		// restore the buffer
		if (DS_OK != lpdsb->Restore()) return 0;
		nextwrite = 0;
		restored = true;
	}

	// 位置取得
	DWORD cplay, cwrite;
	lpdsb->GetCurrentPosition(&cplay, &cwrite);

	//printf("POS=%d\n", cplay);

	if (cplay == nextwrite && !restored) return 0;

	// 書きこみサイズ計算
	if (cplay < nextwrite)
		writelength = cplay + buffersize - nextwrite;
	else
		writelength = cplay - nextwrite;

	writelength &= -1 << sampleshift;

	//printf("play = %d  writelength = %d buf %d:%d pool %d\n", cplay, writelength, sndbuf->GetReadPtr(), sndbuf->GetEndPtr(), sndbuf->GetPoolSize());
	//printf("play = %5d  write = %5d  length = %5d(%d)\n", cplay, nextwrite, writelength, buffersize);

	return writelength;
}


void DriverDS::Send()
{
	if (writelength <= 0) return;

	void* a1, * a2;
	DWORD al1, al2;

	sndbuf->PrepareReadBuffer();

	// Lock
	if (DS_OK != lpdsb->Lock(nextwrite, writelength,
		(void**)&a1, &al1, (void**)&a2, &al2, 0)) return;

	// 書きこみ
	if (sndbuf) {
		if (a1) {
			sndbuf->GetBuffer16(a1, (al1 >> sampleshift)*2);
		}
		if (a2) {
			sndbuf->GetBuffer16(a2, (al2 >> sampleshift)*2);
		}
	}
	//printf( "SendPut%d\n", ((al1+al2) >> sampleshift) * 2);

	// Unlock
	lpdsb->Unlock(a1, al1, a2, al2);

	nextwrite += writelength;
	if (nextwrite >= buffersize)
		nextwrite -= buffersize;
	if (restored) {
		lpdsb->Play(0, 0, DSBPLAY_LOOPING);
	}

}


