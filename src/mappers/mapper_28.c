/*
 * mapper_28.c
 *
 *  Created on: 29/dic/2013
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

static void INLINE nmt_setup_28(void);
static void INLINE prg_setup_28(void);

BYTE static const inner_and[4] = { 0x01, 0x03, 0x07, 0x0F };
BYTE static const outer_and[4] = { 0x7E, 0x7C, 0x78, 0x70 };

void map_init_28(void) {
	EXTCL_CPU_WR_MEM(28);
	EXTCL_CPU_RD_MEM(28);
	EXTCL_SAVE_MAPPER(28);
	mapper.internal_struct[0] = (BYTE *) &m28;
	mapper.internal_struct_size[0] = sizeof(m28);

	if (info.reset >= HARD) {
		memset(&m28, 0x00, sizeof(m28));
		m28.prg[2] = 0x3F;
		m28.prg[0] = 0x0F;
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_28(WORD address, BYTE value) {
	if (address < 0x5000) {
		return;
	}
	if (address < 0x6000) {
		m28.index = ((value & 0x80) >> 6) | (value & 0x01);
		return;
	}
	if (address < 0x8000) {
		return;
	}
	switch (m28.index) {
		case 0: {
			DBWORD bank;

			if (!(m28.mirroring & 0x02)) {
				m28.mirroring = (m28.mirroring & 0x02) | ((value & 0x10) >> 4);
				nmt_setup_28();
			}

			value &= 0x03;
			control_bank(info.chr.rom.max.banks_8k)
			bank = value << 13;
			chr.bank_1k[0] = &chr.data[bank];
			chr.bank_1k[1] = &chr.data[bank | 0x0400];
			chr.bank_1k[2] = &chr.data[bank | 0x0800];
			chr.bank_1k[3] = &chr.data[bank | 0x0C00];
			chr.bank_1k[4] = &chr.data[bank | 0x1000];
			chr.bank_1k[5] = &chr.data[bank | 0x1400];
			chr.bank_1k[6] = &chr.data[bank | 0x1800];
			chr.bank_1k[7] = &chr.data[bank | 0x1C00];
			return;
		}
		case 1:
			m28.prg[0] = value & 0x0F;
			prg_setup_28();

			if (!(m28.mirroring & 0x02)) {
				m28.mirroring = (m28.mirroring & 0x02) | ((value & 0x10) >> 4);
				nmt_setup_28();
			}
			return;
		case 2:
			m28.prg[1] = value & 0x3C;
			prg_setup_28();

			m28.mirroring = value & 0x03;
			nmt_setup_28();
			return;
		case 3:
			m28.prg[2] = value & 0x3F;
			prg_setup_28();
			return;
	}
}
BYTE extcl_cpu_rd_mem_28(WORD address, BYTE openbus, BYTE before) {
	if ((address > 0x4FFF) && (address < 0x6000)) {
		return (before);
	}
	return (openbus);
}
BYTE extcl_save_mapper_28(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m28.index);
	save_slot_ele(mode, slot, m28.mirroring);
	save_slot_ele(mode, slot, m28.prg);

	return (EXIT_OK);
}

static void INLINE nmt_setup_28(void) {
	switch (m28.mirroring) {
		case 0:
			mirroring_SCR0();
			break;
		case 1:
			mirroring_SCR1();
			break;
		case 2:
			mirroring_V();
			break;
		case 3:
			mirroring_H();
			break;
	}
}
static void INLINE prg_setup_28(void) {
	BYTE i = (m28.prg[1] & 0x30) >> 4, value;

	if (!(m28.prg[1] & 0x08)) {
		value = (m28.prg[0] & (inner_and[i] >> 1)) | ((m28.prg[2] << 1) & outer_and[i]);
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 0, value);
		value |= 1;
		control_bank(info.prg.rom.max.banks_16k)
		map_prg_rom_8k(2, 2, value);
	} else {
		value = (m28.prg[0] & inner_and[i]) | ((m28.prg[2] << 1) & outer_and[i]);

		if (!(m28.prg[1] & 0x04)) {
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);

			value = (m28.prg[2] << 1);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
		} else {
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);

			value = (m28.prg[2] << 1) | 1;
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
		}
	}
	map_prg_rom_8k_update();
}
