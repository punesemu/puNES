/*
 * mapper_90_209_211.c
 *
 *  Created on: 11/dic/2013
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"
#include "ppu_inline.h"

static void INLINE prg_setup_90_209_211(void);
static void INLINE chr_setup_90_209_211(void);
static void INLINE nmt_setup_90_209_211(void);
static void INLINE irq_clock_prescaler_90_209_211(void);
static void INLINE irq_clock_count_90_209_211(void);

#define chr_90_209_211(index)\
	((m90_209_211.chr.low[index] | (m90_209_211.chr.high[index] << 8)) & mask) | base
#define nmt_rom_90_209_211(index)\
	value = m90_209_211.nmt.reg[index];\
	control_bank(info.chr.rom.max.banks_1k)\
	ntbl.bank_1k[index] = chr_chip_byte_pnt(0, value << 10);\
	m90_209_211.nmt.write[index] = FALSE
#define nmt_ram_90_209_211(index)\
	value = m90_209_211.nmt.reg[index] & 0x01;\
	ntbl.bank_1k[index] = &ntbl.data[value << 10];\
	m90_209_211.nmt.write[index] = TRUE

void map_init_90_209_211(BYTE model) {
	BYTE i;

	EXTCL_CPU_WR_MEM(90_209_211);
	EXTCL_CPU_RD_MEM(90_209_211);
	EXTCL_SAVE_MAPPER(90_209_211);
	EXTCL_CPU_EVERY_CYCLE(90_209_211);
	EXTCL_RD_PPU(90_209_211);
	EXTCL_RD_CHR(90_209_211);
	EXTCL_WR_NMT(90_209_211);
	EXTCL_PPU_000_TO_255(90_209_211);
	EXTCL_PPU_256_TO_319(90_209_211);
	EXTCL_PPU_320_TO_34X(90_209_211);
	EXTCL_UPDATE_R2006(90_209_211);

	mapper.internal_struct[0] = (BYTE *) &m90_209_211;
	mapper.internal_struct_size[0] = sizeof(m90_209_211);

	if (info.reset >= HARD) {
		memset(&m90_209_211, 0x00, sizeof(m90_209_211));

		m90_209_211.model = model;

		m90_209_211.mul[0] = 0xFF;
		m90_209_211.mul[1] = 0xFF;
		m90_209_211.single_byte_ram = 0xFF;

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				m90_209_211.prg[i] = 0xFF;
				m90_209_211.nmt.write[i] = TRUE;
			}
			m90_209_211.chr.low[i] = 0xFF;
			m90_209_211.chr.high[i] = 0xFF;
		}

		m90_209_211.chr.latch[0] = 0;
		m90_209_211.chr.latch[1] = 4;

		m90_209_211.irq.prescaler = 0xFF;
		m90_209_211.irq.premask = 0x07;

		if (m90_209_211.model == MAP211) {
			m90_209_211.tekker = 0xC0;
		}
	} else {
		/* RESET */
		m90_209_211.tekker += 0x40;
		m90_209_211.tekker &= 0xC0;

		for (i = 0; i < 8; i++) {
			if (i < 4) {
				m90_209_211.prg[i] = 0xFF;
				m90_209_211.mode[i] = 0;
			}
		}
	}
	prg_setup_90_209_211();
	chr_setup_90_209_211();

	info.mapper.extend_wr = TRUE;
}
void extcl_cpu_wr_mem_90_209_211(WORD address, BYTE value) {
	if (address <= 0x4FFF) {
		return;
	}
	if (address <= 0x5FFF) {
		switch (address & 0x5C03) {
			case 0x5800:
				m90_209_211.mul[0] = value;
				break;
			case 0x5801:
				m90_209_211.mul[1] = value;
				break;
			case 0x5803:
				m90_209_211.single_byte_ram = value;
				break;
		}
		return;
	}
	if (address <= 0x7FFF) {
		return;
	}
	if (address <= 0x8FF0) {
		m90_209_211.prg[address & 0x0003] = value;
		prg_setup_90_209_211();
		return;
	}
	if (address <= 0x8FFF) {
		return;
	}
	if (address <= 0x9FFF) {
		m90_209_211.chr.low[address & 0x0007] = value;
		chr_setup_90_209_211();
		return;
	}
	if (address <= 0xAFFF) {
		m90_209_211.chr.high[address & 0x0007] = value;
		chr_setup_90_209_211();
		return;
	}
	if (address <= 0xBFFF) {
		BYTE index = address & 0x0003;

		if (address & 0x0004) {
			m90_209_211.nmt.reg[index] &= 0x00FF;
			m90_209_211.nmt.reg[index] |= (value << 8);
		} else {
			m90_209_211.nmt.reg[index] &= 0xFF00;
			m90_209_211.nmt.reg[index] |= value;
		}

		nmt_setup_90_209_211();
		return;
	}
	if (address <= 0xCFFF) {
		switch (address & 0x0007) {
			case 0:
				m90_209_211.irq.active = value & 0x01;
				if (!m90_209_211.irq.active) {
					irq.high &= ~EXT_IRQ;
				}
				break;
			case 1:
				m90_209_211.irq.mode = value;

				if (m90_209_211.irq.mode & 0x04) {
					m90_209_211.irq.premask = 0x07;
				} else {
					m90_209_211.irq.premask = 0xFF;
				}
				break;
			case 2:
				m90_209_211.irq.active = 0;
				irq.high &= ~EXT_IRQ;
				break;
			case 3:
				m90_209_211.irq.active = 1;
				break;
			case 4:
				m90_209_211.irq.prescaler = value ^ m90_209_211.irq.xor_value;
				break;
			case 5:
				m90_209_211.irq.count = value ^ m90_209_211.irq.xor_value;
				break;
			case 6:
				m90_209_211.irq.xor_value = value;
				break;
			case 7:
				m90_209_211.irq.pre_size = value;
				break;
		}
		return;
	}
	if (address <= 0xDFFF) {
		BYTE index = address & 0x0003;

		m90_209_211.mode[index] = value;

		if (index == 0) {
			if (((m90_209_211.mode[0] & 0x20) && (m90_209_211.model == MAP209))
					|| (m90_209_211.model == MAP211)) {
				m90_209_211.nmt.extended_mode = TRUE;
			} else {
				m90_209_211.nmt.extended_mode = FALSE;
			}
		}
		prg_setup_90_209_211();
		chr_setup_90_209_211();
		nmt_setup_90_209_211();
		return;
	}
	/* questi non servono
	if (address <= 0xDFFF) {
		return;
	}
	if (address <= 0xFFFF) {
		return;
	}
	*/
}
BYTE extcl_cpu_rd_mem_90_209_211(WORD address, BYTE openbus, BYTE before) {
	if (address <= 0x4FFF) {
		return (openbus);
	}
	if (address <= 0x5FFF) {
		switch (address & 0x5C03) {
			case 0x5800:
				return (m90_209_211.mul[0] * m90_209_211.mul[1]);
			case 0x5801:
				return ((m90_209_211.mul[0] * m90_209_211.mul[1]) >> 8);
			case 0x5803:
				return (m90_209_211.single_byte_ram);
			default:
				return (m90_209_211.tekker);
		}
		return (0xFF);
	}
	if ((address <= 0x7FFF) && (m90_209_211.mode[0] & 0x80)) {
		return (m90_209_211.m6000.rom_8k [address & 0x1FFF]);
	}
	return (openbus);
}
BYTE extcl_save_mapper_90_209_211(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, m90_209_211.mul);
	save_slot_ele(mode, slot, m90_209_211.single_byte_ram);
	save_slot_ele(mode, slot, m90_209_211.tekker);

	save_slot_ele(mode, slot, m90_209_211.mode);

	save_slot_ele(mode, slot, m90_209_211.prg);

	save_slot_ele(mode, slot, m90_209_211.chr.latch);
	save_slot_ele(mode, slot, m90_209_211.chr.low);
	save_slot_ele(mode, slot, m90_209_211.chr.high);

	save_slot_ele(mode, slot, m90_209_211.nmt.extended_mode);
	save_slot_ele(mode, slot, m90_209_211.nmt.reg);
	save_slot_ele(mode, slot, m90_209_211.nmt.write);

	save_slot_ele(mode, slot, m90_209_211.irq.active);
	save_slot_ele(mode, slot, m90_209_211.irq.mode);
	save_slot_ele(mode, slot, m90_209_211.irq.prescaler);
	save_slot_ele(mode, slot, m90_209_211.irq.count);
	save_slot_ele(mode, slot, m90_209_211.irq.xor_value);
	save_slot_ele(mode, slot, m90_209_211.irq.pre_size);
	save_slot_ele(mode, slot, m90_209_211.irq.premask);

	if (mode == SAVE_SLOT_READ) {
		prg_setup_90_209_211();
		chr_setup_90_209_211();
		nmt_setup_90_209_211();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_90_209_211(void) {
	if ((m90_209_211.irq.mode & 0x03) == 0) {
		irq_clock_prescaler_90_209_211();
	}
}
void extcl_rd_ppu_90_209_211(WORD address) {
	if ((m90_209_211.irq.mode & 0x03) == 2) {
		/*
		if (lastread != A) {
			ClockCounter();
			ClockCounter();
		}
		lastread = A;
		*/
		irq_clock_prescaler_90_209_211();
	}
}
BYTE extcl_rd_chr_90_209_211(WORD address) {
	if (m90_209_211.model == MAP209) {
		switch (address & 0x0FF8) {
			case 0x0FD8:
			case 0x0FE8:
				m90_209_211.chr.latch[address >> 12] = address >> 4 & ((address >> 10 & 0x4) | 0x2);
				chr_setup_90_209_211();
				break;
		}
	}

	return (chr.bank_1k[address >> 10][address & 0x3FF]);
}
void extcl_wr_nmt_90_209_211(WORD address, BYTE value) {
	BYTE index = (address & 0x0FFF) >> 10;

	if ((m90_209_211.nmt.extended_mode == TRUE) && (m90_209_211.nmt.write[index] == FALSE)) {
		return;
	}

	ntbl.bank_1k[index][address & 0x3FF] = value;
}
void extcl_ppu_000_to_255_90_209_211(void) {
	if (r2001.visible) {
		if ((ppu.frame_x & 0x0007) != 0x0003) {
			return;
		}

		if (ppu.frame_x == 323) {
			ppu_spr_adr(7);
		}

		ppu_bck_adr(r2000.bpt_adr, r2006.value);

		if ((m90_209_211.irq.mode & 0x03) == 1) {
			if ((ppu.bck_adr & 0x1000) > (ppu.spr_adr & 0x1000)) {
				irq_clock_prescaler_90_209_211();
			}
		}
	}
}
void extcl_ppu_256_to_319_90_209_211(void) {
	if ((ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!spr_ev.count_plus) && (r2000.size_spr == 16)) {
		ppu.spr_adr = 0x1000;
	} else {
		ppu_spr_adr((ppu.frame_x & 0x0038) >> 3);
	}

	if ((m90_209_211.irq.mode & 0x03) == 1) {
		if ((ppu.spr_adr & 0x1000) > (ppu.bck_adr & 0x1000)) {
			irq_clock_prescaler_90_209_211();
		}
	}
}
void extcl_ppu_320_to_34x_90_209_211(void) {
	extcl_ppu_000_to_255_90_209_211();
}
void extcl_update_r2006_90_209_211(WORD new_r2006, WORD old_r2006) {
	if ((m90_209_211.irq.mode & 0x03) == 1) {
		if ((new_r2006 & 0x1000) > (old_r2006 & 0x1000)) {
			irq_clock_prescaler_90_209_211();
		}
	}
}

static void INLINE prg_setup_90_209_211(void) {
	BYTE value, bankmode = ((m90_209_211.mode[3] & 0x06) << 5);

	switch (m90_209_211.mode[0] & 0x07) {
		case 0:
			/* prg rom da 8k al 0x6000 */
			value = (((m90_209_211.prg[3] << 2) + 3) & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211.m6000.prg = value;
			/* prg rom switch normale */
			value = 0x0F | ((m90_209_211.mode[3] & 0x06) << 3);
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			break;
		case 1:
			/* prg rom da 8k al 0x6000 */
			value = (((m90_209_211.prg[3] << 1) + 1) & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211.m6000.prg = value;
			/* prg rom switch normale */
			value = (m90_209_211.prg[1] & 0x1F) | ((m90_209_211.mode[3] & 0x06) << 4);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			value = 0x1F | ((m90_209_211.mode[3] & 0x06) << 4);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
		case 3: // bit reversion
		case 2:
			/* prg rom da 8k al 0x6000 */
			value = (m90_209_211.prg[3] & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211.m6000.prg = value;
			/* prg rom switch normale */
			value = (m90_209_211.prg[0] & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			value = (m90_209_211.prg[1] & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			value = (m90_209_211.prg[2] & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			value = 0x3F | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, value);
			break;
		case 4:
			/* prg rom da 8k al 0x6000 */
			value = (((m90_209_211.prg[3] << 2) + 3) & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211.m6000.prg = value;
			/* prg rom switch normale */
			value = (m90_209_211.prg[3] & 0x0F) | ((m90_209_211.mode[3] & 0x06) << 3);
			control_bank(info.prg.rom.max.banks_32k)
			map_prg_rom_8k(4, 0, value);
			break;
		case 5:
			/* prg rom da 8k al 0x6000 */
			value = (((m90_209_211.prg[3] << 1) + 1) & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211.m6000.prg = value;
			/* prg rom switch normale */
			value = (m90_209_211.prg[1] & 0x1F) | ((m90_209_211.mode[3] & 0x06) << 4);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			value = (m90_209_211.prg[3] & 0x1F) | ((m90_209_211.mode[3] & 0x06) << 4);
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 2, value);
			break;
		case 7: // bit reversion
		case 6:
			/* prg rom da 8k al 0x6000 */
			value = (m90_209_211.prg[3] & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			m90_209_211.m6000.prg = value;
			/* prg rom switch normale */
			value = (m90_209_211.prg[0] & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 0, value);
			value = (m90_209_211.prg[1] & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 1, value);
			value = (m90_209_211.prg[2] & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 2, value);
			value = (m90_209_211.prg[3] & 0x3F) | bankmode;
			control_bank(info.prg.rom.max.banks_8k)
			map_prg_rom_8k(1, 3, value);
			break;
	}
	m90_209_211.m6000.rom_8k = prg_chip_byte_pnt(0, m90_209_211.m6000.prg << 13);
	map_prg_rom_8k_update();
}
static void INLINE chr_setup_90_209_211(void) {
	WORD value, base = 0, mask = 0xFFFF;
	DBWORD bank;

	if (!(m90_209_211.mode[3] & 0x20)) {
		base = (m90_209_211.mode[3] & 0x01) | ((m90_209_211.mode[3] & 0x18) >> 2);
		switch (m90_209_211.mode[0] & 0x18) {
			case 0x00:
				base <<= 5;
				mask = 0x001F;
				break;
			case 0x08:
				base <<= 6;
				mask = 0x003F;
				break;
			case 0x10:
				base <<= 7;
				mask = 0x007F;
				break;
			case 0x18:
				base <<= 8;
				mask = 0x00FF;
				break;
		}
	}
	switch (m90_209_211.mode[0] & 0x18) {
		case 0x00:
			value = chr_90_209_211(0);
			control_bank(info.chr.rom.max.banks_8k)
			bank = value << 13;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank | 0x1000);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x1400);
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x1800);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x1C00);
			break;
		case 0x08:
			value = chr_90_209_211(m90_209_211.chr.latch[0]);
			control_bank(info.chr.rom.max.banks_4k)
			bank = value << 12;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0C00);
			value = chr_90_209_211(m90_209_211.chr.latch[1]);
			control_bank(info.chr.rom.max.banks_4k)
			bank = value << 12;
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank | 0x0800);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0C00);
			break;
		case 0x10:
			value = chr_90_209_211(0);
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank | 0x0400);
			value = chr_90_209_211(2);
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank | 0x0400);
			value = chr_90_209_211(4);
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank | 0x0400);
			value = chr_90_209_211(6);
			control_bank(info.chr.rom.max.banks_2k)
			bank = value << 11;
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank);
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank | 0x0400);
			break;
		case 0x18:
			value = chr_90_209_211(0);
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[0] = chr_chip_byte_pnt(0, bank);
			value = chr_90_209_211(1);
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[1] = chr_chip_byte_pnt(0, bank);
			value = chr_90_209_211(2);
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[2] = chr_chip_byte_pnt(0, bank);
			value = chr_90_209_211(3);
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[3] = chr_chip_byte_pnt(0, bank);
			value = chr_90_209_211(4);
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[4] = chr_chip_byte_pnt(0, bank);
			value = chr_90_209_211(5);
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[5] = chr_chip_byte_pnt(0, bank);
			value = chr_90_209_211(6);
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[6] = chr_chip_byte_pnt(0, bank);
			value = chr_90_209_211(7);
			control_bank(info.chr.rom.max.banks_1k)
			bank = value << 10;
			chr.bank_1k[7] = chr_chip_byte_pnt(0, bank);
			break;
	}
}
static void INLINE nmt_setup_90_209_211(void) {
	if (m90_209_211.nmt.extended_mode == TRUE) {
		WORD value;

		if (m90_209_211.mode[0] & 0x40) {
			nmt_rom_90_209_211(0);
			nmt_rom_90_209_211(1);
			nmt_rom_90_209_211(2);
			nmt_rom_90_209_211(3);
		} else {
			BYTE i;

			for (i = 0; i < 4; i++) {
				if ((m90_209_211.mode[2] ^ m90_209_211.nmt.reg[i]) & 0x80) {
					nmt_rom_90_209_211(i);
				} else {
					nmt_ram_90_209_211(i);
				}
			}
		}
	} else {
		switch (m90_209_211.mode[1] & 0x03) {
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
	}
}
static void INLINE irq_clock_prescaler_90_209_211(void) {
	if ((m90_209_211.irq.mode >> 6) == 1) {
		if ((++m90_209_211.irq.prescaler & m90_209_211.irq.premask) == 0) {
			irq_clock_count_90_209_211();
		}
	} else if ((m90_209_211.irq.mode >> 6) == 2) {
		if ((--m90_209_211.irq.prescaler & m90_209_211.irq.premask) == m90_209_211.irq.premask) {
			irq_clock_count_90_209_211();
		}
	}
}
static void INLINE irq_clock_count_90_209_211(void) {
	if ((m90_209_211.irq.mode >> 6) == 1) {
		if ((++m90_209_211.irq.count == 0) && m90_209_211.irq.active) {
			irq.high |= EXT_IRQ;
		}
	} else if ((m90_209_211.irq.mode >> 6) == 2) {
		if ((--m90_209_211.irq.count == 0xFF) && m90_209_211.irq.active) {
			irq.high |= EXT_IRQ;
		}
	}
}
