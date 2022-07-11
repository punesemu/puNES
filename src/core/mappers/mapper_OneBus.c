/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#include <string.h>
#include "mappers.h"
#include "mem_map.h"
#include "info.h"
#include "irqA12.h"
#include "save_slot.h"

enum onebus_models {
	ONEBUS_NORMAL,
	ONEBUS_WAIXING_VT03,
	ONEBUS_POWER_JOY_SUPERMAX,
	ONEBUS_ZECHESS,
	ONEBUS_SPORTS_GAME,
	ONEBUS_WAIXING_VT02,
	ONEBUS_MODELS
};

INLINE static void prg_fix_OneBus(void);
INLINE static void chr_fix_OneBus(void);

static const BYTE chr_mask[8] = { 0xFF, 0x7F, 0x3F, 0x00, 0x1F, 0x0F, 0x07, 0x00 };
static const BYTE chrreg[ONEBUS_MODELS][6] = {
	{ 6, 7, 2, 3, 4, 5 }, // ONEBUS_NORMAL
	{ 6, 7, 2, 3, 4, 5 }, // ONEBUS_WAIXING_VT03
	{ 6, 7, 2, 3, 4, 5 }, // ONEBUS_POWER_JOY_SUPERMAX
	{ 3, 2, 6, 7, 5, 4 }, // ONEBUS_ZECHESS
	{ 5, 3, 4, 7, 2, 6 }, // ONEBUS_SPORTS_GAME
	{ 5, 4, 3, 2, 6, 7 }, // ONEBUS_WAIXING_VT02
};
static const BYTE prgreg[ONEBUS_MODELS][2] = {
	{ 7, 8 }, // ONEBUS_NORMAL
	{ 7, 8 }, // ONEBUS_WAIXING_VT03
	{ 8, 7 }, // ONEBUS_POWER_JOY_SUPERMAX
	{ 7, 8 }, // ONEBUS_ZECHESS
	{ 7, 8 }, // ONEBUS_SPORTS_GAME
	{ 7, 8 }, // ONEBUS_WAIXING_VT02
};

struct _onebus {
	struct _onebus_regs {
		BYTE cpu[16];
		BYTE ppu[16];
		BYTE apu[64];
	} reg;
	struct _onebus_pcm {
		SWORD address;
		SWORD size;
		SWORD latch;
		SWORD clock;
		BYTE enable;
		BYTE irq;
	} pcm;

	// da non salvare
	BYTE model;
} onebus;

void map_init_OneBus(void) {
	EXTCL_CPU_WR_MEM(OneBus);
	EXTCL_SAVE_MAPPER(OneBus);
	EXTCL_CPU_EVERY_CYCLE(OneBus);
	EXTCL_WR_PPU_REG(OneBus);
	EXTCL_WR_APU(OneBus);
	EXTCL_RD_APU(OneBus);
	EXTCL_IRQ_A12_CLOCK(OneBus);
	EXTCL_PPU_000_TO_34X(MMC3);
	EXTCL_PPU_000_TO_255(MMC3);
	EXTCL_PPU_256_TO_319(MMC3);
	EXTCL_PPU_320_TO_34X(MMC3);
	EXTCL_UPDATE_R2006(MMC3);
	mapper.internal_struct[0] = (BYTE *)&onebus;
	mapper.internal_struct_size[0] = sizeof(onebus);

	memset(&irqA12, 0x00, sizeof(irqA12));
	memset(&onebus.reg, 0x00, sizeof(onebus.reg));

	if (info.reset >= HARD) {
		onebus.pcm.enable = 0;
		onebus.pcm.irq = 0;
		onebus.pcm.address = 0;
		onebus.pcm.size = 0;
		onebus.pcm.latch = 0;
		onebus.pcm.clock = 0xE1;
	}

	if (info.format != NES_2_0) {
		if (info.mapper.submapper == DEFAULT) {

			if ((info.crc32.prg == 0x947AC898) || // Power Joy Supermax 30-in-1 (Unl) [U][!].unf
				(info.crc32.prg == 0x1AB45228)) { // Power Joy Supermax 60-in-1 (Unl) [U][!].unf
				info.mapper.submapper = ONEBUS_POWER_JOY_SUPERMAX;
			} else if (info.crc32.prg == 0x1242DA7F) {
				// "Sports Game 69-in-1 (Unl) [U][!].unf" e "DreamGEAR 75-in-1 (Unl) [U][!].unf"
				// alcuni giochi (tipo SHARKS) di queste roms non funzionano correttamente perche' utilizzano l'extended mode
				// del VT03, lo si puo' notare dal fatto che viene settato il registro $2010 a 0x86 o 0x96 (ho dato un'occhiata
				// al src/devices/video/ppu2c0x_vt.cpp del mame per capire il significato dei bits di questo registro).
				info.mapper.submapper = ONEBUS_SPORTS_GAME;
			}
		}
		if (info.prg.ram.banks_8k_plus == 0) {
			info.prg.ram.banks_8k_plus = 1;
		}
	}

	onebus.model = info.mapper.submapper == DEFAULT ? ONEBUS_NORMAL : info.mapper.submapper;

	info.mapper.extend_wr = TRUE;
	mapper.write_vram = FALSE;

	irqA12.present = TRUE;

	prg_fix_OneBus();
	chr_fix_OneBus();
}
void extcl_cpu_wr_mem_OneBus(WORD address, BYTE value) {
	if ((address >= 0x4100) && (address <= 0x410F)) {
		switch (address & 0x000F) {
			case 1:
				onebus.reg.cpu[1] = value & 0xFE;
				break;
			case 2:
				irqA12.reload = TRUE;
				break;
			case 3 :
				irq.high &= ~EXT_IRQ;
				irqA12.enable = FALSE;
				break;
			case 4:
				irqA12.enable = TRUE;
				break;
			default:
				onebus.reg.cpu[address & 0x000F] = value;
				prg_fix_OneBus();
				chr_fix_OneBus();
				break;
		}
		return;
	}
	if (address >= 0x8000) {
		switch (address & 0xE001) {
			case 0x8000:
				onebus.reg.cpu[5] = (onebus.reg.cpu[5] & 0x38) | (value & 0xC7);
				prg_fix_OneBus();
				chr_fix_OneBus();
				break;
			case 0x8001:
				switch (onebus.reg.cpu[5] & 0x07) {
					case 0:
						onebus.reg.ppu[chrreg[onebus.model][0]] = value;
						chr_fix_OneBus();
						break;
					case 1:
						onebus.reg.ppu[chrreg[onebus.model][1]] = value;
						chr_fix_OneBus();
						break;
					case 2:
						onebus.reg.ppu[chrreg[onebus.model][2]] = value;
						chr_fix_OneBus();
						break;
					case 3:
						onebus.reg.ppu[chrreg[onebus.model][3]] = value;
						chr_fix_OneBus();
						break;
					case 4:
						onebus.reg.ppu[chrreg[onebus.model][4]] = value;
						chr_fix_OneBus();
						break;
					case 5:
						onebus.reg.ppu[chrreg[onebus.model][5]] = value;
						chr_fix_OneBus();
						break;
					case 6:
						onebus.reg.cpu[7] = value;
						prg_fix_OneBus();
						break;
					case 7:
						onebus.reg.cpu[8] = value;
						prg_fix_OneBus();
						break;
				}
				break;
			case 0xA000:
				onebus.reg.cpu[6] = value;
				chr_fix_OneBus();
				break;
			case 0xC000:
				onebus.reg.cpu[1] = value & 0xFE;
				break;
			case 0xC001:
				irqA12.reload = TRUE;
				break;
			case 0xE000:
				irq.high &= ~EXT_IRQ;
				irqA12.enable = FALSE;
				break;
			case 0xE001:
				irqA12.enable = TRUE;
				break;
		}
	}
}
BYTE extcl_save_mapper_OneBus(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, onebus.reg.cpu);
	save_slot_ele(mode, slot, onebus.reg.ppu);
	save_slot_ele(mode, slot, onebus.reg.apu);
	save_slot_ele(mode, slot, onebus.pcm.address);
	save_slot_ele(mode, slot, onebus.pcm.size);
	save_slot_ele(mode, slot, onebus.pcm.latch);
	save_slot_ele(mode, slot, onebus.pcm.clock);
	save_slot_ele(mode, slot, onebus.pcm.enable);
	save_slot_ele(mode, slot, onebus.pcm.irq);

	if (mode == SAVE_SLOT_READ) {
		prg_fix_OneBus();
		chr_fix_OneBus();
	}

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_OneBus(void) {
	if (onebus.pcm.enable) {
		onebus.pcm.latch--;
		if (onebus.pcm.latch <= 0) {
			onebus.pcm.latch += onebus.pcm.clock;
			onebus.pcm.size--;
			if (onebus.pcm.size < 0) {
				onebus.pcm.irq = 0x80;
				onebus.pcm.enable = 0;
				irq.high |= EXT_IRQ;
			} else {
				WORD address = onebus.pcm.address | ((onebus.reg.apu[48] ^ 3) << 14);

				apu_wr_mem_mapper(0x4011, (BYTE)(cpu_rd_mem_dbg(address) >> 1));
				onebus.pcm.address++;
				onebus.pcm.address &= 0x7FFF;
			}
		}
	}
}
BYTE extcl_wr_ppu_reg_OneBus(WORD address, BYTE *value) {
	if ((address >= 0x2010) && (address <= 0x201F)) {
		if (onebus.model == ONEBUS_WAIXING_VT02) {
			if (address == 0x2016) {
				address = 0x2015;
			} else if (address == 0x2017) {
				address = 0x2014;
			}
		}
		onebus.reg.ppu[address & 0x0F] = (*value);
		prg_fix_OneBus();
		chr_fix_OneBus();
		return (TRUE);
	}
	return (FALSE);
}
BYTE extcl_wr_apu_OneBus(WORD address, BYTE *value) {
	if ((address >= 0x4000) && (address <= 0x403F)) {
		onebus.reg.apu[address & 0x3F] = (*value);
		switch (address & 0x003F) {
			case 0x12:
				if (onebus.reg.apu[48] & 0x10) {
					onebus.pcm.address = (*value) << 6;
				}
				break;
			case 0x13:
				if (onebus.reg.apu[48] & 0x10) {
					onebus.pcm.size = ((*value) << 4) + 1;
				}
				break;
			case 0x15:
				if (onebus.reg.apu[48] & 0x10) {
					onebus.pcm.enable = (*value) & 0x10;
					if (onebus.pcm.irq) {
						irq.high &= ~EXT_IRQ;
						onebus.pcm.irq = 0;
					}
					if (onebus.pcm.enable) {
						onebus.pcm.latch = onebus.pcm.clock;
					}
					(*value) &= 0xEF;
				}
				break;
		}
	}
	return (FALSE);
}
BYTE extcl_rd_apu_OneBus(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	switch (address & 0x003F) {
		case 0x15:
			if (onebus.reg.apu[48] & 0x10) {
				openbus = (openbus & 0x7F) | onebus.pcm.irq;
			}
			break;
	}
	return (openbus);
}
void extcl_irq_A12_clock_OneBus(void) {
	BYTE count = irqA12.counter;

	if (!count || irqA12.reload) {
		irqA12.counter = onebus.reg.cpu[1];
		irqA12.reload = FALSE;
	} else {
		irqA12.counter--;
	}
	if (count && !irqA12.counter) {
		if (irqA12.enable) {
			irq.high |= EXT_IRQ;
		}
	}
}

INLINE static void prg_fix_OneBus(void) {
	BYTE mode = onebus.reg.cpu[11] & 0x07;
	BYTE mask = (mode == 0x07) ? 0xFF : (0x3F >> mode);
	BYTE swap = (onebus.reg.cpu[5] & 0x40) >> 5;
	WORD block = ((onebus.reg.cpu[0] & 0xF0) << 4) | (onebus.reg.cpu[10] & ~mask);
	WORD value;

	value = block | (onebus.reg.cpu[prgreg[onebus.model][0]] & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 0 ^ swap, value);

	value = block | (onebus.reg.cpu[prgreg[onebus.model][1]] & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 1, value);

	value = block | ((BYTE)(onebus.reg.cpu[11] & 0x40 ? onebus.reg.cpu[9] : ~1) & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 2 ^ swap, value);

	value = block | ((BYTE)(~0) & mask);
	control_bank(info.prg.rom.max.banks_8k)
	map_prg_rom_8k(1, 3, value);

	map_prg_rom_8k_update();
}
INLINE static void chr_fix_OneBus(void) {
	BYTE mask = chr_mask[onebus.reg.ppu[10] & 0x07];
	BYTE swap = (onebus.reg.cpu[5] & 0x80) >> 5;
	WORD block = ((onebus.reg.cpu[0] & 0x0F) << 11) | ((onebus.reg.ppu[8] & 0x70) << 4) | (onebus.reg.ppu[10] & ~mask);
	WORD value;

	value = block | ((onebus.reg.ppu[chrreg[onebus.model][0]] & ~1) & mask);
	control_bank(info.prg.rom.max.banks_1k)
	chr.bank_1k[0 ^ swap] = prg_pnt(value << 10);

	value = block | ((onebus.reg.ppu[chrreg[onebus.model][0]] | 1) & mask);
	control_bank(info.prg.rom.max.banks_1k)
	chr.bank_1k[1 ^ swap] = prg_pnt(value << 10);

	value = block | ((onebus.reg.ppu[chrreg[onebus.model][1]] & ~1) & mask);
	control_bank(info.prg.rom.max.banks_1k)
	chr.bank_1k[2 ^ swap] = prg_pnt(value << 10);

	value = block | ((onebus.reg.ppu[chrreg[onebus.model][1]] | 1) & mask);
	control_bank(info.prg.rom.max.banks_1k)
	chr.bank_1k[3 ^ swap] = prg_pnt(value << 10);

	value = block | (onebus.reg.ppu[chrreg[onebus.model][2]] & mask);
	control_bank(info.prg.rom.max.banks_1k)
	chr.bank_1k[4 ^ swap] = prg_pnt(value << 10);

	value = block | (onebus.reg.ppu[chrreg[onebus.model][3]] & mask);
	control_bank(info.prg.rom.max.banks_1k)
	chr.bank_1k[5 ^ swap] = prg_pnt(value << 10);

	value = block | (onebus.reg.ppu[chrreg[onebus.model][4]] & mask);
	control_bank(info.prg.rom.max.banks_1k)
	chr.bank_1k[6 ^ swap] = prg_pnt(value << 10);

	value = block | (onebus.reg.ppu[chrreg[onebus.model][5]] & mask);
	control_bank(info.prg.rom.max.banks_1k)
	chr.bank_1k[7 ^ swap] = prg_pnt(value << 10);

	if (onebus.reg.cpu[6] & 0x01) {
		mirroring_H();
	} else {
		mirroring_V();
	}
}
