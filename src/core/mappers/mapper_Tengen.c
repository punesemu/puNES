/*
 * mapper_Tengen.c
 *
 *  Created on: 17/set/2011
 *      Author: fhorse
 */

#include <string.h>
#include "mappers.h"
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "irqA12.h"
#include "save_slot.h"

enum {
	A12_MODE,
	CPU_MODE,
	//tengen_rambo_delay_A12 = 4,
	//tengen_rambo_delay_CPU = 4,
	tengen_rambo_delay_A12 = 3,
	tengen_rambo_delay_CPU = 3,
};

#define tengen_rambo_prg_24k_update()\
{\
	BYTE bank[3];\
	if (tengen_rambo.prg_mode) {\
		bank[0] = tengen_rambo.prg[2];\
		bank[1] = tengen_rambo.prg[0];\
		bank[2] = tengen_rambo.prg[1];\
	} else {\
		bank[0] = tengen_rambo.prg[0];\
		bank[1] = tengen_rambo.prg[1];\
		bank[2] = tengen_rambo.prg[2];\
	}\
	map_prg_rom_8k(1, 0, bank[0]);\
	map_prg_rom_8k(1, 1, bank[1]);\
	map_prg_rom_8k(1, 2, bank[2]);\
	map_prg_rom_8k_update();\
}
#define tengen_rambo_chr_8k_update()\
{\
	BYTE bank[8] = { 0 };\
	if (tengen_rambo.chr_mode & 0x80) {\
		bank[0] = tengen_rambo.chr[2];\
		bank[1] = tengen_rambo.chr[3];\
		bank[2] = tengen_rambo.chr[4];\
		bank[3] = tengen_rambo.chr[5];\
		if (tengen_rambo.chr_mode & 0x20) {\
			tengen_rambo_chr_1k_control(0, 4);\
			bank[5] = tengen_rambo.chr[6];\
			tengen_rambo_chr_1k_control(1, 6);\
			bank[7] = tengen_rambo.chr[7];\
		} else {\
			tengen_rambo_chr_2k_control(0, 4);\
			tengen_rambo_chr_2k_control(1, 6);\
		}\
	} else {\
		bank[4] = tengen_rambo.chr[2];\
		bank[5] = tengen_rambo.chr[3];\
		bank[6] = tengen_rambo.chr[4];\
		bank[7] = tengen_rambo.chr[5];\
		if (tengen_rambo.chr_mode & 0x20) {\
			tengen_rambo_chr_1k_control(0, 0);\
			bank[1] = tengen_rambo.chr[6];\
			tengen_rambo_chr_1k_control(1, 2);\
			bank[3] = tengen_rambo.chr[7];\
		} else {\
			tengen_rambo_chr_2k_control(0, 0);\
			tengen_rambo_chr_2k_control(1, 2);\
		}\
	}\
	chr.bank_1k[0] = chr_chip_byte_pnt(0, bank[0] << 10);\
	chr.bank_1k[1] = chr_chip_byte_pnt(0, bank[1] << 10);\
	chr.bank_1k[2] = chr_chip_byte_pnt(0, bank[2] << 10);\
	chr.bank_1k[3] = chr_chip_byte_pnt(0, bank[3] << 10);\
	chr.bank_1k[4] = chr_chip_byte_pnt(0, bank[4] << 10);\
	chr.bank_1k[5] = chr_chip_byte_pnt(0, bank[5] << 10);\
	chr.bank_1k[6] = chr_chip_byte_pnt(0, bank[6] << 10);\
	chr.bank_1k[7] = chr_chip_byte_pnt(0, bank[7] << 10);\
}
#define tengen_rambo_chr_1k_control(chr_1k, bank_1k)\
	value = tengen_rambo.chr[chr_1k];\
	control_bank(info.chr.rom.max.banks_1k)\
	bank[bank_1k] = value
#define tengen_rambo_chr_2k_control(chr_2k, bank_2k)\
	value = tengen_rambo.chr[chr_2k] >> 1;\
	control_bank(info.chr.rom.max.banks_2k)\
	bank[bank_2k] = value << 1;\
	bank[bank_2k + 1] = bank[bank_2k] | 0x01

static void INLINE irq_clock_Tengen_Rambo(int delay);

BYTE type;

void map_init_Tengen(BYTE model) {
	switch (model) {
		case T800037:
		case TRAMBO:
			EXTCL_CPU_WR_MEM(Tengen_Rambo);
			EXTCL_SAVE_MAPPER(Tengen_Rambo);
			EXTCL_PPU_000_TO_34X(Tengen_Rambo);
			EXTCL_PPU_000_TO_255(MMC3);
			EXTCL_PPU_256_TO_319(MMC3);
			EXTCL_PPU_320_TO_34X(MMC3);
			EXTCL_UPDATE_R2006(Tengen_Rambo);
			EXTCL_IRQ_A12_CLOCK(Tengen_Rambo);
			EXTCL_CPU_EVERY_CYCLE(Tengen_Rambo);
			mapper.internal_struct[0] = (BYTE *) &tengen_rambo;
			mapper.internal_struct_size[0] = sizeof(tengen_rambo);
			mapper.internal_struct[1] = (BYTE *) &irqA12;
			mapper.internal_struct_size[1] = sizeof(irqA12);

			if (info.reset >= HARD) {
				BYTE i;

				memset(&tengen_rambo, 0x00, sizeof(tengen_rambo));
				memset(&irqA12, 0x00, sizeof(irqA12));
				tengen_rambo.irq_mode = CPU_MODE;

				for (i = 0; i < LENGTH(tengen_rambo.chr); i++) {
					tengen_rambo.chr[i] = i;
				}

				for (i = 0; i < LENGTH(tengen_rambo.prg); i++) {
					tengen_rambo.prg[i] = i;
				}
			} else {
				tengen_rambo.irq_mode = CPU_MODE;
			}

			irqA12.present = TRUE;

			{
				BYTE value = 0;
				tengen_rambo_prg_24k_update()
				tengen_rambo_chr_8k_update()
			}
			break;
	}

	type = model;
}

void extcl_cpu_wr_mem_Tengen_Rambo(WORD address, BYTE value) {
	switch (address & 0xF001) {
		case 0x8000:
			tengen_rambo.reg_index = value & 0x0F;
			if ((value & 0x40) != tengen_rambo.prg_mode) {
				tengen_rambo.prg_mode = value & 0x40;
				tengen_rambo_prg_24k_update()
			}
			if ((value & 0xA0) != tengen_rambo.chr_mode) {
				tengen_rambo.chr_mode = value & 0xA0;
				tengen_rambo_chr_8k_update()
			}
			return;
		case 0x8001: {
			switch (tengen_rambo.reg_index) {
				case 0x00:
				case 0x01:
					if ((type == T800037) && !(tengen_rambo.chr_mode & 0x80)) {
						const BYTE slot = tengen_rambo.reg_index << 1;

						ntbl.bank_1k[slot] = &ntbl.data[((value >> 7) ^ 0x01) << 10];
						ntbl.bank_1k[slot | 0x01] = ntbl.bank_1k[slot];
					}
					if (tengen_rambo.chr[tengen_rambo.reg_index] != value) {
						tengen_rambo.chr[tengen_rambo.reg_index] = value;
						tengen_rambo_chr_8k_update()
					}
					break;
				case 0x02:
				case 0x03:
				case 0x04:
				case 0x05:
					if ((type == T800037) && (tengen_rambo.chr_mode & 0x80)) {
						ntbl.bank_1k[tengen_rambo.reg_index - 2] = &ntbl.data[((value >> 7) ^ 0x01)
							<< 10];
					}
					control_bank(info.chr.rom.max.banks_1k)
					if (tengen_rambo.chr[tengen_rambo.reg_index] != value) {
						tengen_rambo.chr[tengen_rambo.reg_index] = value;
						tengen_rambo_chr_8k_update()
					}
					break;
				case 0x06:
				case 0x07: {
					const BYTE index = tengen_rambo.reg_index & 0x01;

					control_bank(info.prg.rom.max.banks_8k)
					if (tengen_rambo.prg[index] != value) {
						tengen_rambo.prg[index] = value;
						tengen_rambo_prg_24k_update()
					}
					break;
				}
				case 0x08:
				case 0x09: {
					const BYTE index = tengen_rambo.reg_index - 0x02;

					control_bank(info.chr.rom.max.banks_1k)
					if (tengen_rambo.chr[index] != value) {
						tengen_rambo.chr[index] = value;
						tengen_rambo_chr_8k_update()
					}
					break;
				}
				case 0x0F:
					control_bank(info.prg.rom.max.banks_8k)
					if (tengen_rambo.prg[2] != value) {
						tengen_rambo.prg[2] = value;
						tengen_rambo_prg_24k_update()
					}
					break;
			}
			return;
		}
		case 0xA000:
			if (type == T800037) {
				return;
			}
			if (value & 0x01) {
				mirroring_H();
			} else {
				mirroring_V();
			}
			return;
		case 0xC000:
			irqA12.latch = value;
			return;
		case 0xC001: {
			irqA12.reload = TRUE;

			if (((value & 0x01) == A12_MODE) && (tengen_rambo.irq_mode == CPU_MODE)) {
				tengen_rambo.irq_force_clock = TRUE;
			}

			if (value & 0x01) {
				/* abilito' la modalita' CPU cycles */
				tengen_rambo.irq_mode = CPU_MODE;
				tengen_rambo.irq_prescaler = 0;
			} else {
				/* abilito' la modalita' A12 */
				tengen_rambo.irq_mode = A12_MODE;
			}
			return;
		}
		case 0xE000:
			irqA12.enable = FALSE;
			irq.high &= ~EXT_IRQ;
			return;
		case 0xE001:
			irqA12.enable = TRUE;
			return;
		default:
			return;
	}
}
BYTE extcl_save_mapper_Tengen_Rambo(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, tengen_rambo.prg_mode);
	save_slot_ele(mode, slot, tengen_rambo.chr_mode);
	save_slot_ele(mode, slot, tengen_rambo.reg_index);
	save_slot_ele(mode, slot, tengen_rambo.chr);
	save_slot_ele(mode, slot, tengen_rambo.prg);
	save_slot_ele(mode, slot, tengen_rambo.irq_mode);
	save_slot_ele(mode, slot, tengen_rambo.irq_delay);
	save_slot_ele(mode, slot, tengen_rambo.irq_prescaler);

	return (EXIT_OK);
}
void extcl_ppu_000_to_34x_Tengen_Rambo(void) {
	if (tengen_rambo.irq_delay && !(--tengen_rambo.irq_delay)) {
		irq.high |= EXT_IRQ;
	}

	irqA12_RS();
}
void extcl_update_r2006_Tengen_Rambo(WORD value_old) {
	if (!(value_old & 0x1000) && (r2006.value & 0x1000)) {
		extcl_irq_A12_clock_Tengen_Rambo();
	}
}
void extcl_irq_A12_clock_Tengen_Rambo(void) {
	if (tengen_rambo.irq_mode != A12_MODE) {
		return;
	}

	if (irqA12.cycles > irqA12_min_cpu_cycles_prev_rising_edge) {
		irqA12.cycles = 0;
		irq_clock_Tengen_Rambo(tengen_rambo_delay_A12);
	}
}
void extcl_cpu_every_cycle_Tengen_Rambo(void) {
	if ((tengen_rambo.irq_mode != CPU_MODE) && (tengen_rambo.irq_force_clock == FALSE)) {
	//if (tengen_rambo.irq_mode != CPU_MODE) {
		return;
	}

	if (tengen_rambo.irq_prescaler < 3) {
		tengen_rambo.irq_prescaler++;
		return;
	}
	tengen_rambo.irq_prescaler = 0;
	tengen_rambo.irq_force_clock = FALSE;

	irq_clock_Tengen_Rambo(tengen_rambo_delay_CPU);
}

static void INLINE irq_clock_Tengen_Rambo(int delay) {
	/**/
	if (irqA12.reload == TRUE) {
		if (irqA12.latch <= 1) {
			irqA12.counter = irqA12.latch + 1;
		} else {
			irqA12.counter = irqA12.latch + 2;
		}
		irqA12.reload = FALSE;
	} else if (!irqA12.counter) {
		irqA12.counter = irqA12.latch + 1;
	}
	/**/

	/*
	if (irqA12.reload == TRUE) {
		if (irqA12.latch == 0) {
			irqA12.counter = irqA12.latch + 1;
		} else {
			irqA12.counter = irqA12.latch + 1;
		}

		irqA12.reload = FALSE;
	} else if (!irqA12.counter) {
		irqA12.counter = irqA12.latch + 2;
	}
	*/

	irqA12.counter--;
	if (!irqA12.counter && irqA12.enable) {
		tengen_rambo.irq_delay = delay;
	}
}
