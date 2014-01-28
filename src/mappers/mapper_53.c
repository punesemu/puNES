/*
 * mapper_53.c
 *
 *  Created on: 21/apr/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

BYTE *prg_6000;

void map_init_53(void) {
	EXTCL_CPU_WR_MEM(53);
	EXTCL_CPU_RD_MEM(53);
	EXTCL_SAVE_MAPPER(53);
	mapper.internal_struct[0] = (BYTE *) &m53;
	mapper.internal_struct_size[0] = sizeof(m53);

	if (info.reset >= HARD) {
		memset(&m53, 0x00, sizeof(m53));

		extcl_cpu_wr_mem_53(0x6000, 0x00);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_53(WORD address, BYTE value) {
	BYTE tmp;

	if (address < 0x6000) {
		return;
	}

	if (address >= 0x8000) {
		m53.reg[1] = value;
	} else {
		m53.reg[0] = value;

		if (m53.reg[0] & 0x20) {
			mirroring_H();
		} else  {
			mirroring_V();
		}
	}

	tmp = (m53.reg[0] << 3) & 0x78;

	m53.prg_6000 = ((tmp << 1) | 0x0F) + 4;
	_control_bank(m53.prg_6000, info.prg.rom.max.banks_8k)
	prg_6000 = &prg.rom[m53.prg_6000 << 13];

	value = (m53.reg[0] & 0x10) ? (tmp | (m53.reg[1] & 0x07)) + 2 : 0;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 0, value);

	value = (m53.reg[0] & 0x10) ? (tmp | (0xFF & 0x07)) + 2 : 1;
	control_bank(info.prg.rom.max.banks_16k)
	map_prg_rom_8k(2, 2, value);

	map_prg_rom_8k_update();
}
BYTE extcl_cpu_rd_mem_53(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg_6000[address & 0x1FFF]);
}
BYTE extcl_save_mapper_53(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m53.reg);
	save_slot_ele(mode, slot, m53.prg_6000);

	if (mode == SAVE_SLOT_READ) {
		prg_6000 = &prg.rom[m53.prg_6000 << 13];
	}

	return (EXIT_OK);
}
