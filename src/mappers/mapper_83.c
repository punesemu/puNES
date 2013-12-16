/*
 * mapper_83.c
 *
 *  Created on: 15/dic/2013
 *      Author: fhorse
 */

#include <stdio.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

static void INLINE sync_83(void);

WORD prg_rom_16k_max, prg_rom_8k_max;
WORD chr_rom_2k_max, chr_rom_1k_max;

void map_init_83(void) {
	prg_rom_16k_max = info.prg_rom_16k_count - 1;
	prg_rom_8k_max = info.prg_rom_8k_count - 1;
	chr_rom_2k_max = (info.chr_rom_1k_count >> 1) - 1;
	chr_rom_1k_max = info.chr_rom_1k_count - 1;

	EXTCL_CPU_WR_MEM(83);
	EXTCL_CPU_RD_MEM(83);
	EXTCL_SAVE_MAPPER(83);
	EXTCL_CPU_EVERY_CYCLE(83);

	mapper.internal_struct[0] = (BYTE *) &m83;
	mapper.internal_struct_size[0] = sizeof(m83);

	if (info.reset >= HARD) {
		memset(&m83, 0x00, sizeof(m83));
	}

	sync_83();

	switch(info.id) {
		case MAP83_REG0:
			m83.dip = 0;
			break;
		case MAP83_DGP:
			m83.dip = 0xFF;
			info.prg_ram_plus_8k_count = 1;
			break;
		default:
			m83.dip = 0xFF;
			break;
	}

	info.mapper_extend_wr = TRUE;
}
void extcl_cpu_wr_mem_83(WORD address, BYTE value) {
	switch (address) {
		case 0x5100:
		case 0x5101:
		case 0x5102:
		case 0x5103:
			m83.low[address & 0x0003] = value;
			break;
		case 0x8000:
			m83.is2kbank = TRUE;
			m83.bank = value;
			m83.mode |= 0x40;
			sync_83();
			break;
		case 0xB000:
		case 0xB0FF:
		case 0xB1FF:
			m83.bank = value;
			m83.mode |= 0x40;
			sync_83();
			break;
		case 0x8100:
			m83.mode = value | (m83.mode & 0x40);
			sync_83();
			break;
		case 0x8200:
			m83.irq.count &= 0xFF00;
			m83.irq.count |= value;
			irq.high &= ~EXT_IRQ;
			break;
		case 0x8201:
			m83.irq.active = m83.mode & 0x80;
			m83.irq.count &= 0x00FF;
			m83.irq.count |= (value << 8);
			break;
		case 0x8300:
		case 0x8301:
		case 0x8302:
			m83.reg[(address & 0x03) | 0x08] = value;
			m83.mode &= 0xBF;
			sync_83();
			break;
		case 0x8312:
		case 0x8313:
		case 0x8314:
		case 0x8315:
			m83.reg[address & 0x0007] = value;
			m83.isnot2kbank = TRUE;
			sync_83();
			break;
		case 0x8310:
		case 0x8311:
		case 0x8316:
		case 0x8317:
			m83.reg[address & 0x0007] = value;
			sync_83();
			break;
	}
}
BYTE extcl_cpu_rd_mem_83(WORD address, BYTE openbus, BYTE before) {
	if (address == 0x5000) {
		return (m83.dip);
	}
	if ((address >= 0x5100) && (address <= 0x5103)) {
		return (m83.low[address & 0x0003]);
	}
	if (address <= 0x5FFF) {
		return (before);
	}
	/* nestopia */
	if ((address <= 0x7FFF) && !info.prg_ram_plus_8k_count) {
		return ((m83.mode & 0x20) ? openbus : (address >> 8));
	}
	/*          */
	return (openbus);
}
BYTE extcl_save_mapper_83(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m83.is2kbank);
	save_slot_ele(mode, slot, m83.isnot2kbank);
	save_slot_ele(mode, slot, m83.mode);
	save_slot_ele(mode, slot, m83.bank);
	save_slot_ele(mode, slot, m83.dip);
	save_slot_ele(mode, slot, m83.low);
	save_slot_ele(mode, slot, m83.reg);
	save_slot_ele(mode, slot, m83.irq.active);
	save_slot_ele(mode, slot, m83.irq.count);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_83(void) {
	if (m83.irq.active) {
		if (--m83.irq.count == 0) {
			irq.high |= EXT_IRQ;
			m83.irq.active = 0;
			m83.irq.count = 0xFFFF;
		}
	}
}

static void INLINE sync_83(void) {
	WORD value;

	switch (m83.mode & 0x03) {
		case 0:
			mirroring_V();
			break;
		case 1:
			mirroring_H();
			break;
		case 2:
			mirroring_SCR0();
			break;
		case 3:
			mirroring_SCR1();
			break;
	}

	if (m83.is2kbank && !m83.isnot2kbank) {
		SDBWORD bank;

		value = m83.reg[0];
		control_bank(chr_rom_2k_max)
		bank = value << 11;
		chr.bank_1k[0] = &chr.data[bank];
		chr.bank_1k[1] = &chr.data[bank | 0x0400];
		value = m83.reg[1];
		control_bank(chr_rom_2k_max)
		bank = value << 11;
		chr.bank_1k[2] = &chr.data[bank];
		chr.bank_1k[3] = &chr.data[bank | 0x0400];
		value = m83.reg[6];
		control_bank(chr_rom_2k_max)
		bank = value << 11;
		chr.bank_1k[4] = &chr.data[bank];
		chr.bank_1k[5] = &chr.data[bank | 0x0400];
		value = m83.reg[7];
		control_bank(chr_rom_2k_max)
		bank = value << 11;
		chr.bank_1k[6] = &chr.data[bank];
		chr.bank_1k[7] = &chr.data[bank | 0x0400];
	} else {
		BYTE i;

		for (i = 0; i < 8; i++) {
			value = ((m83.bank << 4) & 0x0300) | m83.reg[i];
			control_bank(chr_rom_1k_max)
			chr.bank_1k[i] = &chr.data[value << 10];
		}
	}
	if (m83.mode & 0x40) {
		value = (m83.bank & 0x3F);
		control_bank(prg_rom_16k_max)
		map_prg_rom_8k(2, 0, value);
		value = (m83.bank & 0x30) | 0x0F;
		control_bank(prg_rom_16k_max)
		map_prg_rom_8k(2, 2, value);
	} else {
		value = m83.reg[8];
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, 0, value);
		value = m83.reg[9];
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, 1, value);
		value = m83.reg[10];
		control_bank(prg_rom_8k_max)
		map_prg_rom_8k(1, 2, value);
		map_prg_rom_8k(1, 3, prg_rom_8k_max);
	}
	map_prg_rom_8k_update();
}
