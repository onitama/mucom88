// Portable Z80 emulation class
// Copyright (C) Yasuo Kuwahara 2002-2018
// version 2.10

#include "Z80.h"

#ifdef Z80_DEBUG
#include <stdio.h>
#include <stdlib.h>
#endif
#ifndef nil
#define nil			0
#endif

#if ENDIAN_SWITCH
#define B			(r[0])
#define C			(r[1])
#define L			(r[5])
#define F			(r[6])
#define A			(r[7])
#define REG0		(r[0])
#define REG1		(r[1])
#define REG2		(r[2])
#define REG3		(r[3])
#define REG4		(r[4 + rofs])
#define REG5		(r[5 + rofs])
#define REG7		(r[7])
#define REGFIX0		(r[0])
#define REGFIX1		(r[1])
#define REGFIX2		(r[2])
#define REGFIX3		(r[3])
#define REGFIX4		(r[4])
#define REGFIX5		(r[5])
#define REGFIX7		(r[7])
#else
#define B			(r[1])
#define C			(r[0])
#define L			(r[4])
#define F			(r[7])
#define A			(r[6])
#define REG0		(r[1])
#define REG1		(r[0])
#define REG2		(r[3])
#define REG3		(r[2])
#define REG4		(r[5 + rofs])
#define REG5		(r[4 + rofs])
#define REG7		(r[6])
#define REGFIX0		(r[1])
#define REGFIX1		(r[0])
#define REGFIX2		(r[3])
#define REGFIX3		(r[2])
#define REGFIX4		(r[5])
#define REGFIX5		(r[4])
#define REGFIX7		(r[6])
#endif

#define BC			(*(uint16_t *)&r[0])
#define DE			(*(uint16_t *)&r[2])
#define HL			(*(uint16_t *)&r[4 + rofs])
#define HLfix		(*(uint16_t *)&r[4])
#define REG160		BC
#define REG162		DE
#define REG164		HL
#define REGFIX160	BC
#define REGFIX162	DE
#define REGFIX164	HLfix
#define OFS_IX 		8
#define OFS_IY 		16
#if CLOCK_EMU
#if CLOCK_EMU >= 2
#define IMM8		(tmpc = load(pc++), CLOCK_MEM(), tmpc)
#define M1			(tmpc = loadm1(pc++), CLOCK_M1(), tmpc)
#define IMM16		(pc += 2, tmpc = load(pc - 2) | load(pc - 1) << 8, CLOCK_MEM2(), tmpc)
#else
#define IMM8		(CLOCK_MEM(), load(pc++))
#define M1			(CLOCK_M1(), load(pc++))
#define IMM16		(CLOCK_MEM2(), pc += 2, load(pc - 2) | load(pc - 1) << 8)
#endif
#else
#define IMM8		(loadpc(pc++))
#define M1			IMM8
#define IMM16		(pc += 2, loadpc(pc - 2) | loadpc(pc - 1) << 8)
#endif

#define SET2(macro)\
	macro(0)\
	macro(2)
#define SET3(macro)\
	macro(0)\
	macro(2)\
	macro(4)
#define SET7(macro)\
	macro(0)\
	macro(1)\
	macro(2)\
	macro(3)\
	macro(4)\
	macro(5)\
	macro(7)
#define SET7BIT(macro)\
	macro(0)\
	macro(1)\
	macro(2)\
	macro(3)\
	macro(4)\
	macro(5)\
	macro(6)
#define SET_8(macro)\
	macro(0)\
	macro(1)\
	macro(2)\
	macro(3)\
	macro(4)\
	macro(5)\
	macro(6)\
	macro(7)
#define SET7X(macro, X)\
	macro(0, X)\
	macro(1, X)\
	macro(2, X)\
	macro(3, X)\
	macro(4, X)\
	macro(5, X)\
	macro(7, X)
#define SET77REG(macro)\
	SET7X(macro, 0)\
	SET7X(macro, 1)\
	SET7X(macro, 2)\
	SET7X(macro, 3)\
	SET7X(macro, 4)\
	SET7X(macro, 5)\
	SET7X(macro, 7)
#define SET77BIT(macro)\
	SET7X(macro, 0)\
	SET7X(macro, 1)\
	SET7X(macro, 2)\
	SET7X(macro, 3)\
	SET7X(macro, 4)\
	SET7X(macro, 5)\
	SET7X(macro, 6)
#define SET78(macro)\
	SET7X(macro, 0)\
	SET7X(macro, 1)\
	SET7X(macro, 2)\
	SET7X(macro, 3)\
	SET7X(macro, 4)\
	SET7X(macro, 5)\
	SET7X(macro, 6)\
	SET7X(macro, 7)
#define COND8(macro)\
	macro(0, !ResolvZ())\
	macro(1, ResolvZ())\
	macro(2, !ResolvC())\
	macro(3, ResolvC())\
	macro(4, !ResolvPV())\
	macro(5, ResolvPV())\
	macro(6, !ResolvS())\
	macro(7, ResolvS())

#define LS		7
#define LZ		6
#define LH		4
#define LPV		2
#define LN		1
#define LC		0
#define MS		(1 << LS)
#define MZ 		(1 << LZ)
#define MH		(1 << LH)
#define MPV		(1 << LPV)
#define MN		(1 << LN)
#define MC		(1 << LC)

enum {
	FZERO = 2, FONE, FADD8, FSUB8, FADD16, FSUB16, FBEFORE, FPARITY, FPV, FPVZ, FIO
};

#define S0		((uint32_t)FZERO	<< (LS << 2))
#define S8		((uint32_t)FADD8	<< (LS << 2))
#define S16		((uint32_t)FADD16	<< (LS << 2))
#define SB		((uint32_t)FBEFORE	<< (LS << 2))
#define Z1		((uint32_t)FONE		<< (LZ << 2))
#define Z8		((uint32_t)FADD8	<< (LZ << 2))
#define Z16		((uint32_t)FADD16	<< (LZ << 2))
#define ZB		((uint32_t)FBEFORE	<< (LZ << 2))
#define H0		((uint32_t)FZERO	<< (LH << 2))
#define H1		((uint32_t)FONE		<< (LH << 2))
#define HADD8	((uint32_t)FADD8	<< (LH << 2))
#define HSUB8	((uint32_t)FSUB8	<< (LH << 2))
#define HADD16	((uint32_t)FADD16	<< (LH << 2))
#define HSUB16	((uint32_t)FSUB16	<< (LH << 2))
#define HB		((uint32_t)FBEFORE	<< (LH << 2))
#define HIO		((uint32_t)FIO		<< (LH << 2))
#define PV0		((uint32_t)FZERO	<< (LPV << 2))
#define VADD8	((uint32_t)FADD8	<< (LPV << 2))
#define VSUB8	((uint32_t)FSUB8	<< (LPV << 2))
#define VADD16	((uint32_t)FADD16	<< (LPV << 2))
#define VSUB16	((uint32_t)FSUB16	<< (LPV << 2))
#define PVB		((uint32_t)FBEFORE	<< (LPV << 2))
#define PARITY	((uint32_t)FPARITY	<< (LPV << 2))
#define PV		((uint32_t)FPV		<< (LPV << 2))
#define PVZ		((uint32_t)FPVZ		<< (LPV << 2))
#define N0		((uint32_t)FZERO	<< (LN << 2))
#define N1		((uint32_t)FONE		<< (LN << 2))
#define NB		((uint32_t)FBEFORE	<< (LN << 2))
#define NPV		((uint32_t)FPV		<< (LN << 2))
#define C0		((uint32_t)FZERO	<< (LC << 2))
#define C1		((uint32_t)FONE		<< (LC << 2))
#define CADD8	((uint32_t)FADD8	<< (LC << 2))
#define CSUB8	((uint32_t)FSUB8	<< (LC << 2))
#define CADD16	((uint32_t)FADD16	<< (LC << 2))
#define CSUB16	((uint32_t)FSUB16	<< (LC << 2))
#define CB		((uint32_t)FBEFORE	<< (LC << 2))
#define CIO		((uint32_t)FIO		<< (LC << 2))

#define fmnt()			(++fp < fbuf + FBUFMAX ? 0 : ResolvFlags())
#define fmov(x)			(fp->dm = S8 | Z8 | H0 | PVB | N0, fp->b = Iff2 << LPV, fp->a = (x), fmnt())
#define fbtr(x)			(fp->dm = H0 | PV | N0, fp->pv = (x), fmnt())
#define fcp(x, y, z)	(fp->dm = S8 | Z8 | HSUB8 | PV | N1, fp->pv = (x), fp->b = (y), fp->a = (z), fmnt())
#define fadd(x, y, z)	(fp->dm = S8 | Z8 | HADD8 | VADD8 | N0 | CADD8, fp->b = (x), fp->a = (y), fp->pv = (z), fmnt())
#define fsub(x, y, z)	(fp->dm = S8 | Z8 | HSUB8 | VSUB8 | N1 | CSUB8, fp->b = (x), fp->a = (y), fp->pv = (z), fmnt())
#define finc(x, y)		(fp->dm = S8 | Z8 | HADD8 | VADD8 | N0, fp->b = (x), fp->a = (y), fp->pv = 1, fmnt())
#define fdec(x, y)		(fp->dm = S8 | Z8 | HSUB8 | VSUB8 | N1, fp->b = (x), fp->a = (y), fp->pv = 1, fmnt())
#define fand(x)			(fp->dm = S8 | Z8 | H1 | PARITY | N0 | C0, fp->a = (x), fmnt())
#define fxor(x)			(fp->dm = S8 | Z8 | H0 | PARITY | N0 | C0, fp->a = (x), fmnt())
#define fadd16(x, y, z)	(fp->dm = HADD16 | N0 | CADD16, fp->b = (x), fp->a = (y), fp->pv = (z), fmnt())
#define fadc16(x, y, z)	(fp->dm = S16 | Z16 | HADD16 | VADD16 | N0 | CADD16, fp->b = (x), fp->a = (y), fp->pv = (z), fmnt())
#define fsbc16(x, y, z)	(fp->dm = S16 | Z16 | HSUB16 | VSUB16 | N1 | CSUB16, fp->b = (x), fp->a = (y), fp->pv = (z), fmnt())
#define fdaa(x, y)		(fp->dm = S8 | Z8 | HB | PARITY | CB, fp->b = (x), fp->a = (y), fmnt())
#define fcpl()			(fp->dm = H1 | N1, fmnt())
#define fccf(x)			(fp->dm = HB | N0 | CB, fp->b = (x), fmnt())
#define fscf()			(fp->dm = H0 | N0 | C1, fmnt())
#define frot(x)			(fp->dm = H0 | N0 | CB, fp->b = (x), fmnt())
#define frd(x)			(fp->dm = S8 | Z8 | H0 | PARITY | N0, fp->a = (x), fmnt())
#define fbit(x)			(fp->dm = S0 | Z8 | H1 | PVZ | N0, fp->a = (x) & 1, fmnt())
#define fbits(x)		(fp->dm = S8 | Z8 | H1 | PVZ | N0, fp->a = (x) & 0x80, fmnt())
#define frs(x, y)		(fp->dm = S8 | Z8 | H0 | PARITY | N0 | CB, fp->a = (x), fp->b = (y), fmnt())
#define fin(x, y)		(fp->dm = S8 | Z8 | H0 | PARITY | N0, fp->b = (x), fp->a = (y), fmnt())
#define fbio(x, y, z)	(fp->dm = S8 | Z8 | HIO | NPV | CIO, fp->a = (x), fp->b = (y), fp->pv = (z), fmnt())
#define CY				(ResolvC())

#define swap(a, b)		(tmp2 = (a), (a) = (b), (b) = tmp2)

// --- public methods ---

Z80::Z80() {
	Reset();
#if BUILTIN_MEMORY
	m = nil;
#endif
#if CLOCK_EMU >= 2
	wait = 0;
#endif
#ifdef Z80_TRACE
	for (TraceBuffer *p = tracebuf; p < tracebuf + TRACEMAX; p++) {
		p->pc = p->adr1 = p->data1 = p->adr2 = p->data2 = 0;
		p->r0 = p->r1 = 0;
		p->acs1 = p->acs2 = 0;
	}
	tracep = tracebuf;
	trace_enable = true;
#endif
}

void Z80::Reset() {
#ifdef Z80_DEBUG
	IntVec = a_ = f_ = 0xff;
	hl = 0;
	bcde = 0;
	for (int i = 0; i < 24; i++) r[i] = 0;
#endif
	Iff1 = Iff2 = IntMode = RefReg = RefReg7 = 0;
	pc = 0;
	rofs = 0;
	A = 0xff;
	SetupFlags(0xff);
	sp = 0xffff;
	nmireq = intreq = Iff_set = false;
}

uint16_t Z80::GetHL(void)
{
	return HL;
}


void Z80::SetHL(uint16_t adr)
{
	HL = adr;
}


uint8_t Z80::GetA(void)
{
	return A;
}


uint16_t Z80::GetIX(void)
{
	return (*(uint16_t *)&r[4 + OFS_IX]);
}

int32_t Z80::Execute(int32_t n) {
	int32_t tmp, tmp2, cy;
	bool halt = false;
#if CLOCK_EMU >= 2
	int32_t tmpc;
#endif
#if CLOCK_EMU
	clock = 0;
#endif
	do {
#ifdef Z80_TRACE
		if (!rofs) tracep->pc = pc;
		tracep->acs1 = tracep->acs2 = 0;
#endif
		RefReg++;
		switch (M1) {
#define RST(i) case 0xc7 + 8 * (i): if (Extender(8 * i)) break; st16(sp -= 2, pc); pc = 8 * i; CLOCK(1); break;
			SET_8(RST)
			case 0xfe:
			tmp = IMM8;
			fsub(A, A - tmp, 0); // cp n
			break;
			case 0xf6:
			fxor(A |= IMM8); // or n
			break;
			case 0xee:
			fxor(A ^= IMM8); // xor n
			break;
			case 0xe6:
			fand(A &= IMM8); // and n
			break;
			case 0xde: // sbc a,n
			tmp = IMM8 + (cy = CY);
			fsub(A, tmp2 = A - tmp, cy);
			A = tmp2;
			break;
			case 0xd6: // sub n
			tmp = IMM8;
			fsub(A, tmp2 = A - tmp, 0);
			A = tmp2;
			break;
			case 0xce: // adc a,n
			tmp = IMM8 + (cy = CY);
			fadd(A, tmp2 = A + tmp, cy);
			A = tmp2;
			break;
			case 0xc6: // add a,n
			tmp = IMM8;
			fadd(A, tmp2 = A + tmp, 0);
			A = tmp2;
			break;
			case 0xfd:
			rofs = OFS_IY;
			continue;
			case 0xed:
			RefReg++;
			switch (M1) {
				case 0x6f: // rld
				tmp2 = ld8(HLfix) << 4 | (A & 0xf);
				frd(A = A & 0xf0 | tmp2 >> 8 & 0xf);
				st8(HLfix, tmp2);
				CLOCK(4);
				break;
				case 0x67: // rrd
				tmp2 = ld8(HLfix);
				tmp = tmp2 & 0xf;
				tmp2 = (tmp2 >> 4 & 0xf) | A << 4;
				frd(A = A & 0xf0 | tmp);
				st8(HLfix, tmp2);
				CLOCK(4);
				break;
				case 0x5f:
				fmov(A = RefReg & 0x7f | RefReg7);
				CLOCK(1);
				break;
				case 0x57:
				fmov(A = IntVec);
				CLOCK(1);
				break;
				case 0x4f:
				RefReg = A;
				RefReg7 = A & 0x80;
				CLOCK(1);
				break;
				case 0x47:
				IntVec = A;
				CLOCK(1);
				break;
				case 0x44: case 0x4c: case 0x54: case 0x5c: 
				case 0x64: case 0x6c: case 0x74: case 0x7c: // neg
				fsub(0, tmp2 = -A, 0);
				A = tmp2;
				break;
				case 0x4d: // reti
				RETI();
				// fall
				case 0x45: case 0x55: case 0x5d: case 0x65: case 0x6d: case 0x75: case 0x7d: // retn
				Iff1 = Iff2;
				pc = ld16(sp);
				sp += 2;
				break;
				case 0x7b: // ld sp,(nn)
				sp = ld16(IMM16);
				break;
				// ld bc/de/hl,(nn)
#define LD_REG_NN(i) case 0x4b + 8 * (i): REGFIX16##i = ld16(IMM16); break;
				SET3(LD_REG_NN)
				case 0x73: // ld (nn),sp
				st16(IMM16, sp);
				break;
				// ld (nn),bc/de/hl
#define LD_NN_REG(i) case 0x43 + 8 * (i): st16(IMM16, REGFIX16##i); break;
				SET3(LD_NN_REG)
				case 0x7a: // adc hl,sp
				tmp = sp + (cy = CY);
				fadc16(HLfix, tmp2 = (int32_t)HLfix + tmp, cy);
				HLfix = tmp2;
				CLOCK(7);
				break;
				// adc hl,bc/de/hl
#define ADC_HL_REG(i) case 0x4a + 8 * (i): tmp = REGFIX16##i + (cy = CY); fadc16(HLfix, tmp2 = (int32_t)HLfix + tmp, cy); HLfix = tmp2; CLOCK(7); break;
				SET3(ADC_HL_REG)
				case 0x72: // sbc hl,sp
				tmp = sp + (cy = CY);
				fsbc16(HLfix, tmp2 = (int32_t)HLfix - tmp, cy);
				HLfix = tmp2;
				CLOCK(7);
				break;
				// sbc hl,bc/de/hl
#define SBC_HL_REG(i) case 0x42 + 8 * (i): tmp = REGFIX16##i + (cy = CY); fsbc16(HLfix, tmp2 = (int32_t)HLfix - tmp, cy); HLfix = tmp2; CLOCK(7); break;
				SET3(SBC_HL_REG)
				// out (c),reg
#define OUT_C_REG(i) case 0x41 + 8 * (i): out(BC, REGFIX##i); break;
				SET7(OUT_C_REG)
				// in reg,(c)
#define IN_REG_C(i) case 0x40 + 8 * (i): tmp2 = in(BC); fin(REGFIX##i, tmp2); REGFIX##i = tmp2; break;
				SET7(IN_REG_C)
				case 0x71: // out (c),0
				out(BC, 0);
				break;
				case 0x70: // in (c)
				fin(0, in(BC));
				break;
				case 0xbb: // otdr
				B--;
				out(BC, tmp = ld8(HLfix--));
				fbio(B, L + tmp, tmp);
				if (B) {
					pc -= 2;
					CLOCK(5);
				}
				break;
				case 0xab: // outd
				B--;
				out(BC, tmp = ld8(HLfix--));
				fbio(B, L + tmp, tmp);
				break;
				case 0xba: // indr
				st8(HLfix--, tmp = in(BC));
				fbio(--B, (C - 1 & 0xff) + tmp, tmp);
				if (B) {
					pc -= 2;
					CLOCK(5);
				}
				break;
				case 0xaa: // ind
				st8(HLfix--, tmp = in(BC));
				fbio(--B, (C - 1 & 0xff) + tmp, tmp);
				break;
				case 0xb9: // cpdr
				tmp2 = A - ld8(HLfix);
				HLfix--;
				fcp(--BC, A, tmp2);
				if (BC && tmp2) {
					pc -= 2;
					CLOCK(7);
				}
				else CLOCK(4);
				break;
				case 0xa9: // cpd
				tmp2 = A - ld8(HLfix);
				HLfix--;
				fcp(--BC, A, tmp2);
				CLOCK(2);
				break;
				case 0xb8: // lddr
				st8(DE, ld8(HLfix));
				DE--;
				HLfix--;
				fbtr(--BC);
				if (BC) {
					pc -= 2;
					CLOCK(7);
				}
				else CLOCK(2);
				break;
				case 0xa8: // ldd
				st8(DE, ld8(HLfix));
				DE--;
				HLfix--;
				fbtr(--BC);
				CLOCK(2);
				break;
				case 0xb3: // otir
				B--;
				out(BC, tmp = ld8(HLfix++));
				fbio(B, L + tmp, tmp);
				if (B) {
					pc -= 2;
					CLOCK(5);
				}
				break;
				case 0xa3: // outi
				B--;
				out(BC, tmp = ld8(HLfix++));
				fbio(B, L + tmp, tmp);
				break;
				case 0xb2: // inir
				st8(HLfix++, tmp = in(BC));
				fbio(--B, (C + 1 & 0xff) + tmp, tmp);
				if (B) {
					pc -= 2;
					CLOCK(5);
				}
				break;
				case 0xa2: // ini
				st8(HLfix++, tmp = in(BC));
				fbio(--B, (C + 1 & 0xff) + tmp, tmp);
				break;
				case 0xb1: // cpir
				tmp2 = A - ld8(HLfix);
				HLfix++;
				fcp(--BC, A, tmp2);
				if (BC && tmp2) {
					pc -= 2;
					CLOCK(7);
				}
				else CLOCK(4);
				break;
				case 0xa1: // cpi
				tmp2 = A - ld8(HLfix);
				HLfix++;
				fcp(--BC, A, tmp2);
				CLOCK(2);
				break;
				case 0xb0: // ldir
				st8(DE, ld8(HLfix));
				DE++;
				HLfix++;
				fbtr(--BC);
				if (BC) {
					pc -= 2;
					CLOCK(7);
				}
				else CLOCK(2);
				break;
				case 0xa0: // ldi
				st8(DE, ld8(HLfix));
				DE++;
				HLfix++;
				fbtr(--BC);
				CLOCK(2);
				break;
				case 0x46: case 0x4e: case 0x66: case 0x6e:
				IntMode = 0;
				break;
				case 0x56: case 0x76:
				IntMode = 1;
				break;
				case 0x5e: case 0x7e:
				IntMode = 2;
				break;
				default: // any other instructions are 2 NOPs
				break;
			}
			break;
			case 0xdd:
			rofs = OFS_IX;
			continue;
			case 0xcd: // call nn
			st16(sp -= 2, pc + 2);
			tmp2 = IMM16;
			pc = tmp2;
			CLOCK(1);
			break;
			case 0xf5: // push af
			st16(sp -= 2, A << 8 | ResolvFlags());
			break;
			// push bc/de/hl
#define PUSH(i) case 0xc5 + 8 * (i): st16(sp -= 2, REG16##i); CLOCK(1); break;
			SET3(PUSH)
			// conditional call
#define CALL_COND(i, cond) case 0xc4 + 8 * (i): if (cond) { tmp2 = IMM16; st16(sp -= 2, pc); pc = tmp2; CLOCK(1); } else pc += 2; break;
			COND8(CALL_COND)
			case 0xf3:
			Iff1 = Iff2 = 0;
			break;
			case 0xfb:
			Iff_set = true;
			break;
			case 0xeb: // ex de,hl
			swap(DE, HLfix);
			break;
			case 0xe3: // ex (sp),hl
			tmp2 = ld16(sp);
			st16(sp, HL);
			HL = tmp2;
			CLOCK(3);
			break;
			case 0xdb: // in a,(n)
			A = in(IMM8 | A << 8);
			break;
			case 0xd3: // out (n),a
			out(IMM8 | A << 8, A);
			break;
			case 0xcb:
			RefReg++;
			if (rofs) DDCB_FDCB();
			else switch (M1) {
				// rlc reg
#define RLC(i) case (i): tmp2 = REG##i >> 7; frs(REG##i = REG##i << 1 | tmp2 & 1, tmp2); break;
				SET7(RLC)
				// rlc (hl)
				case 6:
				tmp2 = ld8(HL);
				tmp = tmp2 >> 7;
				frs(tmp2 = tmp2 << 1 | tmp & 1, tmp);
				st8(HL, tmp2);
				CLOCK(1);
				break;
				// rrc reg
#define RRC(i) case 8 + (i): tmp2 = REG##i; frs(REG##i = tmp2 >> 1 | tmp2 << 7, tmp2); break;
				SET7(RRC)
				// rrc (hl)
				case 0xe:
				tmp2 = ld8(HL);
				frs(tmp2 = tmp2 >> 1 | tmp2 << 7, tmp2);
				st8(HL, tmp2);
				CLOCK(1);
				break;
				// rl reg
#define RL(i) case 0x10 + (i): tmp2 = REG##i >> 7; frs(REG##i = REG##i << 1 | CY, tmp2); break;
				SET7(RL)
				// rl (hl)
				case 0x16:
				tmp2 = ld8(HL);
				tmp = tmp2 >> 7;
				frs(tmp2 = tmp2 << 1 | CY, tmp);
				st8(HL, tmp2);
				CLOCK(1);
				break;
				// rr reg
#define RR(i) case 0x18 + (i): tmp2 = REG##i; frs(REG##i = tmp2 >> 1 | CY << 7, tmp2); break;
				SET7(RR)
				// rr(hl)
				case 0x1e:
				tmp2 = ld8(HL);
				tmp = tmp2;
				frs(tmp2 = tmp2 >> 1 | CY << 7, tmp);
				st8(HL, tmp2);
				CLOCK(1);
				break;
				// sla/sll reg
#define SLA(i) case 0x20 + (i): case 0x30 + (i): tmp2 = REG##i >> 7; frs(REG##i <<= 1, tmp2); break;
				SET7(SLA)
				// sla/sll (hl)
				case 0x26: case 0x36:
				tmp2 = ld8(HL);
				tmp = tmp2 >> 7;
				frs(tmp2 <<= 1, tmp);
				st8(HL, tmp2);
				CLOCK(1);
				break;
				// sra reg
#define SRA(i) case 0x28 + (i): tmp2 = (int8_t)REG##i; frs(REG##i = tmp2 >> 1, tmp2); break;
				SET7(SRA)
				// sra (hl)
				case 0x2e:
				tmp = tmp2 = (int8_t)ld8(HL);
				frs(tmp2 >>= 1, tmp);
				st8(HL, tmp2);
				CLOCK(1);
				break;
				// srl reg
#define SRL(i) case 0x38 + (i): tmp2 = REG##i; frs(REG##i = tmp2 >> 1, tmp2); break;
				SET7(SRL)
				// srl (hl)
				case 0x3e:
				tmp = tmp2 = ld8(HL);
				frs(tmp2 >>= 1, tmp);
				st8(HL, tmp2);
				CLOCK(1);
				break;
#define BIT(i, j) case 0x40 + (i) + 8 * (j): fbit(REG##i >> (j)); break;
				SET77BIT(BIT)
#define BIT7(i) case 0x78 + (i): fbits(REG##i); break;
				SET7(BIT7)
#define BIT_HL(j) case 0x46 + 8 * (j): fbit(ld8(HL) >> (j)); CLOCK(1); break;
				SET7BIT(BIT_HL)
				case 0x7e: // bit 7,(hl)
				fbits(ld8(HL));
				CLOCK(1);
				break;
#define RES(i, j) case 0x80 + (i) + 8 * (j): REG##i &= ~(1 << (j)); break;
				SET78(RES)
#define RES_HL(j) case 0x86 + 8 * (j): st8(HL, ld8(HL) & ~(1 << (j))); CLOCK(1); break;
				SET_8(RES_HL)
#define SET(i, j) case 0xc0 + (i) + 8 * (j): REG##i |= 1 << (j); break;
				SET78(SET)
#define SET_HL(j) case 0xc6 + 8 * (j): st8(HL, ld8(HL) | 1 << (j)); CLOCK(1); break;
				SET_8(SET_HL)
#ifdef Z80_DEBUG
				default:
				ill();
				break;
#endif
			}
			RefReg++;
			break;
			case 0xc3: // jp nn
			tmp2 = IMM16;
			pc = tmp2;
			break;
			// conditional jump
#define JP_COND(i, cond) case 0xc2 + 8 * (i): tmp2 = IMM16; if (cond) pc = tmp2; break;
			COND8(JP_COND)
			case 0xf9: // ld sp,hl
			sp = HL;
			CLOCK(2);
			break;
			case 0xe9: // jp (hl)
			pc = HL;
			break;
			case 0xd9: // exx
			swap(*(uint32_t *)r, bcde);
			swap(HLfix, hl);
			break;
			case 0xc9: // ret
			pc = ld16(sp);
			sp += 2;
			break;
			case 0xf1: // pop af
			tmp = ld16(sp);
			sp += 2;
			SetupFlags(tmp);
			A = tmp >> 8;
			break;
			// pop bc/de/hl
#define POP(i) case 0xc1 + 8 * (i): REG16##i = ld16(sp); sp += 2; break;
			SET3(POP)
			// conditional return
#define RET_COND(i, cond) case 0xc0 + 8 * (i): if (cond) { pc = ld16(sp); sp += 2; CLOCK(7); } else CLOCK(1); break;
			COND8(RET_COND)
			case 0xbe: // cp (hl)
			tmp = ld8(HL);
			fsub(A, A - tmp, 0);
			if (rofs) {
				pc++;
				CLOCK(4);
			}
			break;
			// cp reg
#define CP(i) case 0xb8 + (i): tmp = REG##i; fsub(A, A - tmp, 0); break;
			SET7(CP)
			// or (hl)
			case 0xb6:
			fxor(A |= ld8(HL));
			if (rofs) {
				pc++;
				CLOCK(4);
			}
			break;
			// or reg
#define OR(i) case 0xb0 + (i): fxor(A |= REG##i); break;
			SET7(OR)
			// xor (hl)
			case 0xae:
			fxor(A ^= ld8(HL));
			if (rofs) {
				pc++;
				CLOCK(4);
			}
			break;
			// xor reg
#define XOR(i) case 0xa8 + (i): fxor(A ^= REG##i); break;
			SET7(XOR)
			// and (hl)
			case 0xa6:
			fand(A &= ld8(HL));
			if (rofs) {
				pc++;
				CLOCK(4);
			}
			break;
			// and reg
#define AND(i) case 0xa0 + (i): fand(A &= REG##i); break;
			SET7(AND)
			// sbc (hl)
			case 0x9e:
			tmp = ld8(HL) + (cy = CY);
			fsub(A, tmp2 = A - tmp, cy);
			A = tmp2;
			if (rofs) {
				pc++;
				CLOCK(4);
			}
			break;
			// sbc reg
#define SBC8(i) case 0x98 + (i): tmp = REG##i + (cy = CY); fsub(A, tmp2 = A - tmp, cy); A = tmp2; break;
			SET7(SBC8)
			// sub (hl)
			case 0x96:
			tmp = ld8(HL);
			fsub(A, tmp2 = A - tmp, 0);
			A = tmp2;
			if (rofs) {
				pc++;
				CLOCK(4);
			}
			break;
			// sub reg
#define SUB(i) case 0x90 + (i): tmp = REG##i; fsub(A, tmp2 = A - tmp, 0); A = tmp2; break;
			SET7(SUB)
			// adc (hl)
			case 0x8e:
			tmp = ld8(HL) + (cy = CY);
			fadd(A, tmp2 = A + tmp, cy);
			A = tmp2;
			if (rofs) {
				pc++;
				CLOCK(4);
			}
			break;
			// adc reg
#define ADC8(i) case 0x88 + (i): tmp = REG##i + (cy = CY); fadd(A, tmp2 = A + tmp, cy); A = tmp2; break;
			SET7(ADC8)
			// add (hl)
			case 0x86:
			tmp = ld8(HL);
			fadd(A, tmp2 = A + tmp, 0);
			A = tmp2;
			if (rofs) {
				pc++;
				CLOCK(4);
			}
			break;
			// add reg
#define ADD8(i) case 0x80 + (i): tmp = REG##i; fadd(A, tmp2 = A + tmp, 0); A = tmp2; break;
			SET7(ADD8)
			case 0x76:
			Halt();
			pc--;
			halt = true;
			break;
			// ld (hl),reg
#define LD_HL_REG(i) case 0x70 + (i): st8(HL, REGFIX##i); if (rofs) { pc++; CLOCK(4); } break;
			SET7(LD_HL_REG)
			// ld reg,(hl)
#define LD_REG_HL(i) case 0x46 + 8 * (i): REGFIX##i = ld8(HL); if (rofs) { pc++; CLOCK(4); } break;
			SET7(LD_REG_HL)
#define REG_REG(i, j) case 0x40 + (i) + 8 * (j): REG##j = REG##i; break;
			SET77REG(REG_REG)
			case 0x3f: // ccf
			tmp = ResolvC();
			fccf(tmp << LH | !tmp);
			break;
			case 0x37: // scf
			fscf();
			break;
			case 0x2f: // cpl
			A = ~A;
			fcpl();
			break;
			case 0x27: // daa
			if (ResolvN()) {
				if (ResolvH()) 
					if (CY) tmp = A - 0x66;
					else tmp = A - 6 - 0x60 * (A >= 0x9a);
				else 
					if (CY) tmp = A - 6 * ((A & 0xf) >= 10) - 0x60;
					else tmp = A - 6 * ((A & 0xf) >= 10) - 0x60 * ((A >> 4 & 0xf) >= 10);
				fdaa(((A & 0xf) < 6) << LH | CY | (A >= 0x9a), tmp);
			}
			else {
				if (ResolvH()) 
					if (CY) tmp = A + 0x66;
					else tmp = A + 6 + 0x60 * (A >= 0x9a);
				else 
					if (CY) tmp = A + 6 * ((A & 0xf) >= 10) + 0x60;
					else tmp = A + 6 * ((A & 0xf) >= 10) + 0x60 * ((A >> 4 & 0xf) >= 10);
				fdaa(((A & 0xf) >= 10) << LH | CY | (A >= 0x9a), tmp);
			}
			A = tmp;
			break;
			case 0x1f: // rra
			tmp2 = A;
			A = A >> 1 | CY << 7;
			frot(tmp2);
			break;
			case 0x17: // rla
			tmp2 = A >> 7;
			A = A << 1 | CY;
			frot(tmp2);
			break;
			case 0xf: // rrca
			frot(A);
			A = A >> 1 | A << 7;
			break;
			case 7: // rlca
			frot(tmp2 = A >> 7);
			A = A << 1 | (tmp2 & 1);
			break;
			case 0x36: // ld (hl),n
			if (rofs) {
				st8(HL, load(pc + 1));
				pc += 2;
				CLOCK_MEM();
			}
			else st8(HL, IMM8);
			break;
			// ld reg,n
#define LD_REG_N(i) case 6 + 8 * (i): REG##i = IMM8; break;
			SET7(LD_REG_N)
			// dec (hl)
			case 0x35:
			tmp2 = ld8(HL);
			fdec(tmp2, tmp2 - 1);
			st8(HL, tmp2 - 1);
			if (rofs) {
				pc++;
				CLOCK(7);
			}
			else CLOCK(1);
			break;
			// dec reg
#define DEC8(i) case 5 + 8 * (i): tmp2 = REG##i; fdec(tmp2, tmp2 - 1); REG##i = tmp2 - 1; break;
			SET7(DEC8)
			// inc (hl)
			case 0x34:
			tmp2 = ld8(HL);
			finc(tmp2, tmp2 + 1);
			st8(HL, tmp2 + 1);
			if (rofs) {
				pc++;
				CLOCK(7);
			}
			else CLOCK(1);
			break;
			// inc reg
#define INC8(i) case 4 + 8 * (i): tmp2 = REG##i; finc(tmp2, tmp2 + 1); REG##i = tmp2 + 1; break;
			SET7(INC8)
			case 0x3b:
			sp--;
			break;
#define DEC16(i) case 0xb + 8 * (i): (REG16##i)--; CLOCK(2); break;
			SET3(DEC16)
			case 0x33:
			sp++;
			break;
#define INC16(i) case 3 + 8 * (i): (REG16##i)++; CLOCK(2); break;
			SET3(INC16)
			case 0x3a: // ld a,(nn)
			A = ld8(IMM16);
			break;
			case 0x32: // ld (nn),a
			st8(IMM16, A);
			break;
			case 0x2a: // ld hl,(nn)
			HL = ld16(IMM16);
			break;
			case 0x22: // ld (nn),hl
			st16(IMM16, HL);
			break;
			// ld a,(bc)/(de)
#define LD_A_MEM(i) case 0xa + 8 * (i): A = ld8(REG16##i); break;
			SET2(LD_A_MEM)
			// ld (bc)/(de),a
#define LD_MEM_A(i) case 2 + 8 * (i): st8(REG16##i, A); break;
			SET2(LD_MEM_A)
			case 0x39: // add hl,sp
			tmp = sp;
			fadd16(HL, tmp2 = (int32_t)HL + tmp, 0);
			HL = tmp2;
			CLOCK(7);
			break;
			// add hl,bc/de/hl
#define ADD_HL_REG(i) case 9 + 8 * (i): tmp = REG16##i; fadd16(HL, tmp2 = (int32_t)HL + tmp, 0); HL = tmp2; CLOCK(7); break;
			SET3(ADD_HL_REG)
			case 0x31:
			sp = IMM16;
			break;
#define LD_REG_IMM(i) case 1 + 8 * (i): REG16##i = IMM16; break;
			SET3(LD_REG_IMM)
			case 0x38:
			tmp2 = (int8_t)load(pc++);
			CLOCK_MEM();
			if (ResolvC()) pc += tmp2;
			CLOCK(5);
			break;
			case 0x30:
			tmp2 = (int8_t)load(pc++);
			CLOCK_MEM();
			if (!ResolvC()) pc += tmp2;
			CLOCK(5);
			break;
			case 0x28:
			tmp2 = (int8_t)load(pc++);
			CLOCK_MEM();
			if (ResolvZ()) pc += tmp2;
			CLOCK(5);
			break;
			case 0x20:
			tmp2 = (int8_t)load(pc++);
			CLOCK_MEM();
			if (!ResolvZ()) pc += tmp2;
			CLOCK(5);
			break;
			case 0x18:
			tmp2 = (int8_t)load(pc++);
			CLOCK_MEM();
			pc += tmp2;
			CLOCK(5);
			break;
			case 0x10:
			tmp2 = (int8_t)load(pc++);
			CLOCK_MEM();
			if (--B) {
				pc += tmp2;
				CLOCK(5);
			}
			else CLOCK(3);
			break;
			case 8: // ex af,af'
			swap(A, a_);
			tmp2 = f_;
			f_ = ResolvFlags();
			SetupFlags(tmp2);
			break;
			case 0: // nop
			break;
#ifdef Z80_DEBUG
			default:
			ill();
			break;
#endif
		}
		rofs = 0;
		if (nmireq) {
			nmireq = Iff_set = false;
			Iff1 = 0;
			if (halt) {
				halt = false;
				pc++;
			}
			st16(sp -= 2, pc);
			pc = 0x66;
		}
		else if (intreq && Iff1) {
			intreq = Iff_set = false;
			Iff1 = Iff2 = 0;
			if (halt) {
				halt = false;
				pc++;
			}
			switch (IntMode) {
				case 0: // only RST instructions supported
				if ((intdata & 0xc7) == 0xc7) {
					st16(sp -= 2, pc);
					pc = intdata & 0x38;
				}
				break;
				case 1:
				st16(sp -= 2, pc);
				pc = 0x38;
				break;
				case 2:
				st16(sp -= 2, pc);
				pc = ld16(IntVec << 8 | (intdata & 0xff));
				break;
			}
		}
		else if (Iff_set) {
			Iff_set = false;
			Iff1 = Iff2 = 1;
		}
#ifdef Z80_TRACE
		F = ResolvFlags();
		tracep->r0 = *(int32_t *)&r[0];
		tracep->r1 = *(int32_t *)&r[4];
		if (trace_enable && ++tracep >= tracebuf + TRACEMAX) tracep = tracebuf;
#endif
	}
#if CLOCK_EMU
	while (!halt && clock < n);
	return clock - n;
#else
	while (!halt && --n > 0);
	return -n;
#endif
}

// --- private methods ---

void Z80::DDCB_FDCB() {
	int32_t tmp, tmp2 = loadm1(pc + 1);
	CLOCK_M1();
	switch (tmp2) {
		// rlc reg
#define RLCI(i) case (i): REG##i = ld8(HL); tmp2 = REG##i >> 7; frs(REG##i = REG##i << 1 | tmp2 & 1, tmp2); st8(HL, REG##i); break;
		SET7(RLCI)
		// rlc (hl)
		case 6:
		tmp2 = ld8(HL);
		tmp = tmp2 >> 7;
		frs(tmp2 = tmp2 << 1 | tmp & 1, tmp);
		st8(HL, tmp2);
		pc += 2;
		CLOCK(1);
		break;
		// rrc reg
#define RRCI(i) case 8 + (i): REG##i = ld8(HL); tmp2 = REG##i; frs(REG##i = tmp2 >> 1 | tmp2 << 7, tmp2); st8(HL, REG##i); break;
		SET7(RRCI)
		// rrc (hl)
		case 0xe:
		tmp2 = ld8(HL);
		frs(tmp2 = tmp2 >> 1 | tmp2 << 7, tmp2);
		st8(HL, tmp2);
		pc += 2;
		CLOCK(1);
		break;
		// rl reg
#define RLI(i) case 0x10 + (i): REG##i = ld8(HL); tmp2 = REG##i >> 7; frs(REG##i = REG##i << 1 | CY, tmp2); st8(HL, REG##i); break;
		SET7(RLI)
		// rl (hl)
		case 0x16:
		tmp2 = ld8(HL);
		tmp = tmp2 >> 7;
		frs(tmp2 = tmp2 << 1 | CY, tmp);
		st8(HL, tmp2);
		pc += 2;
		CLOCK(1);
		break;
		// rr reg
#define RRI(i) case 0x18 + (i): REG##i = ld8(HL); tmp2 = REG##i; frs(REG##i = tmp2 >> 1 | CY << 7, tmp2); st8(HL, REG##i); break;
		SET7(RRI)
		// rr(hl)
		case 0x1e:
		tmp2 = ld8(HL);
		tmp = tmp2;
		frs(tmp2 = tmp2 >> 1 | CY << 7, tmp);
		st8(HL, tmp2);
		pc += 2;
		CLOCK(1);
		break;
		// sla/sll reg
#define SLAI(i) case 0x20 + (i): case 0x30 + (i): REG##i = ld8(HL); tmp2 = REG##i >> 7; frs(REG##i <<= 1, tmp2); st8(HL, REG##i); break;
		SET7(SLAI)
		// sla/sll (hl)
		case 0x26: case 0x36:
		tmp2 = ld8(HL);
		tmp = tmp2 >> 7;
		frs(tmp2 <<= 1, tmp);
		st8(HL, tmp2);
		pc += 2;
		CLOCK(1);
		break;
		// sra reg
#define SRAI(i) case 0x28 + (i): REG##i = ld8(HL); tmp2 = (int8_t)REG##i; frs(REG##i = tmp2 >> 1, tmp2); st8(HL, REG##i); break;
		SET7(SRAI)
		// sra (hl)
		case 0x2e:
		tmp = tmp2 = (int8_t)ld8(HL);
		frs(tmp2 >>= 1, tmp);
		st8(HL, tmp2);
		pc += 2;
		CLOCK(1);
		break;
		// srl reg
#define SRLI(i) case 0x38 + (i): REG##i = ld8(HL); tmp2 = REG##i; frs(REG##i = tmp2 >> 1, tmp2); st8(HL, REG##i); break;
		SET7(SRLI)
		// srl (hl)
		case 0x3e:
		tmp = tmp2 = ld8(HL);
		frs(tmp2 >>= 1, tmp);
		st8(HL, tmp2);
		pc += 2;
		CLOCK(1);
		break;
#define BITI(i, j) case 0x40 + (i) + 8 * (j): fbit(ld8(HL) >> (j)); pc += 2; CLOCK(1); break;
		SET77BIT(BITI)
#define BITI7(i) case 0x78 + (i): fbits(ld8(HL)); pc += 2; CLOCK(1); break;
		SET7(BITI7)
#define BIT_I(j) case 0x46 + 8 * (j): fbit(ld8(HL) >> (j)); pc += 2; CLOCK(1); break;
		SET7BIT(BIT_I)
		case 0x7e:
		fbits(ld8(HL));
		pc += 2;
		CLOCK(1);
		break;
#define RESI(i, j) case 0x80 + (i) + 8 * (j): REG##i = ld8(HL) & ~(1 << (j)); st8(HL, REG##i); CLOCK(1); break;
		SET78(RESI)
#define RES_I(j) case 0x86 + 8 * (j): st8(HL, ld8(HL) & ~(1 << (j))); pc += 2; CLOCK(1); break;
		SET_8(RES_I)
#define SETI(i, j) case 0xc0 + (i) + 8 * (j): REG##i = ld8(HL) | 1 << (j); st8(HL, REG##i); CLOCK(1); break;
		SET78(SETI)
#define SET_I(j) case 0xc6 + 8 * (j): st8(HL, ld8(HL) | 1 << (j)); pc += 2; CLOCK(1); break;
		SET_8(SET_I)
#ifdef Z80_DEBUG
		default:
		ill();
		break;
#endif
	}
}

int Z80::ResolvC() {
	uint32_t sw = 0;
	FlagDecision *p;
	for (p = fp - 1; p >= fbuf && !(sw = p->dm & 0xf); p--)
		;
#ifdef Z80_DEBUG
	if (p < fbuf) Halt();
#endif
	switch (sw) {
		case FZERO:
		break;
		case FONE:
		return MC;
		case FBEFORE:
		return p->b & MC;
		case FADD8:
		return (p->b < p->a && (uint8_t)p->b >= (uint8_t)p->a) << LC;
		case FSUB8:
		return (p->b > p->a && (uint8_t)p->b <= (uint8_t)p->a) << LC;
		case FADD16:
		return (p->b < p->a && (uint16_t)p->b >= (uint16_t)p->a) << LC;
		case FSUB16:
		return (p->b > p->a && (uint16_t)p->b <= (uint16_t)p->a) << LC;
		case FIO:
		return ((p->b & ~0xff) != 0) << LC;
#ifdef Z80_DEBUG
		default:
		Halt();
		break;
#endif
	}
	return 0;
}

int Z80::ResolvN() {
	uint32_t sw = 0;
	FlagDecision *p;
	for (p = fp - 1; p >= fbuf && !(sw = p->dm & 0xf0); p--)
		;
#ifdef Z80_DEBUG
	if (p < fbuf) Halt();
#endif
	switch (sw >> 4) {
		case FZERO:
		break;
		case FONE:
		return MN;
		case FBEFORE:
		return p->b & MN;
		case FPV:
		return p->pv >> 7 << LN;
#ifdef Z80_DEBUG
		default:
		Halt();
		break;
#endif
	}
	return 0;
}

int Z80::ResolvPV() {
	uint32_t sw = 0;
	FlagDecision *p;
	for (p = fp - 1; p >= fbuf && !(sw = p->dm & 0xf00); p--)
		;
#ifdef Z80_DEBUG
	if (p < fbuf) Halt();
#endif
	int x;
	switch (sw >> 8) {
		case FZERO:
		break;
		case FBEFORE:
		return p->b & MPV;
		case FADD8:
		return (~(p->b ^ p->a - p->b) & 0x80 && (p->b ^ p->a) & 0x80) << LPV;
		case FSUB8:
		return ((p->b ^ p->b - p->a) & 0x80 && (p->b ^ p->a) & 0x80) << LPV;
		case FADD16:
		return (~(p->b ^ p->a - p->b) & 0x8000 && (p->b ^ p->a) & 0x8000) << LPV;
		case FSUB16:
		return ((p->b ^ p->b - p->a) & 0x8000 && (p->b ^ p->a) & 0x8000) << LPV;
		case FPARITY:
		x = p->a;
		x ^= x >> 4;
		x ^= x << 2;
		x ^= x >> 1;
		return ~x & MPV;
		case FPV:
		return (p->pv != 0) << LPV;
		case FPVZ:
		return !(p->a & 0xff) << LPV;
		case FIO:
		x = (p->b & 7) ^ p->a;
		x ^= x >> 4;
		x ^= x << 2;
		x ^= x >> 1;
		return ~x & MPV;
#ifdef Z80_DEBUG
		default:
		Halt();
		break;
#endif
	}
	return 0;
}

int Z80::ResolvH() {
	uint32_t sw = 0;
	FlagDecision *p;
	for (p = fp - 1; p >= fbuf && !(sw = p->dm & 0xf0000); p--)
		;
#ifdef Z80_DEBUG
	if (p < fbuf) Halt();
#endif
	switch (sw >> 16) {
		case FZERO:
		break;
		case FONE:
		return MH;
		case FBEFORE:
		return p->b & MH;
		case FADD8:
		return ((p->b & 0xf) > (p->a & 0xf) || (p->pv && (p->b & 0xf) == (p->a & 0xf))) << LH;
		case FSUB8:
		return ((p->b & 0xf) < (p->a & 0xf) || (p->pv && (p->b & 0xf) == (p->a & 0xf))) << LH;
		case FADD16:
		return ((p->b & 0xfff) > (p->a & 0xfff) || (p->pv && (p->b & 0xfff) == (p->a & 0xfff))) << LH;
		case FSUB16:
		return ((p->b & 0xfff) < (p->a & 0xfff) || (p->pv && (p->b & 0xfff) == (p->a & 0xfff))) << LH;
		case FIO:
		return p->b & ~0xff;
#ifdef Z80_DEBUG
		default:
		Halt();
		break;
#endif
	}
	return 0;
}

int Z80::ResolvZ() {
	uint32_t sw = 0;
	FlagDecision *p;
	for (p = fp - 1; p >= fbuf && !(sw = p->dm & 0xf000000); p--)
		;
#ifdef Z80_DEBUG
	if (p < fbuf) Halt();
#endif
	switch (sw >> 24) {
		case FONE:
		return MZ;
		case FBEFORE:
		return p->b & MZ;
		case FADD8:
		return !(p->a & 0xff) << LZ;
		case FADD16:
		return !(p->a & 0xffff) << LZ;
#ifdef Z80_DEBUG
		default:
		Halt();
		break;
#endif
	}
	return 0;
}

int Z80::ResolvS() {
	uint32_t sw = 0;
	FlagDecision *p;
	for (p = fp - 1; p >= fbuf && !(sw = p->dm & 0xf0000000); p--)
		;
#ifdef Z80_DEBUG
	if (p < fbuf) Halt();
#endif
	switch (sw >> 28) {
		case FZERO:
		break;
		case FBEFORE:
		return p->b & MS;
		case FADD8:
		return ((p->a & 0x80) != 0) << LS;
		case FADD16:
		return ((p->a & 0x8000) != 0) << LS;
#ifdef Z80_DEBUG
		default:
		Halt();
		break;
#endif
	}
	return 0;
}

void Z80::SetupFlags(int x) {
	fp = fbuf;
	fp->dm = SB | ZB | HB | PVB | NB | CB;
	fp++->b = x;
}

int Z80::ResolvFlags() {
	int r = ResolvC() | ResolvN() | ResolvPV() | ResolvH() | ResolvZ() | ResolvS() | (fbuf->b & 0x28);
	SetupFlags(r);
	return r;
}

// --- debugging methods ---

#ifdef Z80_TRACE

void Z80::StopTrace() {
	TraceBuffer *endp = tracep;
	int i = 0;
	FILE *fo;
	if (!(fo = fopen("trace.dat", "w"))) exit(1);
	do {
		if (++tracep >= tracebuf + TRACEMAX) tracep = tracebuf;
		fprintf(fo, "%7d %04x\t", i++, tracep->pc);
		fprintf(fo, "%04x %04x %04x %04x ", 
#if ENDIAN_SWITCH
			(uint16_t)(tracep->r0 >> 16), (uint16_t)tracep->r0, (uint16_t)(tracep->r1 >> 16), (uint16_t)tracep->r1);
#else
			(uint16_t)tracep->r0, (uint16_t)(tracep->r0 >> 16), (uint16_t)tracep->r1, (uint16_t)(tracep->r1 >> 16));
#endif
		switch (tracep->acs1) {
			case acsLoad8:
			fprintf(fo, "L %04x %02x ", tracep->adr1, tracep->data1 & 0xff);
			break;
			case acsLoad16:
			fprintf(fo, "L %04x %04x ", tracep->adr1, tracep->data1);
			break;
			case acsIn:
			fprintf(fo, "I %04x %02x ", tracep->adr1, tracep->data1 & 0xff);
			break;
		}
		switch (tracep->acs2) {
			case acsStore8:
			fprintf(fo, "S %04x %02x ", tracep->adr2, tracep->data2 & 0xff);
			break;
			case acsStore16:
			fprintf(fo, "S %04x %04x ", tracep->adr2, tracep->data2);
			break;
			case acsOut:
			fprintf(fo, "O %04x %02x ", tracep->adr2, tracep->data2 & 0xff);
			break;
		}
		fprintf(fo, "\n");
	} while (tracep != endp);
	fclose(fo);
	exit(1);
}

#endif	// Z80_TRACE

#ifdef Z80_DEBUG

void Z80::Halt() {
	printf("CPU Halted.\n");
#ifdef Z80_TRACE
	StopTrace();
#endif
	abort();
}

void Z80::ill() {
	printf("Illegal instruction.\n");
#ifdef Z80_TRACE
	StopTrace();
#endif
	abort();
}

#endif // Z80_DEBUG

