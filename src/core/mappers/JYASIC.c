/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"
#include "ppu_inline.h"

void (*JYASIC_prg_fix)(void);
void (*JYASIC_prg_swap)(WORD address, DBWORD value);
void (*JYASIC_chr_fix)(void);
void (*JYASIC_chr_swap)(WORD address, DBWORD value);
void (*JYASIC_wram_fix)(void);
void (*JYASIC_wram_swap)(WORD address, DBWORD value);
void (*JYASIC_mirroring_fix)(void);
void (*JYASIC_mirroring_swap)(WORD address, DBWORD value);

INLINE static void irq_clock_prescaler_JYASIC(void);
INLINE static BYTE prg_reverse_JYASIC(BYTE value);

_jyasic jyasic;
struct _jyasictmp {
	BYTE extended_mode;
} jyasictmp;

// promemoria
//void map_init_JYASIC(void) {
//	EXTCL_AFTER_MAPPER_INIT(JYASIC);
//	EXTCL_CPU_WR_MEM(JYASIC);
//	EXTCL_CPU_RD_MEM(JYASIC);
//	EXTCL_SAVE_MAPPER(JYASIC);
//	EXTCL_CPU_EVERY_CYCLE(JYASIC);
//	EXTCL_RD_PPU_MEM(JYASIC);
//	EXTCL_RD_CHR(JYASIC);
//	EXTCL_PPU_000_TO_255(JYASIC);
//	EXTCL_PPU_256_TO_319(JYASIC);
//	EXTCL_PPU_320_TO_34X(JYASIC);
//	EXTCL_UPDATE_R2006(JYASIC);
//}

void extcl_after_mapper_init_JYASIC(void) {
	JYASIC_prg_fix();
	JYASIC_chr_fix();
	JYASIC_wram_fix();
	JYASIC_mirroring_fix();
}
void extcl_cpu_wr_mem_JYASIC(WORD address, BYTE value) {
	if ((jyasic.irq.mode & 0x03) == 3) {
		irq_clock_prescaler_JYASIC();
	}
	switch (address & 0xF000) {
		case 0x5000:
			switch (address & 0x5803) {
				case 0x5800:
					jyasic.mul[0] = value;
					break;
				case 0x5801:
					jyasic.mul[1] = value;
					break;
				case 0x5802:
					jyasic.add += value;
					break;
				case 0x5803:
					jyasic.single_byte_ram = value;
					jyasic.add = 0;
					break;
			}
			return;
		case 0x8000:
			if (!(address & 0x0800)) {
				jyasic.prg[address & 0x0003] = value;
				JYASIC_prg_fix();
				JYASIC_wram_fix();
			}
			return;
		case 0x9000:
			if (!(address & 0x0800)) {
				BYTE index = address & 0x0007;

				jyasic.chr.reg[index] = (jyasic.chr.reg[index] & 0xFF00) | value;
				JYASIC_chr_fix();
			}
			return;
		case 0xA000:
			if (!(address & 0x0800)) {
				BYTE index = address & 0x0007;

				jyasic.chr.reg[index] = (jyasic.chr.reg[index] & 0x00FF) | (value << 8);
				JYASIC_chr_fix();
			}
			return;
		case 0xB000:
			if (!(address & 0x0800)) {
				BYTE index = address & 0x0003;

				if (address & 0x0004) {
					jyasic.nmt.reg[index] = (jyasic.nmt.reg[index] & 0x00FF) | (value << 8);
				} else {
					jyasic.nmt.reg[index] = (jyasic.nmt.reg[index] & 0xFF00) | value;
				}
				JYASIC_mirroring_fix();
			}
			return;
		case 0xC000:
			switch (address & 0x0007) {
				case 0:
					jyasic.irq.active = value & 0x01;
					if (!jyasic.irq.active) {
						jyasic.irq.prescaler = 0;
						irq.high &= ~EXT_IRQ;
					}
					break;
				case 1:
					jyasic.irq.mode = value;
					if (jyasic.irq.mode & 0x04) {
						jyasic.irq.premask = 0x07;
					} else {
						jyasic.irq.premask = 0xFF;
					}
					break;
				case 2:
					jyasic.irq.active = 0;
					jyasic.irq.prescaler = 0;
					irq.high &= ~EXT_IRQ;
					break;
				case 3:
					jyasic.irq.active = 1;
					break;
				case 4:
					jyasic.irq.prescaler = value ^ jyasic.irq.xor_value;
					break;
				case 5:
					jyasic.irq.count = value ^ jyasic.irq.xor_value;
					break;
				case 6:
					jyasic.irq.xor_value = value;
					break;
				case 7:
					jyasic.irq.pre_size = value;
					break;
			}
			return;
		case 0xD000:
			if (!(address & 0x0800)) {
				BYTE index = address & 0x0003;

				switch (index) {
					default:
					case 0:
						jyasic.nmt.extended_mode = jyasictmp.extended_mode ? (value & 0x20) != 0 : FALSE;
						jyasic.mode[index] = value;
						break;
					case 1:
						jyasic.mode[index] = !jyasictmp.extended_mode ? value & ~0x08 : value;
						break;
					case 2:
					case 3:
						jyasic.mode[index] = value;
						break;
				}
				JYASIC_prg_fix();
				JYASIC_chr_fix();
				JYASIC_wram_fix();
				JYASIC_mirroring_fix();
			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_JYASIC(WORD address, BYTE openbus) {
	if ((address >= 0x5000) && (address <= 0x5FFF)) {
		if (!(address & 0x03FF) && (address != 0x5800)) {
			return ((dipswitch.value & 0xC0) | (openbus & 0x3F));
		}
		switch (address & 0x5803) {
			case 0x5800:
				return (jyasic.mul[0] * jyasic.mul[1]);
			case 0x5801:
				return ((jyasic.mul[0] * jyasic.mul[1]) >> 8);
			case 0x5802:
				return (jyasic.add);
			case 0x5803:
				return (jyasic.single_byte_ram);
		}
	}
	return (openbus);
}
BYTE extcl_save_mapper_JYASIC(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, jyasic.mul);
	save_slot_ele(mode, slot, jyasic.single_byte_ram);
	save_slot_ele(mode, slot, jyasic.add);

	save_slot_ele(mode, slot, jyasic.mode);

	save_slot_ele(mode, slot, jyasic.prg);

	save_slot_ele(mode, slot, jyasic.chr.latch);
	save_slot_ele(mode, slot, jyasic.chr.reg);

	save_slot_ele(mode, slot, jyasic.nmt.extended_mode);
	save_slot_ele(mode, slot, jyasic.nmt.reg);

	save_slot_ele(mode, slot, jyasic.irq.active);
	save_slot_ele(mode, slot, jyasic.irq.mode);
	save_slot_ele(mode, slot, jyasic.irq.prescaler);
	save_slot_ele(mode, slot, jyasic.irq.count);
	save_slot_ele(mode, slot, jyasic.irq.xor_value);
	save_slot_ele(mode, slot, jyasic.irq.pre_size);
	save_slot_ele(mode, slot, jyasic.irq.premask);

	return (EXIT_OK);
}
void extcl_cpu_every_cycle_JYASIC(void) {
	if ((jyasic.irq.mode & 0x03) == 0) {
		irq_clock_prescaler_JYASIC();
	}
}
void extcl_rd_ppu_mem_JYASIC(UNUSED(WORD address)) {
	if ((jyasic.irq.mode & 0x03) == 2) {
		irq_clock_prescaler_JYASIC();
	}
}
BYTE extcl_rd_chr_JYASIC(WORD address) {
	if (jyasic.mode[3] & 0x80) {
		switch (address & 0x0FF8) {
			case 0x0FD8:
			case 0x0FE8: {
				BYTE last = chr_rd(address);

				jyasic.chr.latch[address >> 12] = (address >> 4) & (((address >> 10) & 0x04) | 0x02);
				if ((jyasic.mode[0] & 0x18) == 0x08) {
					JYASIC_chr_fix();
				}
				return (last);
			}
		}
	}
	return (chr_rd(address));
}
void extcl_ppu_000_to_255_JYASIC(void) {
	if (r2001.visible) {
		extcl_ppu_320_to_34x_JYASIC();
	}
}
void extcl_ppu_256_to_319_JYASIC(void) {
	if ((ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if ((!spr_ev.count_plus || (spr_ev.tmp_spr_plus == spr_ev.count_plus)) && (r2000.size_spr == 16)) {
		ppu.spr_adr = r2000.spt_adr;
	} else {
		ppu_spr_adr((ppu.frame_x & 0x0038) >> 3);
	}
	if ((ppu.spr_adr & 0x1000) > (ppu.bck_adr & 0x1000)) {
		if ((jyasic.irq.mode & 0x03) == 1) {
			irq_clock_prescaler_JYASIC();
		}
	}
}
void extcl_ppu_320_to_34x_JYASIC(void) {
	if ((ppu.frame_x & 0x0007) != 0x0003) {
		return;
	}

	if (ppu.frame_x == 323) {
		ppu_spr_adr(7);
	}

	ppu_bck_adr(r2000.bpt_adr, r2006.value);

	if ((ppu.bck_adr & 0x1000) > (ppu.spr_adr & 0x1000)) {
		if ((jyasic.irq.mode & 0x03) == 1) {
			irq_clock_prescaler_JYASIC();
		}
	}
}
void extcl_update_r2006_JYASIC(WORD new_r2006, WORD old_r2006) {
	if ((new_r2006 & 0x1000) > (old_r2006 & 0x1000)) {
		if ((jyasic.irq.mode & 0x03) == 1) {
			irq_clock_prescaler_JYASIC();
		}
	}
}

void init_JYASIC(BYTE extended_mode, BYTE reset) {
	if (reset >= HARD) {
		memset(&jyasic, 0x00, sizeof(jyasic));
	}

	jyasic.chr.latch[0] = 0;
	jyasic.chr.latch[1] = 4;

	irq.high &= ~EXT_IRQ;

	info.mapper.extend_wr = TRUE;

	JYASIC_prg_fix = prg_fix_JYASIC_base;
	JYASIC_prg_swap = prg_swap_JYASIC_base;
	JYASIC_chr_fix = chr_fix_JYASIC_base;
	JYASIC_chr_swap = chr_swap_JYASIC_base;
	JYASIC_wram_fix = wram_fix_JYASIC_base;
	JYASIC_wram_swap = wram_swap_JYASIC_base;
	JYASIC_mirroring_fix = mirroring_fix_JYASIC_base;
	JYASIC_mirroring_swap = mirroring_swap_JYASIC_base;

	jyasictmp.extended_mode = extended_mode;
}
void prg_fix_JYASIC_base(void) {
	DBWORD last_bank = jyasic.mode[0] & 0x04 ? jyasic.prg[3] : 0xFF;
	DBWORD bank[4];

	switch (jyasic.mode[0] & 0x03) {
		case 0:
			last_bank <<= 2;
			bank[0] = last_bank | 0;
			bank[1] = last_bank | 1;
			bank[2] = last_bank | 2;
			bank[3] = last_bank | 3;
			break;
		case 1:
			bank[0] = (jyasic.prg[1] << 1) | 0;
			bank[1] = (jyasic.prg[1] << 1) | 1;
			bank[2] = (last_bank << 1) | 0;
			bank[3] = (last_bank << 1) | 1;
			break;
		case 2:
			bank[0] = jyasic.prg[0];
			bank[1] = jyasic.prg[1];
			bank[2] = jyasic.prg[2];
			bank[3] = last_bank;
			break;
		case 3:
			bank[0] = prg_reverse_JYASIC(jyasic.prg[0]);
			bank[1] = prg_reverse_JYASIC(jyasic.prg[1]);
			bank[2] = prg_reverse_JYASIC(jyasic.prg[2]);
			bank[3] = prg_reverse_JYASIC(last_bank);
			break;
	}
	JYASIC_prg_swap(0x8000, bank[0]);
	JYASIC_prg_swap(0xA000, bank[1]);
	JYASIC_prg_swap(0xC000, bank[2]);
	JYASIC_prg_swap(0xE000, bank[3]);
}
void prg_swap_JYASIC_base(WORD address, DBWORD value) {
	memmap_auto_8k(MMCPU(address), value);
}
void chr_fix_JYASIC_base(void) {
	DBWORD bank[8];

	switch (jyasic.mode[0] & 0x18) {
		case 0x00:
			bank[0] = (jyasic.chr.reg[0] << 3) | 0;
			bank[1] = (jyasic.chr.reg[0] << 3) | 1;
			bank[2] = (jyasic.chr.reg[0] << 3) | 2;
			bank[3] = (jyasic.chr.reg[0] << 3) | 3;
			bank[4] = (jyasic.chr.reg[0] << 3) | 4;
			bank[5] = (jyasic.chr.reg[0] << 3) | 5;
			bank[6] = (jyasic.chr.reg[0] << 3) | 6;
			bank[7] = (jyasic.chr.reg[0] << 3) | 7;
			break;
		case 0x08:
			bank[0] = (jyasic.chr.reg[jyasic.chr.latch[0]] << 2) | 0;
			bank[1] = (jyasic.chr.reg[jyasic.chr.latch[0]] << 2) | 1;
			bank[2] = (jyasic.chr.reg[jyasic.chr.latch[0]] << 2) | 2;
			bank[3] = (jyasic.chr.reg[jyasic.chr.latch[0]] << 2) | 3;
			bank[4] = (jyasic.chr.reg[jyasic.chr.latch[1]] << 2) | 0;
			bank[5] = (jyasic.chr.reg[jyasic.chr.latch[1]] << 2) | 1;
			bank[6] = (jyasic.chr.reg[jyasic.chr.latch[1]] << 2) | 2;
			bank[7] = (jyasic.chr.reg[jyasic.chr.latch[1]] << 2) | 3;
			break;
		case 0x10:
			bank[0] = (jyasic.chr.reg[0] << 1) | 0;
			bank[1] = (jyasic.chr.reg[0] << 1) | 1;
			bank[2] = (jyasic.chr.reg[2] << 1) | 0;
			bank[3] = (jyasic.chr.reg[2] << 1) | 1;
			bank[4] = (jyasic.chr.reg[4] << 1) | 0;
			bank[5] = (jyasic.chr.reg[4] << 1) | 1;
			bank[6] = (jyasic.chr.reg[6] << 1) | 0;
			bank[7] = (jyasic.chr.reg[6] << 1) | 1;
			break;
		case 0x18:
			bank[0] = jyasic.chr.reg[0];
			bank[1] = jyasic.chr.reg[1];
			bank[2] = jyasic.chr.reg[2];
			bank[3] = jyasic.chr.reg[3];
			bank[4] = jyasic.chr.reg[4];
			bank[5] = jyasic.chr.reg[5];
			bank[6] = jyasic.chr.reg[6];
			bank[7] = jyasic.chr.reg[7];
			break;
	}
	JYASIC_chr_swap(0x0000, bank[0]);
	JYASIC_chr_swap(0x0400, bank[1]);
	JYASIC_chr_swap(0x0800, bank[2]);
	JYASIC_chr_swap(0x0C00, bank[3]);
	JYASIC_chr_swap(0x1000, bank[4]);
	JYASIC_chr_swap(0x1400, bank[5]);
	JYASIC_chr_swap(0x1800, bank[6]);
	JYASIC_chr_swap(0x1C00, bank[7]);
}
void chr_swap_JYASIC_base(WORD address, DBWORD value) {
	BYTE enabled = (jyasic.mode[2] & 0x40) >> 6;

	memmap_auto_wp_1k(MMPPU(address), value, TRUE, enabled);
}
void wram_fix_JYASIC_base(void) {
	DBWORD bank = jyasic.prg[3];

	switch (jyasic.mode[0] & 0x03) {
		case 0:
			bank = (bank << 2) | 3;
			break;
		case 1:
			bank = (bank << 1) | 1;
			break;
		case 2:
			break;
		case 3:
			bank = prg_reverse_JYASIC(bank);
			break;
	}
	if (jyasic.mode[0] & 0x80) {
		JYASIC_wram_swap(0x6000, bank);
	} else {
		memmap_wram_8k(MMCPU(0x6000), 0);
	}
}
// la uso solo per la swap nella PRGROM
void wram_swap_JYASIC_base(WORD address, DBWORD value) {
	memmap_prgrom_8k(MMCPU(address), value);
}
void mirroring_fix_JYASIC_base(void) {
	BYTE i = 0;

	if (jyasic.nmt.extended_mode) {
		for (i = 0; i < 4; i++) {
			WORD address = 0x2000 | (i * 0x400);

			if (((jyasic.mode[2] ^ jyasic.nmt.reg[i]) & 0x80) | (jyasic.mode[0] & 0x40)) {
				JYASIC_mirroring_swap(address, jyasic.nmt.reg[i]);
			} else {
				memmap_nmt_1k(MMPPU(address), (jyasic.nmt.reg[i] & 0x01));
				memmap_nmt_1k(MMPPU(address | 0x1000), (jyasic.nmt.reg[i] & 0x01));
			}
		}
	} else if (jyasic.mode[1] & 0x08) {
		for (i = 0; i < 4; i++) {
			WORD address = 0x2000 | (i * 0x400);

			memmap_nmt_1k(MMPPU(address), (jyasic.nmt.reg[i] & 0x01));
			memmap_nmt_1k(MMPPU(address | 0x1000), (jyasic.nmt.reg[i] & 0x01));
		}
	} else {
		switch (jyasic.mode[1] & 0x03) {
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
// la uso solo per la swap nella CHRROM
void mirroring_swap_JYASIC_base(WORD address, DBWORD value) {
	memmap_nmt_chrrom_1k(MMPPU(address), value);
	memmap_nmt_chrrom_1k(MMPPU(address | 0x1000), value);
}

INLINE static void irq_clock_prescaler_JYASIC(void) {
	BYTE type = 0;

	if (!jyasic.irq.active) {
		return;
	}

	type = jyasic.irq.mode >> 6;

	if (type == 1) {
		if ((++jyasic.irq.prescaler & jyasic.irq.premask) == 0) {
			if (!(jyasic.irq.mode & 0x08)) {
				jyasic.irq.count++;
			}
			if (jyasic.irq.count == 0x00) {
				irq.high |= EXT_IRQ;
			}
		}
	} else if (type == 2) {
		if ((--jyasic.irq.prescaler & jyasic.irq.premask) == jyasic.irq.premask) {
			if (!(jyasic.irq.mode & 0x08)) {
				jyasic.irq.count--;
			}
			if (jyasic.irq.count == 0xFF) {
				irq.high |= EXT_IRQ;
			}
		}
	}
}
INLINE static BYTE prg_reverse_JYASIC(BYTE value) {
	 return(
	 	// 0x40
		((value & 0x01) << 6) |
		// 0x20
		((value & 0x02) << 4) |
		// 0x10
		((value & 0x04) << 2) |
		// 0x08
		((value & 0x08) << 0) |
		// 0x04
		((value & 0x10) >> 2) |
		// 0x02
		((value & 0x20) >> 4) |
		// 0x01
		((value & 0x40) >> 6));
}
