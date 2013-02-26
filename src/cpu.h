/*
 * cpu.h
 *
 *  Created on: 14/gen/2011
 *      Author: fhorse
 */

#ifndef CPU_H_
#define CPU_H_

#include "common.h"

enum cpu_misc { STACK = 0x0100 };
enum interrupt_types {
	INT_NMI = 0xFFFA,
	INT_RESET = 0xFFFC,
	INT_IRQ = 0xFFFE,
	APU_IRQ = 0x01,
	DMC_IRQ =  0x02,
	EXT_IRQ = 0x04,
	FDS_TIMER_IRQ = 0x08,
	FDS_DISK_IRQ = 0x10
};

#define disassemble_SR()\
	cpu.cf = cpu.SR & 0x01;\
	cpu.zf = cpu.SR & 0x02;\
	cpu.im = cpu.SR & 0x04;\
	cpu.df = cpu.SR & 0x08;\
	cpu.bf = cpu.SR & 0x10;\
	cpu.of = cpu.SR & 0x40;\
	cpu.sf = cpu.SR & 0x80
#define init_PC()\
	/* valorizzo il PC con l'indirizzo iniziale */\
	if (fds.info.enabled) {\
		cpu.PC = (prg.rom[(INT_RESET + 1) & 0x1FFF] << 8) |\
				prg.rom[INT_RESET & 0x1FFF];\
	} else {\
		cpu.PC = (prg.rom_8k[((INT_RESET + 1) >> 13) & 0x03][(INT_RESET + 1) & 0x1FFF] << 8)\
				| prg.rom_8k[(INT_RESET >> 13) & 0x03][INT_RESET & 0x1FFF];\
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
	WORD opcode;
	WORD opcode_PC;
	/* il flag che indica se il ciclo della cpu e' dispari */
	BYTE odd_cycle;
	/* buffer di lettura */
	BYTE openbus;
	/*
	 * cicli cpu dell'istruzione e delle
	 * operazioni di lettura e scrittura.
	 */
	SWORD cycles;
	/* DMC */
	WORD opcode_cycle;
	/* doppia lettura */
	BYTE double_rd;
	/* doppia scrittura */
	BYTE double_wr;
	/* lettura PRG Ram attiva/disattiva */
	BYTE prg_ram_rd_active;
	/* scrittura PRG Ram attiva/disattiva */
	BYTE prg_ram_wr_active;
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
	WORD frame_x;
} _nmi;

_cpu cpu;
_irq irq;
_nmi nmi;

void cpu_exe_op(void);
void cpu_turn_on(void);

#endif /* CPU_H_ */
