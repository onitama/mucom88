
//
//	soundbuf.cpp structures
//
#ifndef __soundbuf_h
#define __soundbuf_h

#include "fmgen/types.h"

/*------------------------------------------------------------*/

//
//	soundbuf.cpp functions
//

#define SNDBUF_LIMIT 0x8000			// 1度にバッファに書き込める最大サイズ

class SoundBuf {
public:
	SoundBuf();
	~SoundBuf();

	void Reset(int size);
	int GetBuffer32(void *dst, int size);
	int GetBuffer16(void *dst, int size);
	int GetEndPtr(void) { return m_endptr; }
	int GetReadPtr(void) { return m_readptr; }
	int GetPoolSize(void) { return m_wtick - m_rtick; }

	int32 *PrepareBuffer(int size);
	void UpdateBuffer(int size);
	void PrepareReadBuffer(void);

private:
	//		Settings
	//
	int	m_size;
	int m_curptr;
	int m_endptr;
	int m_readptr;

	int m_wtick;
	int m_rtick;

	int32 *m_buffer;
};


#endif
