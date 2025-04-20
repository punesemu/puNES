/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <string.h>
#include "nes.h"
#include "apu.h"
#include "cpu_inline.h"

enum cpu_opcode_type { RD_OP, WR_OP };

// ----------------------------------------------------------------------
//  metodi di indirizzamento
// ----------------------------------------------------------------------
#define IMP(opTy, cmd)\
{\
	cmd\
}
#define ZPG(opTy, cmd)\
{\
	WORD adr0 = _RDP;\
	cmd\
}
#define ZPX(opTy, cmd, reg)\
{\
	WORD adr1 = _RDP;\
	/* garbage read */\
	_RDZPX_;\
	WORD adr0 = (adr1 + (reg)) & 0x00FF;\
	cmd\
}
#define ABS(opTy, cmd)\
{\
	WORD adr0 = lend_word(nidx, nes[nidx].c.cpu.PC.w, FALSE, TRUE);\
	nes[nidx].c.cpu.PC.w += 2;\
	cmd\
}
#define ABW(opTy, cmd)\
{\
	WORD adr0 = lend_word(nidx, nes[nidx].c.cpu.PC.w, FALSE, FALSE);\
	_DMC\
	nes[nidx].c.cpu.PC.w += 2;\
	cmd\
}
#define ABX(opTy, cmd, reg)\
{\
	WORD adr2 = lend_word(nidx, nes[nidx].c.cpu.PC.w, FALSE, TRUE);\
	WORD adr0 = adr2 + (reg);\
	WORD adr1 = (adr2 & 0xFF00) | (BYTE)adr0;\
	nes[nidx].c.cpu.PC.w += 2;\
	/* puo' essere la lettura corretta o anche semplice garbage */\
	_RDABX_;\
	cmd\
}
#define IDR(opTy)\
{\
	WORD adr0 = lend_word(nidx, nes[nidx].c.cpu.PC.w, FALSE, TRUE);\
	nes[nidx].c.cpu.PC.w = lend_word(nidx, adr0, TRUE, TRUE);\
}
#define IDX(opTy, cmd)\
{\
	WORD adr1 = _RDP;\
	/* garbage read */\
	_RDIDX_;\
	WORD adr0 = lend_word(nidx, (adr1 + nes[nidx].c.cpu.XR) & 0x00FF, TRUE, TRUE);\
	cmd\
}
#define IDY(opTy, cmd)\
{\
	WORD adr2 = lend_word(nidx, _RDP, TRUE, TRUE);\
	WORD adr0 = adr2 + nes[nidx].c.cpu.YR;\
	WORD adr1 = (adr2 & 0xFF00) | (BYTE)adr0;\
	/* puo' essere la lettura corretta o anche semplice garbage */\
	_RDIDY_;\
	cmd\
}

// ----------------------------------------------------------------------
//  istruzioni ufficiali
// ----------------------------------------------------------------------
// ADC, SBC
#define ADC(x)\
	x;\
	_ADC
#define SBC(x)\
	x;\
	_SBC
// AND, DEC, EOR, INC, ORA
#define AND(x, opr) _RSZ(nes[nidx].c.cpu.AR opr x;, nes[nidx].c.cpu.AR)
#define INC(x, opr)\
	{\
	_MSZ(BYTE tmp = x opr 1, tmp, tmp)\
	}
// ASL, LSR, ROL, ROR
#define ASL(x) _SHF(x, _BSH(shift, 0x80, <<=), shift, shift)
#define LSR(x) _SHF(x, _BSH(shift, 0x01, >>=), shift, shift)
#define ROL(x) _SHF(x, _ROL(shift, 0x80, <<=), shift, shift)
#define ROR(x) _SHF(x, _ROR(shift, 0x01, >>=), shift, shift)
// BCC, BCS, BEQ, BMI, BNF, BPL, BVC, BVS
#define BRC(flag, condition) \
	BYTE offset = _RDP;\
	WORD adr0 = nes[nidx].c.cpu.PC.w + (SBYTE)offset;\
	if ((!flag) != condition) {\
		/* A page boundary crossing occurs when the branch destination is on a different page\
		 * than the instruction AFTER the branch instruction. */\
		BYTE cross = !((adr0 & 0xFF00) == (nes[nidx].c.cpu.PC.w & 0xFF00));\
		if (!cross) {\
			if (nes[nidx].c.nmi.high && !nes[nidx].c.nmi.before) {\
				nes[nidx].c.nmi.delay = TRUE;\
			} else if (!(nes[nidx].c.irq.inhibit & 0x04) && nes[nidx].c.irq.high && !nes[nidx].c.irq.before) {\
				nes[nidx].c.irq.delay = TRUE;\
			}\
		}\
		mod_cycles_op(+=, 1);\
		_RDD;\
		nes[nidx].c.cpu.PC.b[0] += offset;\
		if (offset & 0x80) {\
			if (cross) {\
				mod_cycles_op(+=, 1);\
				_RDD;\
				nes[nidx].c.cpu.PC.b[1]--;\
			}\
		} else {\
			if (cross) {\
				mod_cycles_op(+=, 1);\
				_RDD;\
				nes[nidx].c.cpu.PC.b[1]++;\
			}\
		}\
	}
// BIT
#define BIT(x)\
	x;\
	nes[nidx].c.cpu.sf = (nes[nidx].c.cpu.openbus & 0x80);\
	nes[nidx].c.cpu.of = (nes[nidx].c.cpu.openbus & 0x40);\
	ZF((nes[nidx].c.cpu.AR & nes[nidx].c.cpu.openbus))
// BRK, PHP - NOTE: lo Status Register viene salvato solo nello stack con il bf settato a 1.
#define BRK\
	/* dummy read */\
	_RDP;\
	_IRQ(nes[nidx].c.cpu.SR | 0x10)
#define PHP\
	_RDD;\
	assemble_SR(nidx);\
	_PSH(nes[nidx].c.cpu.SR | 0x10);
// CMP, CPX, CPY
#define CMP(x, reg)\
	{\
	_RSZ(_CMP(x, reg), (BYTE)cmp)\
	}
// LDA, LDX, LDY
#define LDX(x, reg) _RSZ((reg) = (x);, reg)
// JMP
#define JMP\
	WORD adr0 = lend_word(nidx, nes[nidx].c.cpu.PC.w, FALSE, TRUE);\
	nes[nidx].c.cpu.PC.w = adr0;
// JSR, RTS
#define JSR\
	WORD adr0 = lend_word(nidx, nes[nidx].c.cpu.PC.w++, FALSE, TRUE);\
	_PSP\
	nes[nidx].c.cpu.PC.w = adr0;
#define RTS\
	/* dummy read */\
	_RDD;\
	_RDX((nes[nidx].c.cpu.SP | STACK), TRUE);\
	nes[nidx].c.cpu.PC.b[0] = _PUL;\
	nes[nidx].c.cpu.PC.b[1] = _PUL;\
	_RDP;
// RTI
#define RTI\
	/* dummy read */\
	_RDD;\
	_RDX((nes[nidx].c.cpu.SP | STACK), TRUE);\
	/* il break flag (bit 4) e' sempre a 0 */\
	nes[nidx].c.cpu.SR = (_PUL & 0xEF);\
	disassemble_SR(nidx);\
	/* nell'RTI non c'e' nessun delay nel settaggio dell'inibizione dell'IRQ.*/\
	nes[nidx].c.irq.inhibit = nes[nidx].c.cpu.im;\
	nes[nidx].c.cpu.PC.b[0] = _PUL;\
	nes[nidx].c.cpu.PC.b[1] = _PUL;
// SEI, PHA, PLA, PLP
#define SEI\
	nes[nidx].c.cpu.im = 0x04;\
	nes[nidx].c.irq.inhibit |= 0x40;
#define PHA\
	_RDD;\
	_PSH(nes[nidx].c.cpu.AR);
#define PLA\
	_RDD;\
	_RDX((nes[nidx].c.cpu.SP | STACK), TRUE);\
	_RSZ(nes[nidx].c.cpu.AR = _PUL;, nes[nidx].c.cpu.AR)
#define PLP\
	_RDD;\
	_RDX((nes[nidx].c.cpu.SP | STACK), TRUE);\
	/* il break flag (bit 4) e' sempre a 0 */\
	nes[nidx].c.cpu.SR = (_PUL & 0xEF);\
	disassemble_SR(nidx);\
	if (nes[nidx].c.cpu.im) {\
		nes[nidx].c.irq.inhibit |= 0x40;\
	}

// ----------------------------------------------------------------------
//  istruzioni non ufficiali
// ----------------------------------------------------------------------

// AAC, ASR, ARR
#define AAC\
	AND(_RDP, &=)\
	nes[nidx].c.cpu.cf = nes[nidx].c.cpu.sf >> 7;
#define ASR\
	nes[nidx].c.cpu.AR &= _RDP;\
	_RSZ(_BSH(nes[nidx].c.cpu.AR, 0x01, >>=), nes[nidx].c.cpu.AR)
#define ARR\
	nes[nidx].c.cpu.AR &= _RDP;\
	_RSZ(_ROR(nes[nidx].c.cpu.AR, 0x01, >>=), nes[nidx].c.cpu.AR)\
	nes[nidx].c.cpu.cf = (nes[nidx].c.cpu.AR & 0x40) >> 6;\
	nes[nidx].c.cpu.of = (nes[nidx].c.cpu.AR & 0x40) ^ ((nes[nidx].c.cpu.AR & 0x20) << 1);
// ATX
#define ATX _RSZ(nes[nidx].c.cpu.XR = nes[nidx].c.cpu.AR = _RDP;, nes[nidx].c.cpu.AR)
// AXS
#define AXS\
	nes[nidx].c.cpu.XR &= nes[nidx].c.cpu.AR;\
	_CMP(_RDP, nes[nidx].c.cpu.XR)\
	_RSZ(nes[nidx].c.cpu.XR = (BYTE)cmp;, nes[nidx].c.cpu.XR)
// AAX
#define AAX\
	BYTE tmp;\
	tmp = nes[nidx].c.cpu.AR & nes[nidx].c.cpu.XR;\
	_AAXIDX(tmp)
// DCP
#define DCP(x)\
	BYTE tmp = (x) - 1;\
	CMP(tmp, nes[nidx].c.cpu.AR)\
	_MSX(tmp)
// ISC
#define ISC(x)\
	BYTE tmp = (x) + 1;\
	_MSX(tmp)\
	_RSZ(_SUB(tmp), nes[nidx].c.cpu.AR)
// LAS
#define LAS\
	nes[nidx].c.cpu.SR &= nes[nidx].c.cpu.openbus;\
	_RSZ(nes[nidx].c.cpu.AR = nes[nidx].c.cpu.XR = nes[nidx].c.cpu.SR;, nes[nidx].c.cpu.AR)
// LAX
#define LAX(x)\
	x;\
	_LAX
// RLA, SLO, SRE
#define RLA(x) _SHF(x, _RLA(shift, 0x80, <<=), nes[nidx].c.cpu.AR, shift)
#define SLO(x) _SHF(x, _SLO(shift, 0x80, <<=, |=), nes[nidx].c.cpu.AR, shift)
#define SRE(x) _SHF(x, _SLO(shift, 0x01, >>=, ^=), nes[nidx].c.cpu.AR, shift)
// RRA
#define RRA(x)\
	BYTE shift = x;\
	_ROR(shift, 0x01, >>=)\
	_MSX(shift)\
	_RSZ(_ADD(shift), nes[nidx].c.cpu.AR)
// SXX
#define SXX(reg)\
	BYTE tmp = (reg) & ((adr2 >> 8) + 1);\
	if (adr1 != adr0) adr0 = (adr0 & (tmp << 8)) | (adr0 & 0x00FF);\
	_SXXABX(tmp)
// XAA
#define XAA\
	nes[nidx].c.cpu.AR = nes[nidx].c.cpu.XR;\
	nes[nidx].c.cpu.AR &= _RDP;
// XAS
#define XAS\
	nes[nidx].c.cpu.SR = nes[nidx].c.cpu.AR & nes[nidx].c.cpu.XR;\
    SXX(nes[nidx].c.cpu.SR)

// ---------------------------------------------------------------------------------
//  flags
// ---------------------------------------------------------------------------------
#define SF(x) nes[nidx].c.cpu.sf = (x) & 0x80;
#define ZF(x) nes[nidx].c.cpu.zf = !(x) << 1;
#define SZ(x)\
	SF(x)\
	ZF(x)

// ----------------------------------------------------------------------
//  varie ed eventuali
// ----------------------------------------------------------------------
#define _ADC _RSZ(_ADD(nes[nidx].c.cpu.openbus), nes[nidx].c.cpu.AR)
// NOTE : BCD Addiction
// 6502
// A Seq. 1
// C Seq. 1
// N Seq. 2
// V Seq. 2
// Z bin
// Seq. 1:
// 1a. AL = (A & $0F) + (B & $0F) + C
// 1b. If AL >= $0A, then AL = ((AL + $06) & $0F) + $10
// 1c. A = (A & $F0) + (B & $F0) + AL
// 1d. Note that A can be >= $100 at this point
// 1e. If (A >= $A0), then A = A + $60
// 1f. The accumulator result is the lower 8 bits of A
// 1g. The carry result is 1 if A >= $100, and is 0 if A < $100
// Seq. 2:
// 2a. AL = (A & $0F) + (B & $0F) + C
// 2b. If AL >= $0A, then AL = ((AL + $06) & $0F) + $10
// 2c. A = (A & $F0) + (B & $F0) + AL, using signed (twos complement) arithmetic
// 2e. The N flag result is 1 if bit 7 of A is 1, and is 0 if bit 7 if A is 0
// 2f. The V flag result is 1 if A < -128 or A > 127, and is 0 if -128 <= A <= 127
// Importante!!! Nel NES il decimal mode non e' supportato ed inoltre
// il codice per il decimal mode potrebbe essere buggato!!!!!!
#define _ADD(opr)\
	{\
	WORD A = 0;\
	if (info.decimal_mode && nes[nidx].c.cpu.df) {\
		BYTE AL = (nes[nidx].c.cpu.AR & 0x0F) + ((opr) & 0x0F) + nes[nidx].c.cpu.cf;\
		if (AL >= 0x0A) { AL = ((AL + 0x06) & 0x0F) + 0x10; }\
		A = (nes[nidx].c.cpu.AR & 0xF0) + ((opr) & 0xF0) + AL;\
		if (A >= 0xA0) { A += 0x60; }\
	} else { A = nes[nidx].c.cpu.AR + (opr) + nes[nidx].c.cpu.cf; }\
	nes[nidx].c.cpu.cf = (A > 0xFF ? 1 : 0);\
	nes[nidx].c.cpu.of = ((!((nes[nidx].c.cpu.AR ^ (opr)) & 0x80) && ((nes[nidx].c.cpu.AR ^ A) & 0x80)) ? 0x40 : 0);\
	nes[nidx].c.cpu.AR = (BYTE)A;\
	}
#define _BSH(dst, bitmask, opr)\
	nes[nidx].c.cpu.cf = ((dst) & (bitmask) ? 1 : 0);\
	dst opr 1;
#define _CMP(x, reg)\
	WORD cmp = (reg) - (x);\
	nes[nidx].c.cpu.cf = (cmp < 0x100 ? 1 : 0);
#define _CYW(cmd) _CY_(cmd, mod_cycles_op(+=, 1);)
#define _CY_(cmd1, cmd2)\
	if (adr1 != adr0) {\
		nes[nidx].c.cpu.double_rd = TRUE;\
		cmd2\
		_RD0;\
	}\
	cmd1
#define _DMC\
	DMC.tick_type = DMC_CPU_WRITE;\
	if (adr0 == 0x4014) {\
		DMC.tick_type = DMC_R4014;\
	}\
	tick_hw(nidx, 1);
#define _LAX\
	nes[nidx].c.cpu.AR = _RDB;\
	_RSZ(nes[nidx].c.cpu.XR = nes[nidx].c.cpu.AR;, nes[nidx].c.cpu.XR)
#define _MSZ(cmd, result1, result2)\
	_RSZ(cmd;, result1)\
	_MSX(result2)
#define _MSX(result)\
	_ASLWR1(_RDB)\
	nes[nidx].c.cpu.double_wr = TRUE;\
	_ASLWR2(result)\
	nes[nidx].c.cpu.double_wr = FALSE;
#define _PUL _RDX(((++nes[nidx].c.cpu.SP) | STACK), TRUE)
#define _PSH(src) _WRX((nes[nidx].c.cpu.SP--) | STACK, src)
#define _PSP\
	_PSH(nes[nidx].c.cpu.PC.b[1]);\
	_PSH(nes[nidx].c.cpu.PC.b[0]);
#define _RD0 _RDX(adr0, TRUE)
#define _RD1 _RDX(adr1, TRUE)
#define _RDB nes[nidx].c.cpu.openbus
#define _RDP _RDX(nes[nidx].c.cpu.PC.w++, TRUE)
#define _RDD _RDX(nes[nidx].c.cpu.PC.w, TRUE)
#define _RDX(src, LASTTICKHW) cpu_rd_mem(nidx, src, LASTTICKHW)
#define _RLA(dst, bitmask, opr)\
	_ROX(dst, bitmask, opr, old_cf);\
	nes[nidx].c.cpu.AR &= shift
#define _ROL(dst, bitmask, opr)\
	_ROX(dst, bitmask, opr, old_cf)
#define _ROR(dst, bitmask, opr)\
	_ROX(dst, bitmask, opr, (old_cf << 7))
#define _ROX(dst, bitmask, opr, oprnd)\
	{\
	BYTE old_cf = nes[nidx].c.cpu.cf;\
	_BSH(dst, bitmask, opr)\
	(dst) |= (oprnd);\
	}
#define _RSZ(cmd, result)\
	cmd\
	SZ(result)
#define _RDZ(cmd, result)\
    _RDD;\
	_RSZ(cmd, result)
#define _SBC _RSZ(_SUB(nes[nidx].c.cpu.openbus), nes[nidx].c.cpu.AR)
#define _SHF(x, cmd, result1, result2)\
	{\
	BYTE shift = x;\
	_MSZ(cmd, result1, result2)\
	}
#define _SLO(dst, bitmask, opr1, opr2)\
	_BSH(dst, bitmask, opr1)\
	nes[nidx].c.cpu.AR opr2 shift

// NOTE : BCD Subtraction
// 6502
// A Seq. 3
// C bin
// N bin
// V bin
// Z bin
// Seq. 3:
// 3a. AL = (A & $0F) - (B & $0F) + C-1
// 3b. If AL < 0, then AL = ((AL - $06) & $0F) - $10
// 3c. A = (A & $F0) - (B & $F0) + AL
// 3d. If A < 0, then A = A - $60
// 3e. The accumulator result is the lower 8 bits of A
// Importante!!! Nel NES il decimal mode non e' supportato ed inoltre
// il codice per il decimal mode potrebbe essere buggato!!!!!!
#define _SUB(opr)\
	{\
	if (info.decimal_mode && nes[nidx].c.cpu.df) {\
		SWORD A = 0; SWORD AL = (nes[nidx].c.cpu.AR & 0x0F) - ((opr) & 0x0F) + nes[nidx].c.cpu.cf - 1;\
		AL = (nes[nidx].c.cpu.AR & 0x0F) - ((opr) & 0x0F) + nes[nidx].c.cpu.cf - 1;\
		if (AL < 0) { AL = ((AL - 0x06) & 0x0F) - 0x10; }\
		A = (nes[nidx].c.cpu.AR & 0xF0) - ((opr) & 0xF0) + AL;\
		if (A < 0) { A -= 0x60; }\
		nes[nidx].c.cpu.cf = (A < 0x100 ? 1 : 0);\
		nes[nidx].c.cpu.of = (((nes[nidx].c.cpu.AR ^ (opr)) & 0x80) & ((nes[nidx].c.cpu.AR ^ A) & 0x80) ? 0x40 : 0);\
		nes[nidx].c.cpu.AR = (BYTE)A;\
	} else {\
		WORD A = nes[nidx].c.cpu.AR - (opr) - !nes[nidx].c.cpu.cf;\
		nes[nidx].c.cpu.cf = (A < 0x100 ? 1 : 0);\
		nes[nidx].c.cpu.of = (((nes[nidx].c.cpu.AR ^ (opr)) & 0x80) & ((nes[nidx].c.cpu.AR ^ A) & 0x80) ? 0x40 : 0);\
		nes[nidx].c.cpu.AR = (BYTE)A;\
	}\
	}
#define _WR0(reg) _WRX(adr0, reg);
#define _WRX(dst, reg) cpu_wr_mem(nidx, dst, reg)

// IRQ
#define _IRQ(flags)\
	BYTE flagNMI = FALSE;\
	_PSP\
	if (nes[nidx].c.nmi.high) {\
		flagNMI = TRUE;\
	}\
	assemble_SR(nidx);\
	_PSH(flags);\
	nes[nidx].c.cpu.im = nes[nidx].c.irq.inhibit = 0x04;\
	if (flagNMI) {\
		nes[nidx].c.nmi.high = nes[nidx].c.nmi.delay = FALSE;\
		nes[nidx].c.cpu.PC.w = lend_word(nidx, INT_NMI, FALSE, TRUE);\
	} else {\
		nes[nidx].c.cpu.PC.w = lend_word(nidx, INT_IRQ, FALSE, TRUE);\
		if (nes[nidx].c.nmi.high) {\
			nes[nidx].c.nmi.delay = TRUE;\
		}\
	}
#define IRQ(flags) \
	_RDD;\
	_IRQ(flags)
#define NMI\
	nes[nidx].c.nmi.high = nes[nidx].c.nmi.delay = FALSE; \
	_RDD;\
	_PSP\
	assemble_SR(nidx);\
	_PSH(nes[nidx].c.cpu.SR & 0xEF);\
	nes[nidx].c.cpu.im = nes[nidx].c.irq.inhibit = 0x04;\
	nes[nidx].c.cpu.PC.w = lend_word(nidx, INT_NMI, FALSE, TRUE);

#define _RDZPG  _RD0
#define _RDZPX_ _RD1
#define _RDZPX  _RD0
#define _RDABS  _RD0
#define _RDABX_ _RD1
#define _RDABX  _RD0
#define _RDIDX_ _RD1
#define _RDIDX  _RD0
#define _RDIDY_ _RD1
#define _STXZPG(reg) _WR0(reg)
#define _STXZPX(reg) _WR0(reg)
#define _STXABS(reg) _WR0(reg)
#define _STXABX(reg) _WR0(reg)
#define _STXIDX(reg) _WR0(reg)
#define _STXIDY(reg) _WR0(reg)
#define _ASLWR1(reg) _WR0(reg)
#define _ASLWR2(reg) _WR0(reg)
#define _AAXIDX(reg) _WR0(reg)
#define _SXXABX(reg) _WR0(reg)

static const BYTE table_opcode_cycles[256] = {
/*    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F     */
/*0*/ 7, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6, /*0*/
/*1*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, /*1*/
/*2*/ 6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6, /*2*/
/*3*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, /*3*/
/*4*/ 6, 6, 0, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6, /*4*/
/*5*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, /*5*/
/*6*/ 6, 6, 0, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6, /*6*/
/*7*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, /*7*/
/*8*/ 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4, /*8*/
/*9*/ 2, 6, 0, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5, /*9*/
/*A*/ 2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4, /*A*/
/*B*/ 2, 5, 0, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4, /*B*/
/*C*/ 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6, /*C*/
/*D*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7, /*D*/
/*E*/ 2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6, /*E*/
/*F*/ 2, 5, 0, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7  /*F*/
/*    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F     */
};

void cpu_exe_op(BYTE nidx) {
	nes[nidx].c.cpu.opcode = FALSE;
	DMC.tick_type = DMC_NORMAL;
	nes[nidx].c.cpu.opcode_PC = nes[nidx].c.cpu.PC.w;

	// ------------------------------------------------
	//                   IRQ handler
	// ------------------------------------------------
	// IRQ supportati: Esterni, BRK, APU frame counter ed NMI.
	//
	// Note:
	// e' importante che dell'inhibit esamini solo
	// gli ultimi 4 bit perche' i primi 4 potrebbero
	// contenere il suo nuovo valore.
	if (nes[nidx].c.irq.high && !(nes[nidx].c.irq.inhibit & 0x04)) {
		// se l'IRQ viene raggiunto nell'ultimo ciclo
		// dell'istruzione precedente (nes[nidx].c.irq.before == 0)
		// devo eseguire l'istruzione successiva e solo
		// dopo avviarlo.
		if (!nes[nidx].c.irq.before || nes[nidx].c.irq.delay) {
			nes[nidx].c.irq.delay = FALSE;
		} else {
			nes[nidx].c.cpu.opcode = 0x200;
		}
	}
	if (nes[nidx].c.nmi.high) {
		nes[nidx].c.cpu.opcode = 0;
		// se l'NMI viene raggiunto nell'ultimo ciclo
		// dell'istruzione precedente (nes[nidx].c.nmi.before = 0)
		// oppure durante un BRK o un IRQ (dal quinto ciclo
		// in poi), devo eseguire l'istruzione successiva e
		// solo dopo avviarlo.
		if (!nes[nidx].c.nmi.before || nes[nidx].c.nmi.delay) {
			nes[nidx].c.nmi.delay = FALSE;
		} else {
			nes[nidx].c.cpu.opcode = 0x100;
		}
	}
	// se codeop e' valorizzato (NMI o IRQ) eseguo
	// un tick hardware altrimenti devo leggere la
	// prossima istruzione.
	if (nes[nidx].c.cpu.opcode & 0x300) {
		tick_hw(nidx, 1);
	} else {
		// memorizzo l'opcode attuale
		nes[nidx].c.cpu.opcode = _RDP;
	}

	// azzero le variabili che utilizzo per sapere
	// quando avviene il DMA del DMC durante l'istruzione.
	nes[nidx].c.cpu.opcode_cycle = DMC.dma_cycle = 0;
	// flag della doppia lettura di un registro
	nes[nidx].c.cpu.double_rd = FALSE;

	// End of vertical blanking, sometime in pre-render
	// scanline: Set NMI_occurred to false.
	// (fonte: http://wiki.nesdev.com/w/index.php/NMI)
	//
	// FIXME: non sono affatto sicuro di questa cosa.
	// Disabilito l'NMI quando viene settato (dal registro $2000)
	// nello stesso momento in cui il vblank viene disabilitato
	// (nes[nidx].p.ppu.frame_x = 0). In questo modo passo la rom di test
	// 07-nmi_on_timing.nes. Non ho trovato informazioni su quando
	// effettivamente questa situazione avvenga.
	if (nes[nidx].c.nmi.high && !nes[nidx].c.nmi.frame_x && (nes[nidx].p.ppu.frame_y == nes[nidx].p.ppu_sclines.vint)) {
		nes[nidx].c.nmi.high = nes[nidx].c.nmi.delay = FALSE;
	}
	// le istruzioni CLI, SEI, e PLP ritardano il
	// cambiamento dell'interrupt mask (im) fino
	// all'istruzione successiva, o meglio l'im
	// viene modificato immediatamente nello Status
	// Register (SR) ma l'effettiva funzione del
	// flag viene ritardata di un'istruzione.
	// L'RTI non soffre di questo difetto.
	if (nes[nidx].c.irq.inhibit != nes[nidx].c.cpu.im) {
		// l'IRQ handler ha gia' controllato la
		// presenza di IRQ con il vecchio valore
		// del flag di inibizione. Adesso, se
		// serve, posso impostare il nuovo.
		nes[nidx].c.irq.inhibit >>= 4;
	}
	// ------------------------------------------------

	// salvo i cicli presi dall'istruzione...
	nes[nidx].c.cpu.base_opcode_cycles = table_opcode_cycles[(BYTE)nes[nidx].c.cpu.opcode];
	mod_cycles_op(+=, nes[nidx].c.cpu.base_opcode_cycles);

	//Jam the cpu if necessary
	if (nes[nidx].c.cpu.jammed) {
		return;
	}

	// ... e la eseguo
	switch (nes[nidx].c.cpu.opcode) {

	case 0x69: IMP(RD_OP, ADC(_RDP)) break;                                                      // ADC #IMM
	case 0x65: ZPG(RD_OP, ADC(_RDZPG)) break;                                                    // ADC $ZPG
	case 0x75: ZPX(RD_OP, ADC(_RDZPX), nes[nidx].c.cpu.XR) break;                                // ADC $ZPG,X
	case 0x6D: ABS(RD_OP, ADC(_RDABS)) break;                                                    // ADC $ABS
	case 0x7D: ABX(RD_OP, _CYW(_ADC), nes[nidx].c.cpu.XR) break;                                 // ADC $ABS,X
	case 0x79: ABX(RD_OP, _CYW(_ADC), nes[nidx].c.cpu.YR) break;                                 // ADC $ABS,Y
	case 0x61: IDX(RD_OP, ADC(_RDIDX)) break;                                                    // ADC ($IND,X)
	case 0x71: IDY(RD_OP, _CYW(_ADC)) break;                                                     // ADC ($IND),Y

	case 0x29: IMP(RD_OP, AND(_RDP, &=)) break;                                                  // AND #IMM
	case 0x25: ZPG(RD_OP, AND(_RDZPG, &=)) break;                                                // AND $ZPG
	case 0x35: ZPX(RD_OP, AND(_RDZPX, &=), nes[nidx].c.cpu.XR) break;                            // AND $ZPG,X
	case 0x2D: ABS(RD_OP, AND(_RDABS, &=)) break;                                                // AND $ABS
	case 0x3D: ABX(RD_OP, _CYW(AND(_RDB, &=)), nes[nidx].c.cpu.XR) break;                        // AND $ABS,X
	case 0x39: ABX(RD_OP, _CYW(AND(_RDB, &=)), nes[nidx].c.cpu.YR) break;                        // AND $ABS,Y
	case 0x21: IDX(RD_OP, AND(_RDIDX, &=)) break;                                                // AND ($IND,X)
	case 0x31: IDY(RD_OP, _CYW(AND(_RDB, &=))) break;                                            // AND ($IND),Y

	case 0x0A: IMP(RD_OP, _RDZ(_BSH(nes[nidx].c.cpu.AR, 0x80, <<=), nes[nidx].c.cpu.AR)) break;  // ASL [AR]
	case 0x06: ZPG(WR_OP, ASL(_RDZPG)) break;                                                    // ASL $ZPG
	case 0x16: ZPX(WR_OP, ASL(_RDZPX), nes[nidx].c.cpu.XR) break;                                // ASL $ZPG,X
	case 0x0E: ABS(WR_OP, ASL(_RDABS)) break;                                                    // ASL $ABS
	case 0x1E: ABX(WR_OP, ASL(_RDABX), nes[nidx].c.cpu.XR) break;                                // ASL $ABS,X

	case 0x90: IMP(RD_OP, BRC(nes[nidx].c.cpu.cf, FALSE)) break;                                 // BCC [C = 0]
	case 0xB0: IMP(RD_OP, BRC(nes[nidx].c.cpu.cf, TRUE)) break;                                  // BCS [C = 1]
	case 0xF0: IMP(RD_OP, BRC(nes[nidx].c.cpu.zf, TRUE)) break;                                  // BEQ [Z = 1]
	case 0x30: IMP(RD_OP, BRC(nes[nidx].c.cpu.sf, TRUE)) break;                                  // BMI [S = 1]
	case 0xD0: IMP(RD_OP, BRC(nes[nidx].c.cpu.zf, FALSE)) break;                                 // BNE [Z = 0]
	case 0x10: IMP(RD_OP, BRC(nes[nidx].c.cpu.sf, FALSE)) break;                                 // BPL [S = 0]
	case 0x50: IMP(RD_OP, BRC(nes[nidx].c.cpu.of, FALSE)) break;                                 // BVC [O = 0]
	case 0x70: IMP(RD_OP, BRC(nes[nidx].c.cpu.of, TRUE)) break;                                  // BVS [O = 1]

	case 0x24: ZPG(RD_OP, BIT(_RDZPG)) break;                                                    // BIT $ZPG
	case 0x2C: ABS(RD_OP, BIT(_RDABS)) break;                                                    // BIT $ABS

	case 0x00: IMP(RD_OP, BRK) break;                                                            // BRK

	case 0x18: IMP(RD_OP, _RDD; nes[nidx].c.cpu.cf = FALSE;) break;                              // CLC [C -> 0]
	case 0xD8: IMP(RD_OP, _RDD; nes[nidx].c.cpu.df = FALSE;) break;                              // CLD [D -> 0]
	case 0x58: IMP(RD_OP, _RDD; nes[nidx].c.cpu.im = FALSE;) break;                              // CLI [I -> 0]
	case 0xB8: IMP(RD_OP, _RDD; nes[nidx].c.cpu.of = FALSE;) break;                              // CLV [O -> 0]

	case 0xC9: IMP(RD_OP, CMP(_RDP, nes[nidx].c.cpu.AR)) break;                                  // CMP #IMM
	case 0xC5: ZPG(RD_OP, CMP(_RDZPG, nes[nidx].c.cpu.AR)) break;                                // CMP $ZPG
	case 0xD5: ZPX(RD_OP, CMP(_RDZPX, nes[nidx].c.cpu.AR), nes[nidx].c.cpu.XR) break;            // CMP $ZPG,X
	case 0xCD: ABS(RD_OP, CMP(_RDABS, nes[nidx].c.cpu.AR)) break;                                // CMP $ABS
	case 0xDD: ABX(RD_OP, _CYW(CMP(_RDB, nes[nidx].c.cpu.AR)), nes[nidx].c.cpu.XR) break;        // CMP $ABS,X
	case 0xD9: ABX(RD_OP, _CYW(CMP(_RDB, nes[nidx].c.cpu.AR)), nes[nidx].c.cpu.YR) break;        // CMP $ABS,Y
	case 0xC1: IDX(RD_OP, CMP(_RDIDX, nes[nidx].c.cpu.AR)) break;                                // CMP ($IND,X)
	case 0xD1: IDY(RD_OP, _CYW(CMP(_RDB, nes[nidx].c.cpu.AR))) break;                            // CMP ($IND),Y

	case 0xE0: IMP(RD_OP, CMP(_RDP, nes[nidx].c.cpu.XR)) break;                                  // CPX #IMM
	case 0xE4: ZPG(RD_OP, CMP(_RDZPG, nes[nidx].c.cpu.XR)) break;                                // CPX $ZPG
	case 0xEC: ABS(RD_OP, CMP(_RDABS, nes[nidx].c.cpu.XR)) break;                                // CPX $ABS

	case 0xC0: IMP(RD_OP, CMP(_RDP, nes[nidx].c.cpu.YR)) break;                                  // CPY #IMM
	case 0xC4: ZPG(RD_OP, CMP(_RDZPG, nes[nidx].c.cpu.YR)) break;                                // CPY $ZPG
	case 0xCC: ABS(RD_OP, CMP(_RDABS, nes[nidx].c.cpu.YR)) break;                                // CPY $ABS

	case 0xC6: ZPG(WR_OP, INC(_RDZPG, -)) break;                                                 // DEC $ZPG
	case 0xD6: ZPX(WR_OP, INC(_RDZPX, -), nes[nidx].c.cpu.XR) break;                             // DEC $ZPG,X
	case 0xCE: ABS(WR_OP, INC(_RDABS, -)) break;                                                 // DEC $ABS
	case 0xDE: ABX(WR_OP, INC(_RDABX, -), nes[nidx].c.cpu.XR) break;                             // DEC $ABS,X

	case 0xCA: IMP(RD_OP, _RDZ(nes[nidx].c.cpu.XR--;, nes[nidx].c.cpu.XR)) break;                // DEX [XR]

	case 0x88: IMP(RD_OP, _RDZ(nes[nidx].c.cpu.YR--;, nes[nidx].c.cpu.YR)) break;                // DEY [YR]

	case 0x49: IMP(RD_OP, AND(_RDP, ^=)) break;                                                  // EOR #IMM
	case 0x45: ZPG(RD_OP, AND(_RDZPG, ^=)) break;                                                // EOR $ZPG
	case 0x55: ZPX(RD_OP, AND(_RDZPX, ^=), nes[nidx].c.cpu.XR) break;                            // EOR $ZPG,X
	case 0x4D: ABS(RD_OP, AND(_RDABS, ^=)) break;                                                // EOR $ABS
	case 0x5D: ABX(RD_OP, _CYW(AND(_RDB, ^=)), nes[nidx].c.cpu.XR) break;                        // EOR $ABS,X
	case 0x59: ABX(RD_OP, _CYW(AND(_RDB, ^=)), nes[nidx].c.cpu.YR) break;                        // EOR $ABS,Y
	case 0x41: IDX(RD_OP, AND(_RDIDX, ^=)) break;                                                // EOR ($IND,X)
	case 0x51: IDY(RD_OP, _CYW(AND(_RDB, ^=))) break;                                            // EOR ($IND),Y

	case 0xE6: ZPG(WR_OP, INC(_RDZPG, +)) break;                                                 // INC $ZPG
	case 0xF6: ZPX(WR_OP, INC(_RDZPX, +), nes[nidx].c.cpu.XR) break;                             // INC $ZPG,X
	case 0xEE: ABS(WR_OP, INC(_RDABS, +)) break;                                                 // INC $ABS
	case 0xFE: ABX(WR_OP, INC(_RDABX, +), nes[nidx].c.cpu.XR) break;                             // INC $ABS,X

	case 0xE8: IMP(RD_OP, _RDZ(nes[nidx].c.cpu.XR++;, nes[nidx].c.cpu.XR)) break;                // INX [XR]

	case 0xC8: IMP(RD_OP, _RDZ(nes[nidx].c.cpu.YR++;, nes[nidx].c.cpu.YR)) break;                // INY [YR]

	case 0x4C: IMP(RD_OP, JMP) break;                                                            // JMP $ABS
	case 0x6C: IDR(RD_OP) break;                                                                 // JMP ($IND)

	case 0x20: IMP(RD_OP, JSR) break;                                                            // JSR $ABS

	case 0xA9: IMP(RD_OP, LDX(_RDP, nes[nidx].c.cpu.AR)) break;                                  // LDA #IMM
	case 0xA5: ZPG(RD_OP, LDX(_RDZPG, nes[nidx].c.cpu.AR)) break;                                // LDA $ZPG
	case 0xB5: ZPX(RD_OP, LDX(_RDZPX, nes[nidx].c.cpu.AR), nes[nidx].c.cpu.XR) break;            // LDA $ZPG,X
	case 0xAD: ABS(RD_OP, LDX(_RDABS, nes[nidx].c.cpu.AR)) break;                                // LDA $ABS
	case 0xBD: ABX(RD_OP, _CYW(LDX(_RDB, nes[nidx].c.cpu.AR)), nes[nidx].c.cpu.XR) break;        // LDA $ABS,X
	case 0xB9: ABX(RD_OP, _CYW(LDX(_RDB, nes[nidx].c.cpu.AR)), nes[nidx].c.cpu.YR) break;        // LDA $ABS,Y
	case 0xA1: IDX(RD_OP, LDX(_RDIDX, nes[nidx].c.cpu.AR)) break;                                // LDA ($IND,X)
	case 0xB1: IDY(RD_OP, _CYW(LDX(_RDB, nes[nidx].c.cpu.AR))) break;                            // LDA ($IND),Y

	case 0xA2: IMP(RD_OP, LDX(_RDP, nes[nidx].c.cpu.XR)) break;                                  // LDX #IMM
	case 0xA6: ZPG(RD_OP, LDX(_RDZPG,nes[nidx].c.cpu.XR)) break;                                 // LDX $ZPG
	case 0xB6: ZPX(RD_OP, LDX(_RDZPX,nes[nidx].c.cpu.XR), nes[nidx].c.cpu.YR) break;             // LDX $ZPG,Y
	case 0xAE: ABS(RD_OP, LDX(_RDABS,nes[nidx].c.cpu.XR)) break;                                 // LDX $ABS
	case 0xBE: ABX(RD_OP, _CYW(LDX(_RDB, nes[nidx].c.cpu.XR)), nes[nidx].c.cpu.YR) break;        // LDX $ABS,Y

	case 0xA0: IMP(RD_OP, LDX(_RDP, nes[nidx].c.cpu.YR)) break;                                  // LDY #IMM
	case 0xA4: ZPG(RD_OP, LDX(_RDZPG, nes[nidx].c.cpu.YR)) break;                                // LDY $ZPG
	case 0xB4: ZPX(RD_OP, LDX(_RDZPX, nes[nidx].c.cpu.YR), nes[nidx].c.cpu.XR) break;            // LDY $ZPG,X
	case 0xAC: ABS(RD_OP, LDX(_RDABS, nes[nidx].c.cpu.YR)) break;                                // LDY $ABS
	case 0xBC: ABX(RD_OP, _CYW(LDX(_RDB, nes[nidx].c.cpu.YR)), nes[nidx].c.cpu.XR) break;        // LDY $ABS,X

	case 0x4A: IMP(RD_OP, _RDZ(_BSH(nes[nidx].c.cpu.AR, 0x01, >>=), nes[nidx].c.cpu.AR)) break;  // LSR [AR]
	case 0x46: ZPG(WR_OP, LSR(_RDZPG)) break;                                                    // LSR $ZPG
	case 0x56: ZPX(WR_OP, LSR(_RDZPX), nes[nidx].c.cpu.XR) break;                                // LSR $ZPG,X
	case 0x4E: ABS(WR_OP, LSR(_RDABS)) break;                                                    // LSR $ABS
	case 0x5E: ABX(WR_OP, LSR(_RDABX), nes[nidx].c.cpu.XR) break;                                // LSR $ABS,X

	case 0xEA: IMP(RD_OP, _RDD;) break;                                                          // NOP

	case 0x09: IMP(RD_OP, AND(_RDP, |=)) break;                                                  // ORA #IMM
	case 0x05: ZPG(RD_OP, AND(_RDZPG, |=)) break;                                                // ORA $ZPG
	case 0x15: ZPX(RD_OP, AND(_RDZPX, |=), nes[nidx].c.cpu.XR) break;                            // ORA $ZPG,X
	case 0x0D: ABS(RD_OP, AND(_RDABS, |=)) break;                                                // ORA $ABS
	case 0x1D: ABX(RD_OP, _CYW(AND(_RDB, |=)), nes[nidx].c.cpu.XR) break;                        // ORA $ABS,X
	case 0x19: ABX(RD_OP, _CYW(AND(_RDB, |=)), nes[nidx].c.cpu.YR) break;                        // ORA $ABS,Y
	case 0x01: IDX(RD_OP, AND(_RDIDX, |=)) break;                                                // ORA ($IND,X)
	case 0x11: IDY(RD_OP, _CYW(AND(_RDB, |=))) break;                                            // ORA ($IND),Y

	case 0x48: IMP(WR_OP, PHA) break;                                                            // PHA
	case 0x08: IMP(WR_OP, PHP) break;                                                            // PHP

	case 0x68: IMP(WR_OP, PLA) break;                                                            // PLA
	case 0x28: IMP(WR_OP, PLP) break;                                                            // PLP

	case 0x2A: IMP(RD_OP, _RDZ(_ROL(nes[nidx].c.cpu.AR, 0x80, <<=), nes[nidx].c.cpu.AR)) break;  // ROL [AR]
	case 0x26: ZPG(WR_OP, ROL(_RDZPG)) break;                                                    // ROL $ZPG
	case 0x36: ZPX(WR_OP, ROL(_RDZPX), nes[nidx].c.cpu.XR) break;                                // ROL $ZPG,X
	case 0x2E: ABS(WR_OP, ROL(_RDABS)) break;                                                    // ROL $ABS
	case 0x3E: ABX(WR_OP, ROL(_RDABX), nes[nidx].c.cpu.XR) break;                                // ROL $ABS,X

	case 0x6A: IMP(RD_OP, _RDZ(_ROR(nes[nidx].c.cpu.AR, 0x01, >>=), nes[nidx].c.cpu.AR)) break;  // ROR [AR]
	case 0x66: ZPG(WR_OP, ROR(_RDZPG)) break;                                                    // ROR $ZPG
	case 0x76: ZPX(WR_OP, ROR(_RDZPX), nes[nidx].c.cpu.XR) break;                                // ROR $ZPG,X
	case 0x6E: ABS(WR_OP, ROR(_RDABS)) break;                                                    // ROR $ABS
	case 0x7E: ABX(WR_OP, ROR(_RDABX), nes[nidx].c.cpu.XR) break;                                // ROR $ABS,X

	case 0x40: IMP(RD_OP, RTI) break;                                                            // RTI

	case 0x60: IMP(RD_OP, RTS) break;                                                            // RTS

	case 0xE9: IMP(RD_OP, SBC(_RDP)) break;                                                      // SBC #IMM
	case 0xE5: ZPG(RD_OP, SBC(_RDZPG)) break;                                                    // SBC $ZPG
	case 0xF5: ZPX(RD_OP, SBC(_RDZPX), nes[nidx].c.cpu.XR) break;                                // SBC $ZPG,X
	case 0xED: ABS(RD_OP, SBC(_RDABS)) break;                                                    // SBC $ABS
	case 0xFD: ABX(RD_OP, _CYW(_SBC), nes[nidx].c.cpu.XR) break;                                 // SBC $ABS,X
	case 0xF9: ABX(RD_OP, _CYW(_SBC), nes[nidx].c.cpu.YR) break;                                 // SBC $ABS,Y
	case 0xE1: IDX(RD_OP, SBC(_RDIDX)) break;                                                    // SBC ($IND,X)
	case 0xF1: IDY(RD_OP, _CYW(_SBC)) break;                                                     // SBC ($IND),Y

	case 0x38: IMP(RD_OP, _RDD; nes[nidx].c.cpu.cf = 0x01;) break;                               // SEC [C -> 1]
	case 0xF8: IMP(RD_OP, _RDD; nes[nidx].c.cpu.df = 0x08;) break;                               // SED [D -> 1]
	case 0x78: IMP(RD_OP, _RDD; SEI) break;                                                      // SEI [I -> 1]

	case 0x85: ZPG(WR_OP, _STXZPG(nes[nidx].c.cpu.AR)) break;                                    // STA $ZPG
	case 0x95: ZPX(WR_OP, _STXZPX(nes[nidx].c.cpu.AR), nes[nidx].c.cpu.XR) break;                // STA $ZPG,X
	case 0x8D: ABW(WR_OP, _STXABS(nes[nidx].c.cpu.AR)) break;                                    // STA $ABS
	case 0x9D: ABX(WR_OP, _STXABX(nes[nidx].c.cpu.AR), nes[nidx].c.cpu.XR) break;                // STA $ABS,X
	case 0x99: ABX(WR_OP, _STXABX(nes[nidx].c.cpu.AR), nes[nidx].c.cpu.YR) break;                // STA $ABS,Y
	case 0x81: IDX(WR_OP, _STXIDX(nes[nidx].c.cpu.AR)) break;                                    // STA ($IND,X)
	case 0x91: IDY(WR_OP, _STXIDY(nes[nidx].c.cpu.AR)) break;                                    // STA ($IND),Y

	case 0x86: ZPG(WR_OP, _STXZPG(nes[nidx].c.cpu.XR)) break;                                    // STX $ZPG
	case 0x96: ZPX(WR_OP, _STXZPX(nes[nidx].c.cpu.XR), nes[nidx].c.cpu.YR) break;                // STX $ZPG,Y
	case 0x8E: ABW(WR_OP, _STXABS(nes[nidx].c.cpu.XR)) break;                                    // STX $ABS

	case 0x84: ZPG(WR_OP, _STXZPG(nes[nidx].c.cpu.YR)) break;                                    // STY $ZPG
	case 0x94: ZPX(WR_OP, _STXZPX(nes[nidx].c.cpu.YR), nes[nidx].c.cpu.XR) break;                // STY $ZPG,X
	case 0x8C: ABW(WR_OP, _STXABS(nes[nidx].c.cpu.YR)) break;                                    // STY $ABS

	case 0xAA: IMP(RD_OP, _RDZ(nes[nidx].c.cpu.XR = nes[nidx].c.cpu.AR;, nes[nidx].c.cpu.XR)) break; // TAX
	case 0xA8: IMP(RD_OP, _RDZ(nes[nidx].c.cpu.YR = nes[nidx].c.cpu.AR;, nes[nidx].c.cpu.YR)) break; // TAY
	case 0xBA: IMP(RD_OP, _RDZ(nes[nidx].c.cpu.XR = nes[nidx].c.cpu.SP;, nes[nidx].c.cpu.XR)) break; // TSX
	case 0x8A: IMP(RD_OP, _RDZ(nes[nidx].c.cpu.AR = nes[nidx].c.cpu.XR;, nes[nidx].c.cpu.AR)) break; // TXA
	case 0x9A: IMP(RD_OP, _RDD; nes[nidx].c.cpu.SP = nes[nidx].c.cpu.XR;) break;                     // TXS
	case 0x98: IMP(RD_OP, _RDZ(nes[nidx].c.cpu.AR = nes[nidx].c.cpu.YR;, nes[nidx].c.cpu.AR)) break; // TYA

	// illegal opcodes
	case 0x0B:                                                                                   // AAC #IMM
	case 0x2B: IMP(RD_OP, AAC) break;                                                            // AAC #IMM

	case 0x87: ZPG(WR_OP, AAX) break;                                                            // AAX $ZPG
	case 0x97: ZPX(WR_OP, AAX, nes[nidx].c.cpu.YR) break;                                        // AAX $ZPG,Y
	case 0x8F: ABS(WR_OP, AAX) break;                                                            // AAX $ABS
	case 0x83: IDX(WR_OP, AAX) break;                                                            // AAX ($IND,X)

	case 0x6B: IMP(RD_OP, ARR) break;                                                            // ARR #IMM

	case 0x4B: IMP(RD_OP, ASR) break;                                                            // ASR #IMM

	case 0xAB: IMP(RD_OP, ATX) break;                                                            // ATX #IMM

	case 0xCB: IMP(RD_OP, AXS) break;                                                            // AXS #IMM

	case 0xC7: ZPG(WR_OP, DCP(_RDZPG)) break;                                                    // DCP $ZPG
	case 0xD7: ZPX(WR_OP, DCP(_RDZPX), nes[nidx].c.cpu.XR) break;                                // DCP $ZPG,X
	case 0xCF: ABS(WR_OP, DCP(_RDABS)) break;                                                    // DCP $ABS
	case 0xDF: ABX(WR_OP, DCP(_RDABX), nes[nidx].c.cpu.XR) break;                                // DCP $ABS,X
	case 0xDB: ABX(WR_OP, DCP(_RDABX), nes[nidx].c.cpu.YR) break;                                // DCP $ABS,Y
	case 0xC3: IDX(WR_OP, DCP(_RDIDX)) break;                                                    // DCP ($IND,X)
	case 0xD3: IDY(WR_OP, DCP(_RDABX)) break;                                                    // DCP ($IND),Y

	case 0x80:                                                                                   // DOP #IMM
	case 0x82:                                                                                   // DOP #IMM
	case 0x89:                                                                                   // DOP #IMM
	case 0XC2:                                                                                   // DOP #IMM
	case 0XE2: IMP(RD_OP, nes[nidx].c.cpu.PC.w++; _RDD;) break;                                  // DOP #IMM
	case 0x04:                                                                                   // DOP $ZPG
	case 0x44:                                                                                   // DOP $ZPG
	case 0x64: ZPG(RD_OP, _RD0;) break;                                                          // DOP $ZPG
	case 0x14:                                                                                   // DOP $ZPG,X
	case 0x34:                                                                                   // DOP $ZPG,X
	case 0x54:                                                                                   // DOP $ZPG,X
	case 0x74:                                                                                   // DOP $ZPG,X
	case 0xD4:                                                                                   // DOP $ZPG,X
	case 0xF4: ZPX(RD_OP, _RD0;, nes[nidx].c.cpu.XR) break;                                      // DOP $ZPG,X

	case 0xE7: ZPG(WR_OP, ISC(_RDZPG)) break;                                                    // ISC $ZPG
	case 0xF7: ZPX(WR_OP, ISC(_RDZPX), nes[nidx].c.cpu.XR) break;                                // ISC $ZPG,X
	case 0xEF: ABS(WR_OP, ISC(_RDABS)) break;                                                    // ISC $ABS
	case 0xFF: ABX(WR_OP, ISC(_RDABX), nes[nidx].c.cpu.XR) break;                                // ISC $ABS,X
	case 0xFB: ABX(WR_OP, ISC(_RDABX), nes[nidx].c.cpu.YR) break;                                // ISC $ABS,Y
	case 0xE3: IDX(WR_OP, ISC(_RDIDX)) break;                                                    // ISC ($IND,X)
	case 0xF3: IDY(WR_OP, _CY_(ISC(_RDB),)) break;                                               // ISC ($IND),Y

	case 0xA7: ZPG(RD_OP, LAX(_RDZPG)) break;                                                    // LAX $ZPG
	case 0xB7: ZPX(RD_OP, LAX(_RDABX), nes[nidx].c.cpu.YR) break;                                // LAX $ZPG,Y
	case 0xAF: ABS(RD_OP, LAX(_RDABS)) break;                                                    // LAX $ABS
	case 0xBF: ABX(RD_OP, _CYW(_LAX), nes[nidx].c.cpu.YR) break;                                 // LAX $ABS,Y
	case 0xA3: IDX(RD_OP, LAX(_RDIDX)) break;                                                    // LAX ($IND,X)
	case 0xB3: IDY(RD_OP, _CYW(_LAX)) break;                                                     // LAX ($IND),Y

	case 0x1A:                                                                                   // NOP
	case 0x3A:                                                                                   // NOP
	case 0x5A:                                                                                   // NOP
	case 0x7A:                                                                                   // NOP
	case 0xDA:                                                                                   // NOP
	case 0xFA: IMP(RD_OP, _RDD;) break;                                                          // NOP

	case 0x27: ZPG(WR_OP, RLA(_RDZPG)) break;                                                    // RLA $ZPG
	case 0x37: ZPX(WR_OP, RLA(_RDZPX), nes[nidx].c.cpu.XR) break;                                // RLA $ZPG,X
	case 0x2F: ABS(WR_OP, RLA(_RDABS)) break;                                                    // RLA $ABS
	case 0x3F: ABX(WR_OP, RLA(_RDABX), nes[nidx].c.cpu.XR) break;                                // RLA $ABS,X
	case 0x3B: ABX(WR_OP, RLA(_RDABX), nes[nidx].c.cpu.YR) break;                                // RLA $ABS,Y
	case 0x23: IDX(WR_OP, RLA(_RDIDX)) break;                                                    // RLA ($IND,X)
	case 0x33: IDY(WR_OP, _CY_(RLA(_RDB),)) break;                                               // RLA ($IND),Y

	case 0x67: ZPG(WR_OP, RRA(_RDZPG)) break;                                                    // RRA $ZPG
	case 0x77: ZPX(WR_OP, RRA(_RDZPX), nes[nidx].c.cpu.XR) break;                                // RRA $ZPG,X
	case 0x6F: ABS(WR_OP, RRA(_RDABS)) break;                                                    // RRA $ABS
	case 0x7F: ABX(WR_OP, RRA(_RDABX), nes[nidx].c.cpu.XR) break;                                // RRA $ABS,X
	case 0x7B: ABX(WR_OP, RRA(_RDABX), nes[nidx].c.cpu.YR) break;                                // RRA $ABS,Y
	case 0x63: IDX(WR_OP, RRA(_RDIDX)) break;                                                    // RRA ($IND,X)
	case 0x73: IDY(WR_OP, _CY_(RRA(_RDB),)) break;                                               // RRA ($IND),Y

	case 0xEB: IMP(RD_OP, SBC(_RDP)) break;                                                      // SBC #IMM

	case 0x07: ZPG(WR_OP, SLO(_RDZPG)) break;                                                    // SLO $ZPG
	case 0x17: ZPX(WR_OP, SLO(_RDZPX), nes[nidx].c.cpu.XR) break;                                // SLO $ZPG,X
	case 0x0F: ABS(WR_OP, SLO(_RDABS)) break;                                                    // SLO $ABS
	case 0x1F: ABX(WR_OP, SLO(_RDABX), nes[nidx].c.cpu.XR) break;                                // SLO $ABS,X
	case 0x1B: ABX(WR_OP, SLO(_RDABX), nes[nidx].c.cpu.YR) break;                                // SLO $ABS,Y
	case 0x03: IDX(WR_OP, SLO(_RDIDX)) break;                                                    // SLO ($IND,X)
	case 0x13: IDY(WR_OP, _CY_(SLO(_RDB),)) break;                                               // SLO ($IND),Y

	case 0x47: ZPG(WR_OP, SRE(_RDZPG)) break;                                                    // SRE $ZPG
	case 0x57: ZPX(WR_OP, SRE(_RDZPX), nes[nidx].c.cpu.XR) break;                                // SRE $ZPG,X
	case 0x4F: ABS(WR_OP, SRE(_RDABS)) break;                                                    // SRE $ABS
	case 0x5F: ABX(WR_OP, SRE(_RDABX), nes[nidx].c.cpu.XR) break;                                // SRE $ABS,X
	case 0x5B: ABX(WR_OP, SRE(_RDABX), nes[nidx].c.cpu.YR) break;                                // SRE $ABS,Y
	case 0x43: IDX(WR_OP, SRE(_RDIDX)) break;                                                    // SRE ($IND,X)
	case 0x53: IDY(WR_OP, _CY_(SRE(_RDB),)) break;                                               // SRE ($IND),Y

	case 0x0C: ABS(RD_OP, _RD0;) break;                                                          // TOP $ABS
	case 0x1C:                                                                                   // TOP $ABS,X
	case 0x3C:                                                                                   // TOP $ABS,X
	case 0X5C:                                                                                   // TOP $ABS,X
	case 0X7C:                                                                                   // TOP $ABS,X
	case 0XDC:                                                                                   // TOP $ABS,X
	case 0XFC: ABX(RD_OP, _CYW(), nes[nidx].c.cpu.XR) break;                                     // TOP $ABS,X

	case 0x9C: ABX(WR_OP, SXX(nes[nidx].c.cpu.YR), nes[nidx].c.cpu.XR) break;                    // SYA $ABS,X
	case 0x9E: ABX(WR_OP, SXX(nes[nidx].c.cpu.XR), nes[nidx].c.cpu.YR) break;                    // SXA $ABS,Y

	// casi incerti
	case 0x8B: IMP(RD_OP, XAA) break;                                                            // XAA #IMM
	case 0x9F: ABX(WR_OP, SXX(nes[nidx].c.cpu.AR & nes[nidx].c.cpu.XR), nes[nidx].c.cpu.YR) break; // AXA $ABS,Y
	case 0x93: IDY(WR_OP, SXX(nes[nidx].c.cpu.AR & nes[nidx].c.cpu.XR)) break;                   // AXA ($IND),Y
	case 0xBB: ABX(RD_OP, _CYW(LAS), nes[nidx].c.cpu.YR) break;                                  // LAS $ABS,Y
	case 0x9B: ABX(WR_OP, XAS, nes[nidx].c.cpu.YR) break;                                        // XAS $ABS,Y

	//KIL/HLT instructions. Set the cpu to be jammed
	case 0x02:
	case 0x12:
	case 0x22:
	case 0x32:
	case 0x42:
	case 0x52:
	case 0x62:
	case 0x72:
	case 0x92:
	case 0xB2:
	case 0xD2:
	case 0xF2:
	default:
		if (!info.no_rom && !info.first_illegal_opcode) {
			log_warning(uL("cpu;alert PC = 0x%04X, CODEOP = 0x%02X"), (nes[nidx].c.cpu.PC.w - 1), nes[nidx].c.cpu.opcode);
			gui_overlay_info_append_msg_precompiled(4, NULL);
			info.first_illegal_opcode = TRUE;
		}
		nes[nidx].c.cpu.jammed = 1;
		nes[nidx].c.cpu.cycles = 0;
		break;
	case OP_NMI: IMP(RD_OP, NMI) break;                                                          // NMI
	case OP_IRQ: IMP(RD_OP, IRQ(nes[nidx].c.cpu.SR & 0xEF)) break;                               // IRQ
	}

	// se presenti eseguo i restanti cicli di PPU e APU
	if (nes[nidx].c.cpu.cycles > 0) {
		tick_hw(nidx, nes[nidx].c.cpu.cycles);
	}
}
void cpu_initial_cycles(BYTE nidx) {
	cpu_rd_mem(nidx, nes[nidx].c.cpu.PC.w, TRUE);
	cpu_rd_mem(nidx, nes[nidx].c.cpu.PC.w, TRUE);
	cpu_rd_mem(nidx, nes[nidx].c.cpu.PC.w, TRUE);
	cpu_rd_mem(nidx, nes[nidx].c.cpu.SP-- | STACK, TRUE);
	cpu_rd_mem(nidx, nes[nidx].c.cpu.SP-- | STACK, TRUE);
	cpu_rd_mem(nidx, nes[nidx].c.cpu.SP-- | STACK, TRUE);
	nes[nidx].c.cpu.PC.b[0] = cpu_rd_mem(nidx, INT_RESET, TRUE);
	nes[nidx].c.cpu.PC.b[1] = cpu_rd_mem(nidx, INT_RESET + 1, TRUE);
	nes[nidx].c.cpu.cycles = 0;
	nes[nidx].c.cpu.opcode_cycle = 0;
	nes[nidx].c.cpu.double_wr = 0;
	nes[nidx].c.cpu.double_rd = 0;
}
void cpu_turn_on(BYTE nidx) {
	if (info.reset >= HARD) {
		memset(&nes[nidx].c.cpu, 0x00, sizeof(nes[nidx].c.cpu));
		// il bit 5 dell'SR e' sempre a 1 e
		// il bit 2 (inhibit maskable interrupt) e'
		// attivo all'avvio (o dopo un reset).
		nes[nidx].c.cpu.SR = 0x34;

		if (tas.type && (tas.emulator == FCEUX)) {
			unsigned int x = 0;

			for (x = 0; x < ram_size(nidx); x++) {
				ram_wr(nidx, x, (x & 0x04) ? 0xFF : 0x00);
			}
		} else {
			emu_initial_ram(ram_pnt(nidx), ram_size(nidx));

//			// reset della ram
//			// Note:
//			// All internal memory ($0000-$07FF) was consistently
//			// set to $ff except
//			//  $0008=$F7
//			//  $0009=$EF
//			//  $000a=$DF
//			//  $000f=$BF
//			//  Please note that you should NOT rely on the
//			//  state of any registers after Power-UP and especially
//			//  not the stack register and WRAM ($0000-$07FF).
//			ram_wr(0x008, 0xF7);
//			ram_wr(0x009, 0xEF);
//			ram_wr(0x00A, 0xDF);
//			//ram_wr(0x00B, 0xBF);
//			ram_wr(0x00F, 0xBF);
//
//			// questo workaround serve solo per 2nd2006.nes e 256inc.nes
//			if (info.mapper.id == 0) {
//				ram_wr(0x000, 0x00);
//			}

			// questo workaround serve solo per Dancing Blocks (Sachen) [!].nes
			if (info.mapper.id == 143) {
				ram_wr(nidx, 0x004, 0x00);
			}

			// questo workaround serve solo per Doraemon (J) (PRG0) [hM15].nes
			if (info.mapper.id == 15) {
				ram_wr(nidx, 0x018, 0x00);
			}

			// questo workaround serve solo per Ultimate Mortal Kombat 3 14 people (Unl)[!].nes
			if (info.mapper.id == 123) {
				ram_wr(nidx, 0x080 ,0x00);
			}
		}
	} else {
		nes[nidx].c.cpu.SR |= 0x04;
		nes[nidx].c.cpu.odd_cycle = 0;
		nes[nidx].c.cpu.cycles = 0;
		nes[nidx].c.cpu.opcode_cycle = 0;
		nes[nidx].c.cpu.double_wr = 0;
		nes[nidx].c.cpu.double_rd = 0;
		nes[nidx].c.cpu.jammed = 0;
	}
	memset(&nes[nidx].c.nmi, 0x00, sizeof(_nmi));
	memset(&nes[nidx].c.irq, 0x00, sizeof(_irq));
	// disassemblo il Processor Status Register
	disassemble_SR(nidx);
	// setto il flag di disabilitazione dell'irq
	nes[nidx].c.irq.inhibit = nes[nidx].c.cpu.im;
}
