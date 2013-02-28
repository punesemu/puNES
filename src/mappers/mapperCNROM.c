/*
 * mapperCNROM.c
 *
 *  Created on: 19/mag/2010
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "save_slot.h"

WORD chr_rom_8k_max;
BYTE type, mask, state;

void map_init_CNROM(BYTE model) {
	chr_rom_8k_max = info.chr_rom_8k_count - 1;

	EXTCL_CPU_WR_MEM(CNROM);

	mask = state = 0x00;

	if ((info.mapper_type >= CNROM_26CE27CE) && (info.mapper_type <= CNROM_26NCE27NCE)) {

		EXTCL_RD_CHR(CNROM);
		EXTCL_SAVE_MAPPER(CNROM);
		mapper.internal_struct[0] = (BYTE *) &cnrom_2627;
		mapper.internal_struct_size[0] = sizeof(cnrom_2627);

		memset(&cnrom_2627, 0x00, sizeof(cnrom_2627));
		mask = 0x03;

		switch (info.mapper_type) {
			case CNROM_26CE27CE:
				state = 0x03;
				break;
			case CNROM_26CE27NCE:
				state = 0x01;
				break;
			case CNROM_26NCE27CE:
				state = 0x02;
				break;
			case CNROM_26NCE27NCE:
				state = 0x00;
				break;
		}
	}

	type = model;
}
void extcl_cpu_wr_mem_CNROM(WORD address, BYTE value) {
	DBWORD bank;

	if (type == CNROM_CNFL) {
		/* bus conflict */
		value &= prg_rom_rd(address);
	}

	if (mask) {
		if ((value & mask) == state) {
			cnrom_2627.chr_rd_enable = FALSE;
		} else {
			cnrom_2627.chr_rd_enable = TRUE;
		}
		value &= ~mask;
	}

	control_bank(chr_rom_8k_max)
	bank = value << 13;

	chr.bank_1k[0] = &chr.data[bank];
	chr.bank_1k[1] = &chr.data[bank | 0x0400];
	chr.bank_1k[2] = &chr.data[bank | 0x0800];
	chr.bank_1k[3] = &chr.data[bank | 0x0C00];
	chr.bank_1k[4] = &chr.data[bank | 0x1000];
	chr.bank_1k[5] = &chr.data[bank | 0x1400];
	chr.bank_1k[6] = &chr.data[bank | 0x1800];
	chr.bank_1k[7] = &chr.data[bank | 0x1C00];
}
BYTE extcl_save_mapper_CNROM(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, cnrom_2627.chr_rd_enable);

	return (EXIT_OK);
}
BYTE extcl_rd_chr_CNROM(WORD address) {
	if (cnrom_2627.chr_rd_enable == TRUE) {
		return (0xFF);
	}

	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
