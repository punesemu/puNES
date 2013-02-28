/*
 * mapper221.c
 *
 *  Created on: 14/feb/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

#define m221_prg_16k_swap()\
	value = ((m221.reg[0] >> 1) & 0x38) | ((m221.reg[0] & 0x01) ? (m221.reg[0] & 0x80) ?\
		m221.reg[1] : (m221.reg[1] & 0x06) : m221.reg[1]);\
   	control_bank(prg_rom_16k_max)\
	map_prg_rom_8k(2, 0, value);\
	value = ((m221.reg[0] >> 1) & 0x38) | ((m221.reg[0] & 0x01) ? (m221.reg[0] & 0x80) ?\
		0x07 : (m221.reg[1] & 0x06) | 0x1 : m221.reg[1]);\
   	control_bank(prg_rom_16k_max)\
	map_prg_rom_8k(2, 2, value)

WORD prg_rom_16k_max;

void map_init_221(void) {
	prg_rom_16k_max = info.prg_rom_16k_count - 1;

	EXTCL_CPU_WR_MEM(221);
	EXTCL_SAVE_MAPPER(221);
	mapper.internal_struct[0] = (BYTE *) &m221;
	mapper.internal_struct_size[0] = sizeof(m221);

	if (info.reset >= HARD) {
		memset(&m221, 0x00, sizeof(m221));
		{
			BYTE value;
			m221_prg_16k_swap();
		}
	}
}
void extcl_cpu_wr_mem_221(WORD address, BYTE value) {
	BYTE reg;

	switch (address & 0xF000) {
		case 0x8000:
		case 0x9000:
		case 0xA000:
		case 0xB000:
			if (address & 0x0001) {
				mirroring_H();
			} else {
				mirroring_V();
			}

			if (m221.reg[0] == (reg = (address >> 1) & 0xFF)) {
				return;
			}
			m221.reg[0] = reg;
			m221_prg_16k_swap();
			break;
		case 0xC000:
		case 0xD000:
		case 0xE000:
		case 0xF000:
			if (m221.reg[1] == (reg = address & 0x07)) {
				return;
			}
			m221.reg[1] = reg;
			m221_prg_16k_swap();
			break;
	}
	map_prg_rom_8k_update();
}
BYTE extcl_save_mapper_221(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m221.reg);

	return (EXIT_OK);
}
