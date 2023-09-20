/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include "nes.h"

enum cpu_misc { STACK = 0x0100 };
enum interrupt_types {
	INT_NMI = 0xFFFA,
	INT_RESET = 0xFFFC,
	INT_IRQ = 0xFFFE,
	APU_IRQ = 0x01,
	DMC_IRQ = 0x02,
	EXT_IRQ = 0x04,
	FDS_TIMER_IRQ = 0x08,
	FDS_DISK_IRQ = 0x10
};

#define assemble_SR()\
	nes.c.cpu.SR = (nes.c.cpu.sf | nes.c.cpu.of | 0x20 | nes.c.cpu.bf | nes.c.cpu.df | nes.c.cpu.im | nes.c.cpu.zf | nes.c.cpu.cf)
#define disassemble_SR()\
	nes.c.cpu.cf = nes.c.cpu.SR & 0x01;\
	nes.c.cpu.zf = nes.c.cpu.SR & 0x02;\
	nes.c.cpu.im = nes.c.cpu.SR & 0x04;\
	nes.c.cpu.df = nes.c.cpu.SR & 0x08;\
	nes.c.cpu.bf = nes.c.cpu.SR & 0x10;\
	nes.c.cpu.of = nes.c.cpu.SR & 0x40;\
	nes.c.cpu.sf = nes.c.cpu.SR & 0x80

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void cpu_exe_op(void);
EXTERNC void cpu_initial_cycles(void);
EXTERNC void cpu_turn_on(void);
EXTERNC BYTE cpu_rd_mem_dbg(WORD address);
EXTERNC BYTE cpu_rd_mem(WORD address, BYTE made_tick);
EXTERNC void cpu_wr_mem(WORD address, BYTE value);
EXTERNC void apu_wr_mem_mapper(WORD address, BYTE value);

#undef EXTERNC

#endif /* CPU_H_ */
