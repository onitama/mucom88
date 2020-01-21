// Portable Z80 emulation class
// Copyright (C) Yasuo Kuwahara 2002-2018
// version 2.10

#ifndef _Z80_H_
#define _Z80_H_

//////// configuration

// ENDIAN_SWITCH
// 0...little endian
// 1...big endian

#define ENDIAN_SWITCH		0

// BUILTIN_MEMORY
// 0...external (default)
// 1...builtin

#define BUILTIN_MEMORY		0

// CLOCK_EMU
// 0...no count					fastest
// 1...count					faster
// 2...count w/wait	emulation	fast

#define CLOCK_EMU			0

//////// debug definition

//#define Z80_TRACE
//#define Z80_DEBUG

////////////////////////////////////////

#ifdef WIN32
typedef signed char int8_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#else
#include <stdint.h>
#endif

#if CLOCK_EMU
#define CLOCK(x)			(clock += (x))
#if CLOCK_EMU >= 2
#define CLOCK_M1()			(clock += 4 + wait)
#define CLOCK_MEM()			(clock += 3 + wait)
#define CLOCK_IO()			(clock += 4 + wait)
#define CLOCK_OFS()			(clock += 4 + wait)
#define CLOCK_MEM2()		(clock += (3 + wait) << 1)
#else
#define CLOCK_M1()			(clock += 4)
#define CLOCK_MEM()			(clock += 3)
#define CLOCK_IO()			(clock += 4)
#define CLOCK_OFS()			(clock += 4)
#define CLOCK_MEM2()		(clock += 3 << 1)
#endif
#else
#define CLOCK(x)
#define CLOCK_M1()
#define CLOCK_MEM()
#define CLOCK_IO()
#define CLOCK_OFS()
#define CLOCK_MEM2()
#endif

#define FBUFMAX				128

#ifdef Z80_TRACE

#define TRACEMAX			100
#define Z80_DEBUG

enum {
	acsStore8 = 4, acsStore16, acsLoad8, acsLoad16, acsIn, acsOut
};

struct TraceBuffer {
	uint16_t pc; uint8_t acs1, acs2;
	uint16_t adr1, data1, adr2, data2;
	int32_t r0, r1;
};

#define TRACE_LOG1(x) (tracep->acs1 = (x), tracep->adr1 = adr, tracep->data1 = data)
#define TRACE_LOG2(x) (tracep->acs2 = (x), tracep->adr2 = adr, tracep->data2 = data)

#else

#define TRACE_LOG1(x)
#define TRACE_LOG2(x)

#endif

struct FlagDecision {
	uint32_t dm;
	int32_t b, a;
	uint16_t pv, cy;
};

typedef struct {
	uint16_t pc;
	uint16_t sp;

	uint16_t af;
	uint16_t bc;
	uint16_t de;
	uint16_t hl;
	uint16_t ix;
	uint16_t iy;

	uint16_t xaf;
	uint16_t xbc;
	uint16_t xde;
	uint16_t xhl;
} RegSet;

class Z80 {
public:
	Z80();
	virtual ~Z80() {}
	void Reset();
	int32_t Execute(int32_t = 1);
	void INT(int data_bus) { intdata = data_bus; intreq = true; }
	void NMI() { nmireq = true; }
	void SetSP(uint16_t adr) { sp = adr; }
	void SetHL(uint16_t adr);
	void SetIntVec(uint8_t data) { IntVec = data; }
	uint16_t GetHL(void);
	uint8_t GetA(void);
	uint16_t GetIX(void);

	bool verbose;

	void EnableBreakPoint(uint16_t adr);
	void DisableBreakPoint();
	void DebugRun();
	void DebugInstExec();
	void DebugPause();
	void DebugDisable();
	void DebugEnable();


	void GetRegSet(RegSet *reg);
	void SetRegSet(RegSet* reg);
#if BUILTIN_MEMORY
	void SetMemoryPtr(uint8_t *p) { m = p; }
#endif
#ifdef Z80_TRACE
	void StopTrace();
#endif
protected:
	bool DebugEnableFlag;
	bool DebugWaitFlag;
	bool DebugInstExecFlag;
	bool EnableBreakPointFlag;
	uint16_t BreakPointAddress;

	uint16_t pc;
#if BUILTIN_MEMORY
	uint8_t *m;
#endif
#if CLOCK_EMU
	int32_t clock;
#endif
#if CLOCK_EMU >= 2
	uint32_t wait;
#endif
private:
#if BUILTIN_MEMORY
#define load(x)		(m[x])
#define store(x, y)	(m[x] = y)
#else
	virtual int32_t load(uint16_t adr) = 0;
	virtual int32_t loadpc(uint16_t adr) = 0;
	virtual void store(uint16_t adr, uint8_t data) = 0;
#endif
#if CLOCK_EMU >= 2
	virtual int32_t loadm1(uint16_t adr) = 0;
#else
#define loadm1(x)	load(x)
#endif
	virtual int32_t input(uint16_t) = 0;
	virtual void output(uint16_t, uint8_t) = 0;
	int32_t ld8(uint16_t adr) {
		if (rofs) {
			adr += (int8_t)load(pc);
			CLOCK_OFS();
		}
		int32_t data = load(adr);
		CLOCK_MEM();
		TRACE_LOG1(acsLoad8);
		return data;
	}
	int32_t ld16(uint16_t adr) {
		int32_t data = load(adr);
		data |= load(adr + 1) << 8;
		CLOCK_MEM2();
		TRACE_LOG1(acsLoad16);
		return data;
	}
	void st8(uint16_t adr, uint8_t data) {
		if (rofs) {
			adr += (int8_t)load(pc);
			CLOCK_OFS();
		}
		store(adr, data);
		CLOCK_MEM();
		TRACE_LOG2(acsStore8);
	}
	void st16(uint16_t adr, uint16_t data) {
		store(adr, (uint8_t)data);
		store(adr + 1, data >> 8);
		CLOCK_MEM2();
		TRACE_LOG2(acsStore16);
	}
	int32_t in(uint16_t adr) {
		int32_t data = input(adr);
		CLOCK_IO();
		TRACE_LOG1(acsIn);
		return data;
	}
	void out(uint16_t adr, uint8_t data) {
		output(adr, data);
		CLOCK_IO();
		TRACE_LOG2(acsOut);
	}
	virtual bool Extender(uint16_t) { return false; }
	virtual void RETI() {}
	void DDCB_FDCB();
	int ResolvC();
	int ResolvN();
	int ResolvPV();
	int ResolvH();
	int ResolvZ();
	int ResolvS();
	void SetupFlags(int);
	int ResolvFlags();
	uint8_t r[24];
	uint16_t sp, hl;
	uint32_t bcde, rofs;
	uint8_t IntVec, RefReg, RefReg7, Iff1, Iff2, IntMode, a_, f_;
	bool nmireq, intreq, Iff_set;
	uint8_t intdata;
	FlagDecision fbuf[FBUFMAX];
	FlagDecision *fp;
#ifdef Z80_TRACE
	TraceBuffer tracebuf[TRACEMAX];
	TraceBuffer *tracep;
	bool trace_enable;
#endif

	virtual void Halt() {}

#ifdef Z80_DEBUG
	void ill();
#endif
};

#endif
