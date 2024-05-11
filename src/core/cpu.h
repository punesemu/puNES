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

#ifndef CPU_H_
#define CPU_H_

#include "common.h"
#include "input.h"

enum cpu_misc { STACK = 0x0100 };
enum interrupt_types {
	INT_NMI = 0xFFFA,
	INT_RESET = 0xFFFC,
	INT_IRQ = 0xFFFE,
	OP_NMI = 0x100,
	OP_IRQ = 0x200,
	APU_IRQ = 0x01,
	DMC_IRQ = 0x02,
	EXT_IRQ = 0x04,
	FDS_TIMER_IRQ = 0x08,
	FDS_DISK_IRQ = 0x10
};

#define assemble_SR(idx)\
	nes[idx].c.cpu.SR = (nes[idx].c.cpu.sf | nes[idx].c.cpu.of | 0x20 | nes[idx].c.cpu.bf | nes[idx].c.cpu.df | nes[idx].c.cpu.im | nes[idx].c.cpu.zf | nes[idx].c.cpu.cf)
#define disassemble_SR(idx)\
	nes[idx].c.cpu.cf = nes[idx].c.cpu.SR & 0x01;\
	nes[idx].c.cpu.zf = nes[idx].c.cpu.SR & 0x02;\
	nes[idx].c.cpu.im = nes[idx].c.cpu.SR & 0x04;\
	nes[idx].c.cpu.df = nes[idx].c.cpu.SR & 0x08;\
	nes[idx].c.cpu.bf = nes[idx].c.cpu.SR & 0x10;\
	nes[idx].c.cpu.of = nes[idx].c.cpu.SR & 0x40;\
	nes[idx].c.cpu.sf = nes[idx].c.cpu.SR & 0x80

typedef struct _cpu {
	// Processor Registers
	union _cpu_pc {
		BYTE b[2];
		WORD w;
	} PC;
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
	// il codice che identifica l'istruzione
	WORD opcode;
	WORD opcode_PC;
	// il flag che indica se il ciclo della cpu e' dispari
	BYTE odd_cycle;
	// cicli cpu dell'istruzione e delle operazioni di lettura e scrittura
	SWORD cycles;
	// DMC
	WORD opcode_cycle;
	// doppia lettura
	BYTE double_rd;
	// doppia scrittura
	BYTE double_wr;
	// lettura PRG Ram attiva/disattiva
	BYTE prg_ram_rd_active;
	// scrittura PRG Ram attiva/disattiva
	BYTE prg_ram_wr_active;
	// i cicli (senza aggiustamenti) impiegati dall'opcode
	WORD base_opcode_cycles;
	// buffer di lettura
	BYTE openbus;
	//Set to true by KIL/HLT instruction
	BYTE jammed;
} _cpu;
typedef struct _irq {
	BYTE high;
	BYTE delay;
	BYTE before;
	BYTE inhibit;
} _irq;
typedef struct _nmi {
	BYTE high;
	BYTE delay;
	BYTE before;
	BYTE inhibit;
	WORD frame_x;
	// i cicli passati dall'inizio dell'NMI
	uint32_t cpu_cycles_from_last_nmi;
} _nmi;
typedef struct _cpu_input {
	BYTE r4016;
	BYTE pindex[PORT_MAX];
	BYTE fsindex[PORT_BASE];
} _cpu_input;
typedef struct _cpu_data {
	_cpu cpu;
	_irq irq;
	_nmi nmi;
	_cpu_input input;
} _cpu_data;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void cpu_exe_op(BYTE nidx);
EXTERNC void cpu_initial_cycles(BYTE nidx);
EXTERNC void cpu_turn_on(BYTE nidx);
EXTERNC BYTE cpu_rd_mem_dbg(BYTE nidx, WORD address);
EXTERNC BYTE cpu_rd_mem(BYTE nidx, WORD address, BYTE made_tick);
EXTERNC void cpu_wr_mem(BYTE nidx, WORD address, BYTE value);
EXTERNC void apu_wr_mem_mapper(BYTE nidx, WORD address, BYTE value);

#undef EXTERNC

#endif /* CPU_H_ */
