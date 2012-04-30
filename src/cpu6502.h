/*
 * cpu6502.h
 *
 *  Created on: 14/gen/2011
 *      Author: fhorse
 */

#ifndef CPU6502_H_
#define CPU6502_H_

#include "common.h"

#define INT_NMI					0xFFFA
#define INT_RESET				0xFFFC
#define INT_IRQ					0xFFFE

#define STACK					0x0100

#define APUIRQ					0x01
#define DMCIRQ					0x02
#define EXTIRQ					0x04
#define FDSTIMERIRQ				0x08
#define FDSDISKIRQ				0x10

#define DIS_SR\
	cpu.cf = cpu.SR & 0x01;\
	cpu.zf = cpu.SR & 0x02;\
	cpu.im = cpu.SR & 0x04;\
	cpu.df = cpu.SR & 0x08;\
	cpu.bf = cpu.SR & 0x10;\
	cpu.of = cpu.SR & 0x40;\
	cpu.sf = cpu.SR & 0x80
#define INIT_PC\
	/* valorizzo il PC con l'indirizzo iniziale */\
	if (fds.info.enabled) {\
		cpu.PC = (prg.rom[(INT_RESET + 1) & 0x1FFF] << 8) |\
				prg.rom[INT_RESET & 0x1FFF];\
	} else {\
		cpu.PC = (prg.rom8k[((INT_RESET + 1) >> 13) & 0x03][(INT_RESET + 1) & 0x1FFF] << 8)\
				| prg.rom8k[(INT_RESET >> 13) & 0x03][INT_RESET & 0x1FFF];\
	}

typedef struct {
	/* Processor Registers */
	WORD PC; // Program Counter
	BYTE SP; // Stack Pointer
	BYTE AR; // Accumulator
	BYTE XR; // Index Register X
	BYTE YR; // Index Register Y
	/* Processor Status Register */
	BYTE SR; // Status Register
	BYTE cf; // C (bit 0) - Carry flag
	BYTE zf; // Z (bit 1) - Zero flag
	BYTE im; // I (bit 2) - Interrupt mask
	BYTE df; // D (bit 3) - Decimal flag
	BYTE bf; // B (bit 4) - Break flag
	/*            (bit 5) - Always 1 */
	BYTE of; // O (bit 6) - Overflow flag
	BYTE sf; // S (bit 7) - Sign flag or N - Negative flag
	/* il codice che identifica l'istruzione */
	WORD codeop;
	WORD codeopPC;
	/* il flag che indica se il ciclo della cpu e' dispari */
	BYTE oddCycle;
	/* buffer di lettura */
	BYTE openbus;
	/*
	 * cicli cpu dell'istruzione e delle
	 * operazioni di lettura e scrittura.
	 */
	SWORD cycles;
	/* DMC */
	WORD opCycle;
	/* doppia lettura */
	BYTE dblRd;
	/* doppia scrittura */
	BYTE dblWr;
	/* lettura PRG Ram attiva/disattiva */
	BYTE prgRamRdActive;
	/* scrittura PRG Ram attiva/disattiva */
	BYTE prgRamWrActive;
} _cpu;
typedef struct {
	BYTE high;
	BYTE delay;
	BYTE before;
	BYTE inhibit;
} _irq;
typedef struct {
	BYTE high;
	BYTE delay;
	BYTE before;
	BYTE inhibit;
	WORD frameX;
} _nmi;

_cpu cpu;
_irq irq;
_nmi nmi;

void cpuExeOP(void);
void cpuInterrupt(void);
void cpuTurnON(void);

#endif /* CPU6502_H_ */
