/*
 * mapper_219.c
 *
 *  Created on: 20/mar/2012
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "irqA12.h"
#include "save_slot.h"

#define m219_chr_1k(a, b)\
	value = m219.reg[2] | ((save >> 1) & a);\
	control_bank(info.chr.rom.max.banks_1k)\
	chr.bank_1k[b] = chr_chip_byte_pnt(0, value << 10)

void map_init_219(void) {
	EXTCL_CPU_WR_MEM(219);
	EXTCL_SAVE_MAPPER(219);
	EXTCL_CPU_EVERY_CYCLE(MMC3);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *) &m219;
	mapper.internal_struct_size[0] = sizeof(m219);

	if (info.reset >= HARD) {
		memset(&m219, 0x00, sizeof(m219));
	}

	memset(&irqA12, 0x00, sizeof(irqA12));

	irqA12.present = TRUE;
	irqA12_delay = 1;
}
void extcl_cpu_wr_mem_219(WORD address, BYTE value) {
	if (address >= 0xA000) {
		extcl_cpu_wr_mem_MMC3(address, value);
		return;
	}

	/* intercetto cio' che mi interessa */
	switch (address & 0x03) {
		case 0:
			m219.reg[0] = 0;
			m219.reg[1] = value;
			return;
		case 1: {
			BYTE save = value;

			value = m219.reg[0] - 0x23;

			if (value < 4) {
				value ^= 0x03;
				control_bank(info.prg.rom.max.banks_8k)
				map_prg_rom_8k(2, 0, value);

				value = ((value >> 5) & 0x01) | ((value >> 3) & 0x02) | ((value >> 1) & 0x04)
				        		| ((value << 1) & 0x08);
				control_bank(info.prg.rom.max.banks_8k)
				map_prg_rom_8k(2, 2, value);

				map_prg_rom_8k_update();
			}

			switch (m219.reg[1]) {
				case 0x08:
				case 0x0A:
				case 0x0E:
				case 0x12:
				case 0x16:
				case 0x1A:
				case 0x1E:
					m219.reg[2] = save << 4;
					return;
				case 0x09:
					m219_chr_1k(0x0E, 0);
					return;
				case 0x0B:
					m219_chr_1k(0x01, 1);
					return;
				case 0x0C:
				case 0x0D:
					m219_chr_1k(0x0E, 2);
					return;
				case 0x0F:
					m219_chr_1k(0x01, 3);
					return;
				case 0x10:
				case 0x11:
					m219_chr_1k(0x0F, 4);
					return;
				case 0x14:
				case 0x15:
					m219_chr_1k(0x0F, 5);
					return;
				case 0x18:
				case 0x19:
					m219_chr_1k(0x0F, 6);
					return;
				case 0x1C:
				case 0x1D:
					m219_chr_1k(0x0F, 7);
					return;
			}

			return;
		}
		case 2:
			m219.reg[0] = value;
			m219.reg[1] = 0;
			return;
	}
}
BYTE extcl_save_mapper_219(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m219.reg);

	return (EXIT_OK);
}
