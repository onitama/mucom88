// ---------------------------------------------------------------------------
//	DirectSound based driver
// ---------------------------------------------------------------------------
//	soundds.h,v 1.2 2002/05/31 09:45:21 cisc
//	soundds.h       modified 2018/11 onitama

#if !defined(win32_soundds_h)
#define win32_soundds_h

#include "headers.h"
#include "fmgen/types.h"
#include "fmgen/opna.h"
#include "soundbuf.h"

typedef int16 Sample;

// ---------------------------------------------------------------------------

namespace WinSoundDriver
{

	class Driver
	{
	public:
		Driver() {}
		virtual ~Driver() {}

		virtual bool Init(uint rate, uint ch, uint buflen) = 0;
		virtual bool Cleanup() = 0;
		void MixAlways(bool yes) { mixalways = yes; }

	protected:
		uint buffersize;
		uint sampleshift;
		volatile bool playing;
		bool mixalways;
	};


class DriverDS : public Driver
{
	static const uint num_blocks;
	static const uint timer_resolution;

public:
	DriverDS();
	~DriverDS();

	bool Init(uint rate, uint ch, uint buflen);
	bool Cleanup();

	SoundBuf *GetSoundBuffer(void) { return sndbuf; }
	void UpdateTime(void);
	bool CheckInterrupt(int ms);

	int PrepareSend(void);
	void Send(void);

	void SetHWND(HWND hwnd) { master_hwnd = hwnd; }

private:

	HWND master_hwnd;
	LPDIRECTSOUND lpds;
	LPDIRECTSOUNDBUFFER lpdsb_primary;
	LPDIRECTSOUNDBUFFER lpdsb;

	int writelength;
	uint nextwrite;
	uint buffer_length;
	bool restored;

	SoundBuf *sndbuf;
};

}

#endif // !defined(win32_soundds_h)
