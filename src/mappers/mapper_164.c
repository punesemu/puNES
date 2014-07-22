/*
 * mapper_164.c
 *
 *  Created on: 6/ott/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "ppu.h"
#include "save_slot.h"

void map_init_164(void) {
	EXTCL_CPU_WR_MEM(164);
	EXTCL_CPU_RD_MEM(164);
	EXTCL_SAVE_MAPPER(164);
	mapper.internal_struct[0] = (BYTE *) &m164;
	mapper.internal_struct_size[0] = sizeof(m164);

	memset(&m164, 0x00, sizeof(m164));
	m164.prg = 0x0F;

	{
		BYTE value = m164.prg;

		control_bank(info.prg.rom.max.banks_32k)
		map_prg_rom_8k(4, 0, value);
	}

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_164(WORD address, BYTE value) {
	switch (address & 0x7300) {
		case 0x5000:
			m164.prg = (m164.prg & 0xF0) | (value & 0x0F);
			value = m164.prg;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x5100:
			m164.prg = (m164.prg & 0x0F) | (value << 4);
			value = m164.prg;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		/*
		case 0x5200:
			return;
		case 0x5300:
			value = m164.prg;
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			map_prg_rom_8k_update();
			return;
		*/
	}
}
BYTE extcl_cpu_rd_mem_164(WORD address, BYTE openbus, BYTE before) {
	if ((address > 0x4FFF) && (address < 0x6000)) {
		return (before);
	}
	return (openbus);
}
BYTE extcl_save_mapper_164(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m164.prg);

	return (EXIT_OK);
}
