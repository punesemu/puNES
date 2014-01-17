/*
 * mapper_183.c
 *
 *  Created on: 9/ott/2011
 *      Author: fhorse
 */

#include <stdlib.h>
#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "cpu.h"
#include "save_slot.h"

WORD prg_rom_8k_max, chr_rom_1k_max;

void map_init_183(void) {
	prg_rom_8k_max = info.prg.rom.banks_8k - 1;
	chr_rom_1k_max = info.chr.rom.banks_1k - 1;

	EXTCL_CPU_WR_MEM(183);
	EXTCL_CPU_RD_MEM(183);
	EXTCL_SAVE_MAPPER(183);
	EXTCL_CPU_EVERY_CYCLE(183);
	mapper.internal_struct[0] = (BYTE *) &m183;
	mapper.internal_struct_size[0] = sizeof(m183);

	if (info.reset >= HARD) {
		memset(&m183, 0x00, sizeof(m183));
	}

	{
		BYTE i;
		for (i = 0; i < LENGTH(m183.chr_rom_bank); i++) {
			m183.chr_rom_bank[i] = i;
		}
	}
}
void extcl_cpu_wr_mem_183(WORD address, BYTE value) {
	switch (address & 0xF80F) {
		case 0x8800:
		case 0x8801:
		case 0x8802:
		case 0x8803:
			control_bank(prg_rom_8k_max)
			map_prg_rom_8k(1, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x9800:
		case 0x9801:
		case 0x9802:
		case 0x9803: {
			switch (value & 0x03) {
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
			return;
		}
		case 0xA000:
		case 0xA001:
		case 0xA002:
		case 0xA003:
			control_bank(prg_rom_8k_max)
			map_prg_rom_8k(1, 2, value);
			map_prg_rom_8k_update();
			return;
		case 0xA800:
		case 0xA801:
		case 0xA802:
		case 0xA803:
			control_bank(prg_rom_8k_max)
			map_prg_rom_8k(1, 1, value);
			map_prg_rom_8k_update();
			return;
		case 0xF000:
		case 0xF001:
		case 0xF002:
		case 0xF003:
		case 0xF800:
		case 0xF801:
		case 0xF802:
		case 0xF803:
			m183.count = (m183.count & 0xF0) | (value & 0x0F);
			return;
		case 0xF004:
		case 0xF005:
		case 0xF006:
		case 0xF007:
		case 0xF804:
		case 0xF805:
		case 0xF806:
		case 0xF807:
			m183.count = (value << 4) | (m183.count & 0x0F);
			return;
		case 0xF008:
		case 0xF009:
		case 0xF00A:
		case 0xF00B:
		case 0xF808:
		case 0xF809:
		case 0xF80A:
		case 0xF80B:
			m183.enabled = value;
			if (!m183.enabled) {
				m183.prescaler = 0;
				irq.high &= ~EXT_IRQ;
			}
			return;
		default:
			if ((address > 0xAFFF) && (address < 0xF000)) {
				const BYTE shift = address & 0x04;
				const BYTE slot = (((address - 0x3000) >> 1 | (address << 7)) & 0x1C00) >> 10;

				value = (m183.chr_rom_bank[slot] & (0xF0 >> shift)) | ((value & 0x0F) << shift);
				control_bank(chr_rom_1k_max)
				chr.bank_1k[slot] = &chr.data[value << 10];
				m183.chr_rom_bank[slot] = value;
			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_183(WORD address, BYTE openbus, BYTE before) {
	if ((address < 0x6000) || (address > 0x7FFF)) {
		return (openbus);
	}

	return (prg.rom[address & 0x1FFF]);
}
BYTE extcl_save_mapper_183(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m183.enabled);
	save_slot_ele(mode, slot, m183.prescaler);
	save_slot_ele(mode, slot, m183.count);
	save_slot_ele(mode, slot, m183.delay);
	save_slot_ele(mode, slot, m183.chr_rom_bank);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_183(void) {
	if (m183.delay && !(--m183.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (++m183.prescaler < 114) {
		return;
	}

	m183.prescaler = 0;

	if (m183.enabled && !(++m183.count)) {
		m183.delay = 1;
	}
}
