/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "mem_map.h"
#include "ppu_inline.h"
#include "clock.h"
#include "cpu_inline.h"
#include "text.h"

enum cpu_opcode_type { RD_OP, WR_OP };

/* ----------------------------------------------------------------------
 *  metodi di indirizzamento
 * ----------------------------------------------------------------------
 */
#define IMP(opTy, cmd)\
{\
	cmd\
}
#define ZPG(opTy, cmd)\
{\
	WORD adr0 = _RDP & 0x00FF;\
	cmd\
}
#define ZPX(opTy, cmd, reg)\
{\
	WORD adr1 = _RDP;\
	/* garbage read */\
	_RDZPX_;\
	WORD adr0 = (adr1 + reg) & 0x00FF;\
	cmd\
}
#define ABS(opTy, cmd)\
{\
	WORD adr0 = lend_word(cpu.PC, FALSE, TRUE);\
	cpu.PC += 2;\
	cmd\
}
#define ABW(opTy, cmd)\
{\
	WORD adr0 = lend_word(cpu.PC, FALSE, FALSE);\
	_DMC;\
	cpu.PC += 2;\
	cmd\
}
#define ABX(opTy, cmd, reg)\
{\
	WORD adr2 = lend_word(cpu.PC, FALSE, TRUE);\
	WORD adr0 = adr2 + reg;\
	WORD adr1 = (adr2 & 0xFF00) | (BYTE) adr0;\
	/* puo' essere la lettura corretta\
	 * o anche semplice garbage */\
	_RDABX_;\
	cpu.PC += 2;\
	cmd\
}
#define AXW(opTy, cmd, reg)\
{\
	WORD adr2 = lend_word(cpu.PC, FALSE, TRUE);\
	WORD adr0 = adr2 + reg;\
	WORD adr1 = (adr2 & 0xFF00) | (BYTE) adr0;\
	/* puo' essere la lettura corretta\
	 * o anche semplice garbage */\
	_RDAXW_;\
	cpu.PC += 2;\
	cmd\
}
#define IDR(opTy)\
{\
	WORD adr0 = lend_word(cpu.PC, FALSE, TRUE);\
	cpu.PC = lend_word(adr0, TRUE, TRUE);\
}
#define IDX(opTy, cmd)\
{\
	WORD adr1 = _RDP;\
	/* garbage read */\
	_RDIDX_;\
	WORD adr0 = lend_word((adr1 + cpu.XR) & 0x00FF, TRUE, TRUE);\
	cmd\
}
#define IDY(opTy, cmd)\
{\
	WORD adr2 = lend_word(_RDP, TRUE, TRUE);\
	WORD adr0 = adr2 + cpu.YR;\
	WORD adr1 = (adr2 & 0xFF00) | (BYTE) adr0;\
	/* puo' essere la lettura corretta
	 * o anche semplice garbage */\
	_RDIDY_;\
	cmd\
}

/* Prove tecniche
#define IXW(opTy, cmd)\
{\
	WORD adr1 = _RDP;\
	_RDIDX_;\
	WORD adr0 = lend_word((adr1 + cpu.XR) & 0x00FF, TRUE, FALSE);\
	_DMC;\
	cmd;\
}
#define IXW(opTy, cmd)\
{\
	WORD adr1 = _RDP;\
	_RDIDX_;\
	WORD adr0 = lend_word((adr1 + cpu.XR) & 0x00FF, TRUE, FALSE);\
	_DMC;\
	cmd;\
}
#define IYW(opTy, cmd)\
{\
	WORD adr2 = lend_word(_RDP, TRUE, TRUE);\
	WORD adr0 = adr2 + cpu.YR;\
	WORD adr1 = (adr2 & 0xFF00) | (BYTE) adr0;\
	_RDIYW_;\
	cmd;\
}
#define _WRX(dst, src)\
	if (!DMC.tick_type) DMC.tick_type = DMC_CPU_WRITE;\
	cpu_wr_mem(dst, src)
*/

/* ----------------------------------------------------------------------
 *  istruzioni ufficiali
 * ----------------------------------------------------------------------
 */
/* ADC, SBC */
#define ADC(x)\
	x;\
	_ADC;
#define SBC(x)\
	x;\
	_SBC
/* AND, DEC, EOR, INC, ORA */
#define AND(x, opr) _RSZ(cpu.AR opr x;, cpu.AR)
#define INC(x, opr)\
	{\
	_MSZ(BYTE tmp = x opr 1, tmp, tmp);\
	}
/* ASL, LSR, ROL, ROR */
#define ASL(x) _SHF(x, _BSH(shift, 0x80, <<=), shift, shift)
#define LSR(x) _SHF(x, _BSH(shift, 0x01, >>=), shift, shift)
#define ROL(x) _SHF(x, _ROL(shift, 0x80, <<=), shift, shift)
#define ROR(x) _SHF(x, _ROR(shift, 0x01, >>=), shift, shift)
/* BCC, BCS, BEQ, BMI, BNF, BPL, BVC, BVS */
#define BRC(flag, condition)\
	WORD adr1 = (cpu.PC + 1);\
	WORD adr0 = adr1 + (SBYTE) _RDX(cpu.PC, TRUE);\
	if ((!flag) != condition) {\
		/*\
		 * A page boundary crossing occurs when the\
		 * branch destination is on a different page\
		 * than the instruction AFTER the branch\
		 * instruction.\
		 */\
		if ((adr0 & 0xFF00) == (adr1 & 0xFF00)) {\
			if (nmi.high && !nmi.before) {\
				nmi.delay = TRUE;\
			} else if (!(irq.inhibit & 0x04) && irq.high && !irq.before) {\
				irq.delay = TRUE;\
			}\
			mod_cycles_op(+=, 1);\
			tick_hw(1);\
		} else {\
			mod_cycles_op(+=, 2);\
			tick_hw(2);\
		}\
		cpu.PC = adr0;\
	} else {\
		cpu.PC++;\
	}
/* BIT */
#define BIT(x)\
	x;\
	cpu.sf = (cpu.openbus & 0x80);\
	cpu.of = (cpu.openbus & 0x40);\
	ZF((cpu.AR & cpu.openbus));
/* BRK, PHP
 *
 * NOTE:
 * lo Status Register viene salvato solo nello stack con
 * il bf settato a 1.
 */
#define BRK\
	/* dummy read */\
	_RDP;\
	_IRQ(cpu.SR | 0x10)
#define PHP\
	tick_hw(1);\
	ASS_SR;\
	_PSH(cpu.SR | 0x10);
/* CMP, CPX, CPY */
#define CMP(x, reg)\
	{\
	_RSZ(_CMP(x, reg);, (BYTE) cmp)\
	}
/* LDA, LDX, LDY */
#define LDX(x, reg) _RSZ(reg = x;, reg)
/* JMP */
#define JMP\
	WORD adr0 = lend_word(cpu.PC, FALSE, TRUE);\
	cpu.PC = adr0;
/* JSR, RTS */
#define JSR\
	WORD adr0 = lend_word(cpu.PC++, FALSE, TRUE);\
	_PSP;\
	cpu.PC = adr0;
#define RTS\
	/* dummy read */\
	_RDP;\
	tick_hw(1);\
	cpu.PC = _PUL;\
	cpu.PC = ((_PUL << 8) | cpu.PC) + 1;
/* RTI */
#define RTI\
	/* dummy read */\
	_RDP;\
	tick_hw(1);\
	/* il break flag (bit 4) e' sempre a 0 */\
	cpu.SR = (_PUL & 0xEF);\
	disassemble_SR();\
	/*\
	 * nell'RTI non c'e' nessun delay nel\
	 * settaggio dell'inibizione dell'IRQ.\
	 */\
	irq.inhibit = cpu.im;\
	cpu.PC = _PUL;\
	cpu.PC = (_PUL << 8) | cpu.PC;
/* SEI, PHA, PLA, PLP */
#define SEI\
	cpu.im = 0x04;\
	irq.inhibit |= 0x40;
#define PHA\
	tick_hw(1);\
	_PSH(cpu.AR);
#define PLA\
	tick_hw(2);\
	_RSZ(cpu.AR = _PUL;, cpu.AR)
#define PLP\
	tick_hw(2);\
	/* il break flag (bit 4) e' sempre a 0 */\
	cpu.SR = (_PUL & 0xEF);\
	disassemble_SR();\
	if (cpu.im) {\
		irq.inhibit |= 0x40;\
	}

/* ----------------------------------------------------------------------
 *  istruzioni non ufficiali
 * ----------------------------------------------------------------------
 */
/* AAC, ASR, ARR */
#define AAC\
	AND(_RDP, &=);\
	cpu.cf = cpu.sf >> 7;
#define ASR\
	cpu.AR &= _RDP;\
	_RSZ(_BSH(cpu.AR, 0x01, >>=);, cpu.AR)
#define ARR\
	cpu.AR &= _RDP;\
	_RSZ(_ROR(cpu.AR, 0x01, >>=);, cpu.AR)\
	cpu.cf = (cpu.AR & 0x40) >> 6;\
	cpu.of = (cpu.AR & 0x40) ^ ((cpu.AR & 0x20) << 1);
/* ATX */
#define ATX _RSZ(cpu.XR = cpu.AR = _RDP;, cpu.AR)
/* AXA */
#define AXA(cmd)\
	BYTE tmp = cpu.AR & cpu.XR & 0x07;\
	cmd
/* AXS */
#define AXS\
	cpu.XR &= cpu.AR;\
	_CMP(_RDP, cpu.XR);\
	_RSZ(cpu.XR = (BYTE) cmp;, cpu.XR)
/* AAX */
#define AAX\
	BYTE tmp;\
	tmp = cpu.AR & cpu.XR;\
	_AAXIDX(tmp)
/* DCP */
#define DCP(x)\
	BYTE tmp = x - 1;\
	CMP(tmp, cpu.AR)\
	_MSX(tmp);
/* ISC */
#define ISC(x)\
	BYTE tmp = x + 1;\
	_MSX(tmp);\
	cpu.openbus = tmp;\
	_RSZ(_SUB, cpu.AR)
/* LAS */
#define LAS\
	cpu.SR &= cpu.openbus;\
	_RSZ(cpu.AR = cpu.XR = cpu.SR;, cpu.AR)
/* LAX */
#define LAX(x)\
	x;\
	_LAX
/* RLA, SLO, SRE */
#define RLA(x) _SHF(x, _RLA(shift, 0x80, <<=), cpu.AR, shift)
#define SLO(x) _SHF(x, _SLO(shift, 0x80, <<=, |=), cpu.AR, shift)
#define SRE(x) _SHF(x, _SLO(shift, 0x01, >>=, ^=), cpu.AR, shift)
/* RRA */
#define RRA(x)\
	BYTE shift = x;\
	_ROR(shift, 0x01, >>=);\
	_MSX(shift);\
	cpu.openbus = shift;\
	_RSZ(_ADD, cpu.AR)
/* SXX */
#define SXX(reg)\
	BYTE tmp = reg & ((adr2 >> 8) + 1);\
	if (adr1 != adr0) adr0 &= 0x00FF;\
	_SXXABX(tmp)
/* XAA */
#define XAA\
	cpu.AR = cpu.XR;\
	cpu.AR &= _RDP;
/* XAS */
#define XAS\
	cpu.SR = cpu.AR & cpu.YR;\
	if (adr1 != adr0) adr0 &= 0x00FF;\
	BYTE tmp = cpu.SR & ((adr1 >> 8) + 1);\
	_XASABX(tmp)

/* ---------------------------------------------------------------------------------
 *  flags
 * ---------------------------------------------------------------------------------
 */
#define SF(x) cpu.sf = x & 0x80
#define SZ(x)\
	SF(x);\
	ZF(x)
#define ZF(x) cpu.zf = !x << 1
#define ASS_SR cpu.SR = (cpu.sf | cpu.of | 0x20 | cpu.bf | cpu.df |\
		cpu.im | cpu.zf | cpu.cf)

/* ----------------------------------------------------------------------
 *  varie ed eventuali
 * ----------------------------------------------------------------------
 */
#define _ADC _RSZ(_ADD, cpu.AR)
/* NOTE : BCD Addiction
 * 1a. AL = (A & $0F) + (B & $0F) + C
 * 1b. If AL >= $0A, then AL = ((AL + $06) & $0F) + $10
 * 1c. A = (A & $F0) + (B & $F0) + AL
 * 1d. Note that A can be >= $100 at this point
 * 1e. If (A >= $A0), then A = A + $60
 * 1f. The accumulator result is the lower 8 bits of A
 * 1g. The carry result is 1 if A >= $100, and is 0 if A < $100
 * Common BCD an Binary
 * 2e. The N flag result is 1 if bit 7 of A is 1, and is 0 if bit 7 if A is 0
 * 2f. The V flag result is 1 if A < -128 or A > 127, and is 0 if -128 <= A <= 127
 *
 * Importante!!! Nel NES il decimal mode non e' supportato ed inoltre
 * il codice per il decimal mode potrebbe essere buggato!!!!!!
 */
#define _ADD\
	{\
	WORD A;\
	if (FALSE && cpu.df) {\
		WORD AL = (cpu.AR & 0x0F) + (cpu.openbus & 0x0F) + cpu.cf;\
		if (AL >= 0x0A) { AL = ((AL + 0x06) & 0x0F) + 0x10; }\
		A = (cpu.AR & 0xF0) + (cpu.openbus & 0xF0) + AL;\
		if (A >= 0xA0) { A += 0x60; }\
	} else { A = cpu.AR + cpu.openbus + cpu.cf; }\
	cpu.cf = (A > 0xFF ? 1 : 0);\
	cpu.of = ((!((cpu.AR ^ cpu.openbus) & 0x80) &&\
			((cpu.AR ^ A) & 0x80)) ? 0x40 : 0);\
	cpu.AR = (BYTE) A;\
	}
#define _BSH(dst, bitmask, opr)\
	cpu.cf = ((dst & bitmask) ? 1 : 0);\
	dst opr 1
#define _CMP(x, reg)\
	WORD cmp = reg - x;\
	cpu.cf = (cmp < 0x100 ? 1 : 0)
#define _CYW(cmd) _CY_(cmd, mod_cycles_op(+=, 1);)
#define _CY_(cmd1, cmd2)\
	if (adr1 != adr0) {\
		cpu.double_rd = TRUE;\
		cmd2\
		_RD0;\
	}\
	cmd1
#define _DMC\
	DMC.tick_type = DMC_CPU_WRITE;\
	if (adr0 == 0x4014) {\
		DMC.tick_type = DMC_R4014;\
	}\
	tick_hw(1)
#define _LAX\
	cpu.AR = _RDB;\
	_RSZ(cpu.XR = cpu.AR;, cpu.XR)
#define _MSZ(cmd, result1, result2)\
	_RSZ(cmd;, result1)\
	_MSX(result2)
#define _MSX(result)\
	_ASLWR1(_RDB)\
	cpu.double_wr = TRUE;\
	_ASLWR2(result)\
	cpu.double_wr = FALSE
#define _PUL _RDX(((++cpu.SP) + STACK), TRUE)
#define _PSH(src) _WRX((cpu.SP--) + STACK, src)
#define _PSP\
	_PSH(cpu.PC >> 8);\
	_PSH(cpu.PC)
#define _RD0 _RDX(adr0, TRUE)
#define _RD1 _RDX(adr1, TRUE)
#define _RD2\
	_RDX(adr1, FALSE);\
	_DMC
#define _RDB cpu.openbus
#define _RDP _RDX(cpu.PC++, TRUE)
#define _RDX(src, LASTTICKHW) cpu_rd_mem(src, LASTTICKHW)
#define _RLA(dst, bitmask, opr)\
	_ROX(dst, bitmask, opr, old_cf);\
	cpu.AR &= shift
#define _ROL(dst, bitmask, opr)\
	_ROX(dst, bitmask, opr, old_cf)
#define _ROR(dst, bitmask, opr)\
	_ROX(dst, bitmask, opr, (old_cf << 7))
#define _ROX(dst, bitmask, opr, oprnd)\
	{\
	BYTE old_cf = cpu.cf;\
	_BSH(dst, bitmask, opr);\
	dst |= oprnd;\
	}
#define _RSZ(cmd, result)\
	cmd\
	SZ(result);
#define _SBC _RSZ(_SUB, cpu.AR)
#define _SHF(x, cmd, result1, result2)\
	{\
	BYTE shift = x;\
	_MSZ(cmd, result1, result2);\
	}
#define _SLO(dst, bitmask, opr1, opr2)\
	_BSH(dst, bitmask, opr1);\
	cpu.AR opr2 shift

/* NOTE : BCD Subtraction
 * 3a. AL = (A & $0F) - (B & $0F) + C-1
 * 3b. If AL < 0, then AL = ((AL - $06) & $0F) - $10
 * 3c. A = (A & $F0) - (B & $F0) + AL
 * 3d. If A < 0, then A = A - $60
 * 3e. The accumulator result is the lower 8 bits of A

 * Importante!!! Nel NES il decimal mode non e' supportato ed inoltre
 * il codice per il decimal mode potrebbe essere buggato!!!!!!
 */
#define _SUB\
	{\
	WORD A;\
	if (FALSE && cpu.df) {\
		WORD AL = (cpu.AR & 0x0F) - (cpu.openbus & 0x0F) - !cpu.cf;\
		if (AL < 0) { AL = ((AL - 0x06) & 0x0F) - 0x10; }\
		A = (cpu.AR & 0xF0) - (cpu.openbus & 0xF0) + AL;\
		if (A < 0) { A -= 0x60; }\
	} else { A = cpu.AR - cpu.openbus - !cpu.cf; }\
	cpu.cf = (A < 0x100 ? 1 : 0);\
	cpu.of = (((cpu.AR ^ cpu.openbus) & 0x80) & ((cpu.AR ^ A) & 0x80) ? 0x40 : 0);\
	cpu.AR = (BYTE) A;\
	}
#define _WR0(reg) _WRX(adr0, reg);
#define _WRX(dst, reg) cpu_wr_mem(dst, reg)

/* IRQ */
#define _IRQ(flags)\
	BYTE flagNMI = FALSE;\
	_PSP;\
	if (nmi.high) {\
		flagNMI = TRUE;\
	}\
	ASS_SR;\
	_PSH(flags);\
	cpu.im = irq.inhibit = 0x04;\
	if (flagNMI) {\
		nmi.high = nmi.delay = FALSE;\
		cpu.PC = lend_word(INT_NMI, FALSE, TRUE);\
	} else {\
		cpu.PC = lend_word(INT_IRQ, FALSE, TRUE);\
		if (nmi.high) {\
			nmi.delay = TRUE;\
		}\
	}
#define IRQ(flags)\
	tick_hw(1);\
	_IRQ(flags)
#define NMI\
	nmi.high = nmi.delay = FALSE;\
	tick_hw(1);\
	_PSP;\
	ASS_SR;\
	_PSH(cpu.SR & 0xEF);\
	cpu.im = irq.inhibit = 0x04;\
	cpu.PC = lend_word(INT_NMI, FALSE, TRUE);

#define _RDZPG  _RD0
#define _RDZPX_ _RD1
#define _RDZPX  _RD0
#define _RDABS  _RD0
#define _RDABX_ _RD1
#define _RDAXW_ _RD2
#define _RDABX  _RD0
#define _RDIDX_ _RD1
#define _RDIDX  _RD0
#define _RDIDY_ _RD1
#define _RDIYW_ _RD2
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
#define _AXAABX(reg) _WR0(reg)
#define _AXAIDY(reg) _WR0(reg)
#define _XASABX(reg) _WR0(reg)

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

void cpu_exe_op(void) {
	cpu.opcode = FALSE;
	DMC.tick_type = DMC_NORMAL;
	cpu.opcode_PC = cpu.PC;

	/* ------------------------------------------------ */
	/*                   IRQ handler                    */
	/* ------------------------------------------------ */
	/*
	 * IRQ supportati: Esterni, BRK, APU frame counter ed NMI.
	 *
	 * Note:
	 * e' importante che dell'inhibit esamini solo
	 * gli ultimi 4 bit perche' i primi 4 potrebbero
	 * contenere il suo nuovo valore.
	 */
	if (irq.high && !(irq.inhibit & 0x04)) {
		/*
		 * se l'IRQ viene raggiunto nell'ultimo ciclo
		 * dell'istruzione precedente (irq.before == 0)
		 * devo eseguire l'istruzione successiva e solo
		 * dopo avviarlo.
		 */
		if (!irq.before || irq.delay) {
			irq.delay = FALSE;
		} else {
			cpu.opcode = 0x200;
		}
	}
	if (nmi.high) {
		cpu.opcode = 0;
		/*
		 * se l'NMI viene raggiunto nell'ultimo ciclo
		 * dell'istruzione precedente (nmi.before = 0)
		 * oppure durante un BRK o un IRQ (dal quinto ciclo
		 * in poi), devo eseguire l'istruzione successiva e
		 * solo dopo avviarlo.
		 */
		if (!nmi.before || nmi.delay) {
			nmi.delay = FALSE;
		} else {
			cpu.opcode = 0x100;
		}
	}
	/*
	 * se codeop e' valorizzato (NMI o IRQ) eseguo
	 * un tick hardware altrimenti devo leggere la
	 * prossima istruzione.
	 */
	if (cpu.opcode & 0x300) {
		tick_hw(1);
	} else {
		/* memorizzo l'opcode attuale */
		cpu.opcode = _RDP;
	}

	/*
	 * azzero le variabili che utilizzo per sapere
	 * quando avviene il DMA del DMC durante l'istruzione.
	 */
	cpu.opcode_cycle = DMC.dma_cycle = 0;
	/* flag della doppia lettura di un registro */
	cpu.double_rd = FALSE;

	/*
	 * End of vertical blanking, sometime in pre-render
	 * scanline: Set NMI_occurred to false.
	 * (fonte: http://wiki.nesdev.com/w/index.php/NMI)
	 *
	 * FIXME: non sono affatto sicuro di questa cosa.
	 * Disabilito l'NMI quando viene settato (dal registro $2000)
	 * nello stesso momento in cui il vblank viene disabilitato
	 * (ppu.frame_x = 0). In questo modo passo la rom di test
	 * 07-nmi_on_timing.nes. Non ho trovato informazioni su quando
	 * effettivamente questa situazione avvenga.
	 */
	if (nmi.high && !nmi.frame_x && (ppu.frame_y == ppu_sclines.vint)) {
		nmi.high = nmi.delay = FALSE;
	}
	/*
	 * le istruzioni CLI, SEI, e PLP ritardano il
	 * cambiamento dell'interrupt mask (im) fino
	 * all'istruzione successiva, o meglio l'im
	 * viene modificato immediatamente nello Status
	 * Register (SR) ma l'effettiva funzione del
	 * flag viene ritardata di un'istruzione.
	 * L'RTI non soffre di questo difetto.
	 */
	if (irq.inhibit != cpu.im) {
		/*
		 * l'IRQ handler ha gia' controllato la
		 * presenza di IRQ con il vecchio valore
		 * del flag di inibizione. Adesso, se
		 * serve, posso impostare il nuovo.
		 */
		irq.inhibit >>= 4;
	}
	/* ------------------------------------------------ */

	/* salvo i cicli presi dall'istruzione... */
	cpu.base_opcode_cycles = table_opcode_cycles[(BYTE) cpu.opcode];
	mod_cycles_op(+=, cpu.base_opcode_cycles);

	/* ... e la eseguo */
	switch (cpu.opcode) {

	case 0x69: IMP(RD_OP, ADC(_RDP)) break;                              // ADC #IMM
	case 0x65: ZPG(RD_OP, ADC(_RDZPG)) break;                            // ADC $ZPG
	case 0x75: ZPX(RD_OP, ADC(_RDZPX), cpu.XR) break;                    // ADC $ZPG,X
	case 0x6D: ABS(RD_OP, ADC(_RDABS)) break;                            // ADC $ABS
	case 0x7D: ABX(RD_OP, _CYW(_ADC), cpu.XR) break;                     // ADC $ABS,X
	case 0x79: ABX(RD_OP, _CYW(_ADC), cpu.YR) break;                     // ADC $ABS,Y
	case 0x61: IDX(RD_OP, ADC(_RDIDX)) break;                            // ADC ($IND,X)
	case 0x71: IDY(RD_OP, _CYW(_ADC)) break;                             // ADC ($IND),Y

	case 0x29: IMP(RD_OP, AND(_RDP, &=)) break;                          // AND #IMM
	case 0x25: ZPG(RD_OP, AND(_RDZPG, &=)) break;                        // AND $ZPG
	case 0x35: ZPX(RD_OP, AND(_RDZPX, &=), cpu.XR) break;                // AND $ZPG,X
	case 0x2D: ABS(RD_OP, AND(_RDABS, &=)) break;                        // AND $ABS
	case 0x3D: ABX(RD_OP, _CYW(AND(_RDB, &=)), cpu.XR) break;            // AND $ABS,X
	case 0x39: ABX(RD_OP, _CYW(AND(_RDB, &=)), cpu.YR) break;            // AND $ABS,Y
	case 0x21: IDX(RD_OP, AND(_RDIDX, &=)) break;                        // AND ($IND,X)
	case 0x31: IDY(RD_OP, _CYW(AND(_RDB, &=))) break;                    // AND ($IND),Y

	case 0x0A: IMP(RD_OP, _RSZ(_BSH(cpu.AR, 0x80, <<=);, cpu.AR)) break; // ASL [AR]
	case 0x06: ZPG(WR_OP, ASL(_RDZPG)) break;                            // ASL $ZPG
	case 0x16: ZPX(WR_OP, ASL(_RDZPX), cpu.XR) break;                    // ASL $ZPG,X
	case 0x0E: ABS(WR_OP, ASL(_RDABS)) break;                            // ASL $ABS
	case 0x1E: ABX(WR_OP, ASL(_RDABX), cpu.XR) break;                    // ASL $ABS,X

	case 0x90: IMP(RD_OP, BRC(cpu.cf, FALSE)) break;                     // BCC [C = 0]
	case 0xB0: IMP(RD_OP, BRC(cpu.cf, TRUE)) break;                      // BCS [C = 1]
	case 0xF0: IMP(RD_OP, BRC(cpu.zf, TRUE)) break;                      // BEQ [Z = 1]
	case 0x30: IMP(RD_OP, BRC(cpu.sf, TRUE)) break;                      // BMI [S = 1]
	case 0xD0: IMP(RD_OP, BRC(cpu.zf, FALSE)) break;                     // BNE [Z = 0]
	case 0x10: IMP(RD_OP, BRC(cpu.sf, FALSE)) break;                     // BPL [S = 0]
	case 0x50: IMP(RD_OP, BRC(cpu.of, FALSE)) break;                     // BVC [O = 0]
	case 0x70: IMP(RD_OP, BRC(cpu.of, TRUE)) break;                      // BVS [O = 1]

	case 0x24: ZPG(RD_OP, BIT(_RDZPG)) break;                            // BIT $ZPG
	case 0x2C: ABS(RD_OP, BIT(_RDABS)) break;                            // BIT $ABS

	case 0x00: IMP(RD_OP, BRK) break;                                    // BRK

	case 0x18: IMP(RD_OP, cpu.cf = FALSE;) break;                        // CLC [C -> 0]
	case 0xD8: IMP(RD_OP, cpu.df = FALSE;) break;                        // CLD [D -> 0]
	case 0x58: IMP(RD_OP, cpu.im = FALSE;) break;                        // CLI [I -> 0]
	case 0xB8: IMP(RD_OP, cpu.of = FALSE;) break;                        // CLV [O -> 0]

	case 0xC9: IMP(RD_OP, CMP(_RDP, cpu.AR)) break;                      // CMP #IMM
	case 0xC5: ZPG(RD_OP, CMP(_RDZPG, cpu.AR)) break;                    // CMP $ZPG
	case 0xD5: ZPX(RD_OP, CMP(_RDZPX, cpu.AR), cpu.XR) break;            // CMP $ZPG,X
	case 0xCD: ABS(RD_OP, CMP(_RDABS, cpu.AR)) break;                    // CMP $ABS
	case 0xDD: ABX(RD_OP, _CYW(CMP(_RDB, cpu.AR)), cpu.XR) break;        // CMP $ABS,X
	case 0xD9: ABX(RD_OP, _CYW(CMP(_RDB, cpu.AR)), cpu.YR) break;        // CMP $ABS,Y
	case 0xC1: IDX(RD_OP, CMP(_RDIDX, cpu.AR)) break;                    // CMP ($IND,X)
	case 0xD1: IDY(RD_OP, _CYW(CMP(_RDB, cpu.AR))) break;                // CMP ($IND),Y

	case 0xE0: IMP(RD_OP, CMP(_RDP, cpu.XR)) break;                      // CPX #IMM
	case 0xE4: ZPG(RD_OP, CMP(_RDZPG, cpu.XR)) break;                    // CPX $ZPG
	case 0xEC: ABS(RD_OP, CMP(_RDABS, cpu.XR)) break;                    // CPX $ABS

	case 0xC0: IMP(RD_OP, CMP(_RDP, cpu.YR)) break;                      // CPY #IMM
	case 0xC4: ZPG(RD_OP, CMP(_RDZPG, cpu.YR)) break;                    // CPY $ZPG
	case 0xCC: ABS(RD_OP, CMP(_RDABS, cpu.YR)) break;                    // CPY $ABS

	case 0xC6: ZPG(WR_OP, INC(_RDZPG, -)) break;                         // DEC $ZPG
	case 0xD6: ZPX(WR_OP, INC(_RDZPX, -), cpu.XR) break;                 // DEC $ZPG,X
	case 0xCE: ABS(WR_OP, INC(_RDABS, -)) break;                         // DEC $ABS
	case 0xDE: ABX(WR_OP, INC(_RDABX, -), cpu.XR) break;                 // DEC $ABS,X

	case 0xCA: IMP(RD_OP, _RSZ(cpu.XR--;, cpu.XR)) break;                // DEX [XR]

	case 0x88: IMP(RD_OP, _RSZ(cpu.YR--;, cpu.YR)) break;                // DEY [YR]

	case 0x49: IMP(RD_OP, AND(_RDP, ^=)) break;                          // EOR #IMM
	case 0x45: ZPG(RD_OP, AND(_RDZPG, ^=)) break;                        // EOR $ZPG
	case 0x55: ZPX(RD_OP, AND(_RDZPX, ^=), cpu.XR) break;                // EOR $ZPG,X
	case 0x4D: ABS(RD_OP, AND(_RDABS, ^=)) break;                        // EOR $ABS
	case 0x5D: ABX(RD_OP, _CYW(AND(_RDB, ^=)), cpu.XR) break;            // EOR $ABS,X
	case 0x59: ABX(RD_OP, _CYW(AND(_RDB, ^=)), cpu.YR) break;            // EOR $ABS,Y
	case 0x41: IDX(RD_OP, AND(_RDIDX, ^=)) break;                        // EOR ($IND,X)
	case 0x51: IDY(RD_OP, _CYW(AND(_RDB, ^=))) break;                    // EOR ($IND),Y

	case 0xE6: ZPG(WR_OP, INC(_RDZPG, +)) break;                         // INC $ZPG
	case 0xF6: ZPX(WR_OP, INC(_RDZPX, +), cpu.XR) break;                 // INC $ZPG,X
	case 0xEE: ABS(WR_OP, INC(_RDABS, +)) break;                         // INC $ABS
	case 0xFE: ABX(WR_OP, INC(_RDABX, +), cpu.XR) break;                 // INC $ABS,X

	case 0xE8: IMP(RD_OP, _RSZ(cpu.XR++;, cpu.XR)) break;                // INX [XR]

	case 0xC8: IMP(RD_OP, _RSZ(cpu.YR++;, cpu.YR)) break;                // INY [YR]

	case 0x4C: IMP(RD_OP, JMP) break;                                    // JMP $ABS
	case 0x6C: IDR(RD_OP) break;                                         // JMP ($IND)

	case 0x20: IMP(RD_OP, JSR) break;                                    // JSR $ABS

	case 0xA9: IMP(RD_OP, LDX(_RDP, cpu.AR)) break;                      // LDA #IMM
	case 0xA5: ZPG(RD_OP, LDX(_RDZPG, cpu.AR)) break;                    // LDA $ZPG
	case 0xB5: ZPX(RD_OP, LDX(_RDZPX, cpu.AR), cpu.XR) break;            // LDA $ZPG,X
	case 0xAD: ABS(RD_OP, LDX(_RDABS, cpu.AR)) break;                    // LDA $ABS
	case 0xBD: ABX(RD_OP, _CYW(LDX(_RDB, cpu.AR)), cpu.XR) break;        // LDA $ABS,X
	case 0xB9: ABX(RD_OP, _CYW(LDX(_RDB, cpu.AR)), cpu.YR) break;        // LDA $ABS,Y
	case 0xA1: IDX(RD_OP, LDX(_RDIDX, cpu.AR)) break;                    // LDA ($IND,X)
	case 0xB1: IDY(RD_OP, _CYW(LDX(_RDB, cpu.AR))) break;                // LDA ($IND),Y

	case 0xA2: IMP(RD_OP, LDX(_RDP, cpu.XR)) break;                      // LDX #IMM
	case 0xA6: ZPG(RD_OP, LDX(_RDZPG,cpu.XR)) break;                     // LDX $ZPG
	case 0xB6: ZPX(RD_OP, LDX(_RDZPX,cpu.XR), cpu.YR) break;             // LDX $ZPG,Y
	case 0xAE: ABS(RD_OP, LDX(_RDABS,cpu.XR)) break;                     // LDX $ABS
	case 0xBE: ABX(RD_OP, _CYW(LDX(_RDB, cpu.XR)), cpu.YR) break;        // LDX $ABS,Y

	case 0xA0: IMP(RD_OP, LDX(_RDP, cpu.YR)) break;                      // LDY #IMM
	case 0xA4: ZPG(RD_OP, LDX(_RDZPG, cpu.YR)) break;                    // LDY $ZPG
	case 0xB4: ZPX(RD_OP, LDX(_RDZPX, cpu.YR), cpu.XR) break;            // LDY $ZPG,X
	case 0xAC: ABS(RD_OP, LDX(_RDABS, cpu.YR)) break;                    // LDY $ABS
	case 0xBC: ABX(RD_OP, _CYW(LDX(_RDB, cpu.YR)), cpu.XR) break;        // LDY $ABS,X

	case 0x4A: IMP(RD_OP, _RSZ(_BSH(cpu.AR, 0x01, >>=);, cpu.AR)) break; // LSR [AR]
	case 0x46: ZPG(WR_OP, LSR(_RDZPG)) break;                            // LSR $ZPG
	case 0x56: ZPX(WR_OP, LSR(_RDZPX), cpu.XR) break;                    // LSR $ZPG,X
	case 0x4E: ABS(WR_OP, LSR(_RDABS)) break;                            // LSR $ABS
	case 0x5E: ABX(WR_OP, LSR(_RDABX), cpu.XR) break;                    // LSR $ABS,X

	case 0xEA: break;                                                    // NOP

	case 0x09: IMP(RD_OP, AND(_RDP, |=)) break;                          // ORA #IMM
	case 0x05: ZPG(RD_OP, AND(_RDZPG, |=)) break;                        // ORA $ZPG
	case 0x15: ZPX(RD_OP, AND(_RDZPX, |=), cpu.XR) break;                // ORA $ZPG,X
	case 0x0D: ABS(RD_OP, AND(_RDABS, |=)) break;                        // ORA $ABS
	case 0x1D: ABX(RD_OP, _CYW(AND(_RDB, |=)), cpu.XR) break;            // ORA $ABS,X
	case 0x19: ABX(RD_OP, _CYW(AND(_RDB, |=)), cpu.YR) break;            // ORA $ABS,Y
	case 0x01: IDX(RD_OP, AND(_RDIDX, |=)) break;                        // ORA ($IND,X)
	case 0x11: IDY(RD_OP, _CYW(AND(_RDB, |=))) break;                    // ORA ($IND),Y

	case 0x48: IMP(WR_OP, PHA) break;                                    // PHA
	case 0x08: IMP(WR_OP, PHP) break;                                    // PHP

	case 0x68: IMP(WR_OP, PLA) break;                                    // PLA
	case 0x28: IMP(WR_OP, PLP) break;                                    // PLP

	case 0x2A: IMP(RD_OP, _RSZ(_ROL(cpu.AR, 0x80, <<=), cpu.AR)) break;  // ROL [AR]
	case 0x26: ZPG(WR_OP, ROL(_RDZPG)) break;                            // ROL $ZPG
	case 0x36: ZPX(WR_OP, ROL(_RDZPX), cpu.XR) break;                    // ROL $ZPG,X
	case 0x2E: ABS(WR_OP, ROL(_RDABS)) break;                            // ROL $ABS
	case 0x3E: ABX(WR_OP, ROL(_RDABX), cpu.XR) break;                    // ROL $ABS,X

	case 0x6A: IMP(RD_OP, _RSZ(_ROR(cpu.AR, 0x01, >>=), cpu.AR)) break;  // ROR [AR]
	case 0x66: ZPG(WR_OP, ROR(_RDZPG)) break;                            // ROR $ZPG
	case 0x76: ZPX(WR_OP, ROR(_RDZPX), cpu.XR) break;                    // ROR $ZPG,X
	case 0x6E: ABS(WR_OP, ROR(_RDABS)) break;                            // ROR $ABS
	case 0x7E: ABX(WR_OP, ROR(_RDABX), cpu.XR) break;                    // ROR $ABS,X

	case 0x40: IMP(RD_OP, RTI) break;                                    // RTI

	case 0x60: IMP(RD_OP, RTS) break;                                    // RTS

	case 0xE9: IMP(RD_OP, SBC(_RDP)) break;                              // SBC #IMM
	case 0xE5: ZPG(RD_OP, SBC(_RDZPG)) break;                            // SBC $ZPG
	case 0xF5: ZPX(RD_OP, SBC(_RDZPX), cpu.XR) break;                    // SBC $ZPG,X
	case 0xED: ABS(RD_OP, SBC(_RDABS)) break;                            // SBC $ABS
	case 0xFD: ABX(RD_OP, _CYW(_SBC), cpu.XR) break;                     // SBC $ABS,X
	case 0xF9: ABX(RD_OP, _CYW(_SBC), cpu.YR) break;                     // SBC $ABS,Y
	case 0xE1: IDX(RD_OP, SBC(_RDIDX)) break;                            // SBC ($IND,X)
	case 0xF1: IDY(RD_OP, _CYW(_SBC)) break;                             // SBC ($IND),Y

	case 0x38: IMP(RD_OP, cpu.cf = 0x01;) break;                         // SEC [C -> 1]
	case 0xF8: IMP(RD_OP, cpu.df = 0x08;) break;                         // SED [D -> 1]
	case 0x78: IMP(RD_OP, SEI) break;                                    // SEI [I -> 1]

	case 0x85: ZPG(WR_OP, _STXZPG(cpu.AR)) break;                        // STA $ZPG
	case 0x95: ZPX(WR_OP, _STXZPX(cpu.AR), cpu.XR) break;                // STA $ZPG,X
	case 0x8D: ABW(WR_OP, _STXABS(cpu.AR)) break;                        // STA $ABS
	case 0x9D: ABX(WR_OP, _STXABX(cpu.AR), cpu.XR) break;                // STA $ABS,X
	case 0x99: ABX(WR_OP, _STXABX(cpu.AR), cpu.YR) break;                // STA $ABS,Y
	case 0x81: IDX(WR_OP, _STXIDX(cpu.AR)) break;                        // STA ($IND,X)
	case 0x91: IDY(WR_OP, _STXIDY(cpu.AR)) break;                        // STA ($IND),Y

	case 0x86: ZPG(WR_OP, _STXZPG(cpu.XR)) break;                        // STX $ZPG
	case 0x96: ZPX(WR_OP, _STXZPX(cpu.XR), cpu.YR) break;                // STX $ZPG,Y
	case 0x8E: ABW(WR_OP, _STXABS(cpu.XR)) break;                        // STX $ABS

	case 0x84: ZPG(WR_OP, _STXZPG(cpu.YR)) break;                        // STY $ZPG
	case 0x94: ZPX(WR_OP, _STXZPX(cpu.YR), cpu.XR) break;                // STY $ZPG,X
	case 0x8C: ABW(WR_OP, _STXABS(cpu.YR)) break;                        // STY $ABS

	case 0xAA: IMP(RD_OP, _RSZ(cpu.XR = cpu.AR;, cpu.XR)) break;         // TAX
	case 0xA8: IMP(RD_OP, _RSZ(cpu.YR = cpu.AR;, cpu.YR)) break;         // TAY
	case 0xBA: IMP(RD_OP, _RSZ(cpu.XR = cpu.SP;, cpu.XR)) break;         // TSX
	case 0x8A: IMP(RD_OP, _RSZ(cpu.AR = cpu.XR;, cpu.AR)) break;         // TXA
	case 0x9A: IMP(RD_OP, cpu.SP = cpu.XR;) break;                       // TXS
	case 0x98: IMP(RD_OP, _RSZ(cpu.AR = cpu.YR;, cpu.AR)) break;         // TYA

	/* illegal opcodes */
#if !defined (ILLEGAL)
	case 0x0B:                                                           // AAC #IMM
	case 0x2B: IMP(RD_OP, AAC) break;                                    // AAC #IMM

	case 0x87: ZPG(WR_OP, AAX) break;                                    // AAX $ZPG
	case 0x97: ZPX(WR_OP, AAX, cpu.YR) break;                            // AAX $ZPG,Y
	case 0x8F: ABS(WR_OP, AAX) break;                                    // AAX $ABS
	case 0x83: IDX(WR_OP, AAX) break;                                    // AAX ($IND,X)

	case 0x6B: IMP(RD_OP, ARR) break;                                    // ARR #IMM

	case 0x4B: IMP(RD_OP, ASR) break;                                    // ASR #IMM

	case 0xAB: IMP(RD_OP, ATX) break;                                    // ATX #IMM

	case 0xCB: IMP(RD_OP, AXS) break;                                    // AXS #IMM

	case 0xC7: ZPG(WR_OP, DCP(_RDZPG)) break;                            // DCP $ZPG
	case 0xD7: ZPX(WR_OP, DCP(_RDZPX), cpu.XR) break;                    // DCP $ZPG,X
	case 0xCF: ABS(WR_OP, DCP(_RDABS)) break;                            // DCP $ABS
	case 0xDF: ABX(WR_OP, DCP(_RDABX), cpu.XR) break;                    // DCP $ABS,X
	case 0xDB: ABX(WR_OP, DCP(_RDABX), cpu.YR) break;                    // DCP $ABS,Y
	case 0xC3: IDX(WR_OP, DCP(_RDIDX)) break;                            // DCP ($IND,X)
	case 0xD3: IDY(WR_OP, DCP(_RDABX)) break;                            // DCP ($IND),Y

	case 0x80:                                                           // DOP #IMM
	case 0x82:                                                           // DOP #IMM
	case 0x89:                                                           // DOP #IMM
	case 0XC2:                                                           // DOP #IMM
	case 0XE2: cpu.PC++; break;                                          // DOP #IMM
	case 0x04:                                                           // DOP $ZPG
	case 0x44:                                                           // DOP $ZPG
	case 0x64: ZPG(RD_OP, adr0 = adr0;) break;                           // DOP $ZPG
	case 0x14:                                                           // DOP $ZPG,X
	case 0x34:                                                           // DOP $ZPG,X
	case 0x54:                                                           // DOP $ZPG,X
	case 0x74:                                                           // DOP $ZPG,X
	case 0xD4:                                                           // DOP $ZPG,X
	case 0xF4: ZPX(RD_OP, adr0 = adr0;, cpu.XR) break;                   // DOP $ZPG,X

	case 0xE7: ZPG(WR_OP, ISC(_RDZPG)) break;                            // ISC $ZPG
	case 0xF7: ZPX(WR_OP, ISC(_RDZPX), cpu.XR) break;                    // ISC $ZPG,X
	case 0xEF: ABS(WR_OP, ISC(_RDABS)) break;                            // ISC $ABS
	case 0xFF: ABX(WR_OP, ISC(_RDABX), cpu.XR) break;                    // ISC $ABS,X
	case 0xFB: ABX(WR_OP, ISC(_RDABX), cpu.YR) break;                    // ISC $ABS,Y
	case 0xE3: IDX(WR_OP, ISC(_RDIDX)) break;                            // ISC ($IND,X)
	case 0xF3: IDY(WR_OP, _CY_(ISC(_RDB),)) break;                       // ISC ($IND),Y

	case 0xA7: ZPG(RD_OP, LAX(_RDZPG)) break;                            // LAX $ZPG
	case 0xB7: ZPX(RD_OP, LAX(_RDABX), cpu.YR) break;                    // LAX $ZPG,Y
	case 0xAF: ABS(RD_OP, LAX(_RDABS)) break;                            // LAX $ABS
	case 0xBF: ABX(RD_OP, _CYW(_LAX), cpu.YR) break;                     // LAX $ABS,Y
	case 0xA3: IDX(RD_OP, LAX(_RDIDX)) break;                            // LAX ($IND,X)
	case 0xB3: IDY(RD_OP, _CYW(_LAX)) break;                             // LAX ($IND),Y

	case 0x1A:                                                           // NOP
	case 0x3A:                                                           // NOP
	case 0x5A:                                                           // NOP
	case 0x7A:                                                           // NOP
	case 0xDA:                                                           // NOP
	case 0xFA: break;                                                    // NOP

	case 0x27: ZPG(WR_OP, RLA(_RDZPG)) break;                            // RLA $ZPG
	case 0x37: ZPX(WR_OP, RLA(_RDZPX), cpu.XR) break;                    // RLA $ZPG,X
	case 0x2F: ABS(WR_OP, RLA(_RDABS)) break;                            // RLA $ABS
	case 0x3F: ABX(WR_OP, RLA(_RDABX), cpu.XR) break;                    // RLA $ABS,X
	case 0x3B: ABX(WR_OP, RLA(_RDABX), cpu.YR) break;                    // RLA $ABS,Y
	case 0x23: IDX(WR_OP, RLA(_RDIDX)) break;                            // RLA ($IND,X)
	case 0x33: IDY(WR_OP, _CY_(RLA(_RDB),)) break;                       // RLA ($IND),Y

	case 0x67: ZPG(WR_OP, RRA(_RDZPG)) break;                            // RRA $ZPG
	case 0x77: ZPX(WR_OP, RRA(_RDZPX), cpu.XR) break;                    // RRA $ZPG,X
	case 0x6F: ABS(WR_OP, RRA(_RDABS)) break;                            // RRA $ABS
	case 0x7F: ABX(WR_OP, RRA(_RDABX), cpu.XR) break;                    // RRA $ABS,X
	case 0x7B: ABX(WR_OP, RRA(_RDABX), cpu.YR) break;                    // RRA $ABS,Y
	case 0x63: IDX(WR_OP, RRA(_RDIDX)) break;                            // RRA ($IND,X)
	case 0x73: IDY(WR_OP, _CY_(RRA(_RDB),)) break;                       // RRA ($IND),Y

	case 0xEB: IMP(RD_OP, SBC(_RDP)) break;                              // SBC #IMM

	case 0x07: ZPG(WR_OP, SLO(_RDZPG)) break;                            // SLO $ZPG
	case 0x17: ZPX(WR_OP, SLO(_RDZPX), cpu.XR) break;                    // SLO $ZPG,X
	case 0x0F: ABS(WR_OP, SLO(_RDABS)) break;                            // SLO $ABS
	case 0x1F: ABX(WR_OP, SLO(_RDABX), cpu.XR) break;                    // SLO $ABS,X
	case 0x1B: ABX(WR_OP, SLO(_RDABX), cpu.YR) break;                    // SLO $ABS,Y
	case 0x03: IDX(WR_OP, SLO(_RDIDX)) break;                            // SLO ($IND,X)
	case 0x13: IDY(WR_OP, _CY_(SLO(_RDB),)) break;                       // SLO ($IND),Y

	case 0x47: ZPG(WR_OP, SRE(_RDZPG)) break;                            // SRE $ZPG
	case 0x57: ZPX(WR_OP, SRE(_RDZPX), cpu.XR) break;                    // SRE $ZPG,X
	case 0x4F: ABS(WR_OP, SRE(_RDABS)) break;                            // SRE $ABS
	case 0x5F: ABX(WR_OP, SRE(_RDABX), cpu.XR) break;                    // SRE $ABS,X
	case 0x5B: ABX(WR_OP, SRE(_RDABX), cpu.YR) break;                    // SRE $ABS,Y
	case 0x43: IDX(WR_OP, SRE(_RDIDX)) break;                            // SRE ($IND,X)
	case 0x53: IDY(WR_OP, _CY_(SRE(_RDB),)) break;                       // SRE ($IND),Y

	case 0x0C: ABS(RD_OP, adr0 = adr0;) break;                           // TOP $ABS
	case 0x1C:                                                           // TOP $ABS,X
	case 0x3C:                                                           // TOP $ABS,X
	case 0X5C:                                                           // TOP $ABS,X
	case 0X7C:                                                           // TOP $ABS,X
	case 0XDC:                                                           // TOP $ABS,X
	case 0XFC: ABX(RD_OP, _CYW(adr0 = 0;), cpu.XR) break;                // TOP $ABS,X

	case 0x9C: ABX(WR_OP, SXX(cpu.YR), cpu.XR) break;                    // SYA $ABS,X
	case 0x9E: ABX(WR_OP, SXX(cpu.XR), cpu.YR) break;                    // SXA $ABS,Y

	/* casi incerti */
	case 0x8B: IMP(RD_OP, XAA) break;                                    // XAA #IMM
	case 0x9F: ABX(WR_OP, AXA(_AXAABX(tmp)), cpu.YR) break;              // AXA $ABS,Y
	case 0x93: IDY(WR_OP, AXA(_AXAIDY(tmp))) break;                      // AXA ($IND),Y
	case 0xBB: ABX(RD_OP, _CYW(LAS), cpu.YR) break;                      // LAS $ABS,Y
	case 0x9B: ABX(WR_OP, XAS, cpu.YR) break;                            // XAS $ABS,Y
#endif

	case 0x02: // JAM
	case 0x12: // JAM
	case 0x22: // JAM
	case 0x32: // JAM
	case 0x42: // JAM
	case 0x52: // JAM
	case 0x62: // JAM
	case 0x72: // JAM
	case 0x92: // JAM
	case 0xB2: // JAM
	case 0xD2: // JAM
	case 0xF2: // JAM
	default:
		if (!info.no_rom && !info.first_illegal_opcode) {
			fprintf(stderr, "Alert: PC = 0x%04X, CODEOP = 0x%02X \n", (cpu.PC - 1), cpu.opcode);
			text_add_line_info(1, "[red]Illegal Opcode 0x%02X at 0x%04X", cpu.opcode, (cpu.PC - 1));
			info.first_illegal_opcode = TRUE;
		}
		cpu.cycles = 0;
		//info.stop = TRUE;
		//info.execute_cpu = FALSE;
		break;
	case 0x100: IMP(RD_OP, NMI) break;                                   // NMI
	case 0x200: IMP(RD_OP, IRQ(cpu.SR & 0xEF)) break;                    // IRQ
	}

	/* se presenti eseguo i restanti cicli di PPU e APU */
	if (cpu.cycles > 0) {
		tick_hw(cpu.cycles);
	}
}
void cpu_init_PC(void) {
	BYTE save_cheat_mode = cfg->cheat_mode;
	BYTE save_cpu_openbus = cpu.openbus;

	cfg->cheat_mode = NOCHEAT_MODE;
	cpu.PC = (cpu_rd_mem(INT_RESET + 1, FALSE) << 8 | cpu_rd_mem(INT_RESET, FALSE));
	cfg->cheat_mode = save_cheat_mode;
	cpu.openbus = save_cpu_openbus;
}
void cpu_turn_on(void) {
	if (info.reset >= HARD) {
		memset(&cpu, 0x00, sizeof(cpu));
		/* inizializzo lo Stack Pointer */
		cpu.SP = 0xFD;
		/*
		 * il bit 5 dell'SR e' sempre a 1 e
		 * il bit 2 (inhibit maskable interrupt) e'
		 * attivo all'avvio (o dopo un reset).
		 */
		cpu.SR = 0x34;

		tas.total_lag_frames = 0;

		if (tas.type && (tas.emulator == FCEUX)) {
			unsigned int x;

			for (x = 0; x < sizeof(mmcpu.ram); x++) {
				mmcpu.ram[x] = (x & 0x04) ? 0xFF : 0x00;
			}
		} else {
			/*
			 * reset della ram
			 * Note:
			 * All internal memory ($0000-$07FF) was consistently
			 * set to $ff except
			 *  $0008=$F7
			 *  $0009=$EF
			 *  $000a=$DF
			 *  $000f=$BF
			 *  Please note that you should NOT rely on the
			 *  state of any registers after Power-UP and especially
			 *  not the stack register and WRAM ($0000-$07FF).
			 */
			memset(mmcpu.ram, 0xFF, sizeof(mmcpu.ram));

			mmcpu.ram[0x008] = 0xF7;
			mmcpu.ram[0x009] = 0xEF;
			mmcpu.ram[0x00A] = 0xDF;
			//mmcpu.ram[0x00B] = 0xBF;
			mmcpu.ram[0x00F] = 0xBF;

			/*
			 * questo workaround serve solo per
			 * Dancing Blocks (Sachen) [!].nes
			 */
			if (info.mapper.id == 143) {
				mmcpu.ram[0x004] = 0x00;
			}

			/*
			 * questo workaround serve solo per
			 * Doraemon (J) (PRG0) [hM15].nes
			 */
			if (info.mapper.id == 15) {
				mmcpu.ram[0x018] = 0x00;
			}

			/*
			 * questo workaround serve solo per
			 * Ultimate Mortal Kombat 3 14 people (Unl)[!].nes
			 */
			if (info.mapper.id == 123) {
				mmcpu.ram[0x080] = 0x00;
			}
		}

		/* reset della PRG Ram */
		map_prg_ram_memset();
	} else {
		cpu.SP -= 0x03;
		cpu.SR |= 0x04;
		cpu.odd_cycle = 0;
		cpu.cycles = cpu.opcode_cycle = 0;
		cpu.double_rd = cpu.double_wr = 0;
	}
	memset(&nmi, 0x00, sizeof(nmi));
	memset(&irq, 0x00, sizeof(irq));
	/* di default attivo la lettura e la scrittura dalla PRG Ram */
	cpu.prg_ram_rd_active = cpu.prg_ram_wr_active = TRUE;
	/* disassemblo il Processor Status Register */
	disassemble_SR();
	/* setto il flag di disabilitazione dell'irq */
	irq.inhibit = cpu.im;
}
