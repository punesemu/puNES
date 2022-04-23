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
#include "info.h"
#include "mem_map.h"
#include "cpu.h"
#include "ppu.h"
#include "save_slot.h"
#include "tas.h"

enum {
	MODE_IDLE,
	MODE_DATA,
	MODE_ADDRESS,
	MODE_READ,
	MODE_WRITE,
	MODE_ACK,
	MODE_NOT_ACK,
	MODE_ACK_WAIT,
	MODE_MAX
};

#define b161x02x74_chr_4k_update()\
	value = (save & 0x04) | (b161x02x74.chr_rom_bank & 0x03);\
	control_bank(bandaitmp.chr_ram_4k_max)\
	b161x02x74.chr_rom_bank = value;\
	bank = value << 12;\
	chr.bank_1k[0] = chr_pnt(bank);\
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);\
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);\
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);\
	value = (save & 0x04) | 0x03;\
	control_bank(bandaitmp.chr_ram_4k_max)\
	bank = value << 12;\
	chr.bank_1k[4] = chr_pnt(bank);\
	chr.bank_1k[5] = chr_pnt(bank | 0x0400);\
	chr.bank_1k[6] = chr_pnt(bank | 0x0800);\
	chr.bank_1k[7] = chr_pnt(bank | 0x0C00)
#define b16x02x74_r2006(adr)\
{\
	const BYTE value = (b161x02x74.chr_rom_bank & 0x04) | ((adr >> 8) & 0x03);\
	const DBWORD bank = value << 12;\
	b161x02x74.chr_rom_bank = value;\
	chr.bank_1k[0] = chr_pnt(bank);\
	chr.bank_1k[1] = chr_pnt(bank | 0x0400);\
	chr.bank_1k[2] = chr_pnt(bank | 0x0800);\
	chr.bank_1k[3] = chr_pnt(bank | 0x0C00);\
}

#define datach_set_scl(scl) e24C0x_set(scl, FCGX.e1.sda, &FCGX.e1)
#define datach_set_sda(sda) e24C0x_set(FCGX.e1.scl, sda, &FCGX.e1)
#define e24C0x_save(epr)\
	save_slot_ele(mode, slot, epr.eeprom);\
	save_slot_ele(mode, slot, epr.size);\
	save_slot_ele(mode, slot, epr.mode);\
	save_slot_ele(mode, slot, epr.next);\
	save_slot_ele(mode, slot, epr.bit);\
	save_slot_ele(mode, slot, epr.address);\
	save_slot_ele(mode, slot, epr.data);\
	save_slot_ele(mode, slot, epr.scl);\
	save_slot_ele(mode, slot, epr.sda);\
	save_slot_ele(mode, slot, epr.rw);\
	save_slot_ele(mode, slot, epr.output)

typedef struct _FCGXeeprom {
	BYTE eeprom[256];
	WORD size;
	BYTE mode;
	BYTE next;
	BYTE bit;
	BYTE address;
	BYTE data;
	BYTE scl;
	BYTE sda;
	BYTE rw;
	BYTE output;
} _FCGXeeprom;

void e24C0x_set(BYTE scl, BYTE sda, _FCGXeeprom *eeprom);

struct _b161x02x74 {
	BYTE chr_rom_bank;
} b161x02x74;
struct _FCGX {
	BYTE reg[8];
	BYTE enabled;
	WORD count;
	WORD reload;
	BYTE delay;
	_FCGXeeprom e0;
	_FCGXeeprom e1;
} FCGX;
struct _bandaitmp {
	BYTE type;
	WORD chr_ram_4k_max;
} bandaitmp;

void map_init_Bandai(BYTE model) {
	bandaitmp.chr_ram_4k_max = info.chr.rom.banks_4k - 1;

	switch (model) {
		case B161X02X74:
			EXTCL_CPU_WR_MEM(Bandai_161x02x74);
			EXTCL_SAVE_MAPPER(Bandai_161x02x74);
			EXTCL_UPDATE_R2006(Bandai_161x02x74);
			EXTCL_RD_NMT(Bandai_161x02x74);
			mapper.internal_struct[0] = (BYTE *)&b161x02x74;
			mapper.internal_struct_size[0] = sizeof(b161x02x74);

			if (info.reset >= HARD) {
				b161x02x74.chr_rom_bank = 0;

				map_prg_rom_8k(4, 0, 0);

				{
					BYTE value, save = 0;
					DBWORD bank;

					b161x02x74_chr_4k_update();
				}
			}
			break;
		case FCGx:
		case E24C01:
		case E24C02:
		case DATACH: {
			EXTCL_CPU_WR_MEM(Bandai_FCGX);
			EXTCL_CPU_RD_MEM(Bandai_FCGX);
			EXTCL_SAVE_MAPPER(Bandai_FCGX);
			EXTCL_BATTERY_IO(Bandai_FCGX);
			EXTCL_CPU_EVERY_CYCLE(Bandai_FCGX);
			mapper.internal_struct[0] = (BYTE *)&FCGX;
			mapper.internal_struct_size[0] = sizeof(FCGX);

			info.mapper.extend_wr = TRUE;

			if (info.reset >= HARD) {
				memset(&FCGX, 0x00, sizeof(FCGX));
				FCGX.e0.output = FCGX.e1.output = 0x10;

				if (info.prg.rom.banks_16k >= 32) {
					map_prg_rom_8k(2, 2, info.prg.rom.max.banks_16k);
				}
			} else {
				BYTE i;
				for (i = 0; i < 8; i++) {
					FCGX.reg[i] = 0;
				}
			}

			switch (model) {
				case E24C01:
					info.prg.ram.bat.banks = TRUE;
					FCGX.e0.size = 128;
					break;
				case E24C02:
					info.prg.ram.bat.banks = TRUE;
					FCGX.e0.size = 256;
					break;
				case DATACH:
					info.prg.ram.bat.banks = TRUE;
					FCGX.e0.size = 256;
					FCGX.e1.size = 128;
					break;
			}
			break;
		}
	}

	switch (info.id) {
		case FAMICOMJUMPII:
			info.prg.ram.banks_8k_plus = 1;
			info.prg.ram.bat.banks = 1;
			break;
	}

	bandaitmp.type = model;
}

void extcl_cpu_wr_mem_Bandai_161x02x74(WORD address, BYTE value) {
	/* bus conflict */
	const BYTE save = value &= prg_rom_rd(address);
	DBWORD bank;

	control_bank_with_AND(0x03, info.prg.rom.max.banks_32k)
	map_prg_rom_8k(4, 0, value);
	map_prg_rom_8k_update();

	b161x02x74_chr_4k_update();
}
BYTE extcl_save_mapper_Bandai_161x02x74(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, b161x02x74.chr_rom_bank);

	return (EXIT_OK);
}
void extcl_update_r2006_Bandai_161x02x74(WORD new_r2006, UNUSED(WORD old_r2006)) {
	if ((new_r2006 >= 0x2000) && ((new_r2006 & 0x03FF) < 0x03C0)) {
		b16x02x74_r2006(new_r2006)
	}
}
BYTE extcl_rd_nmt_Bandai_161x02x74(WORD address) {
	if ((address & 0x03FF) < 0x03C0) {
		b16x02x74_r2006(address);
	}

	return (ntbl.bank_1k[address >> 10][address & 0x3FF]);
}

void extcl_cpu_wr_mem_Bandai_FCGX(WORD address, BYTE value) {
	if (address < 0x6000) {
		return;
	}

	if (!info.prg.ram.banks_8k_plus) {
		address |= 0x8000;
	}

	switch (address & 0x800F) {
		case 0x8000:
		case 0x8001:
		case 0x8002:
		case 0x8003:
		case 0x8004:
		case 0x8005:
		case 0x8006:
		case 0x8007: {
			const BYTE slot = address & 0x000F;

			if (info.prg.rom.banks_16k >= 32) {
				BYTE i;

				FCGX.reg[slot] = value;
				value = 0;
				for (i = 0; i < 8; i++) {
					value |= (FCGX.reg[i] << 4) & 0x10;
				}
				value |= ((mapper.rom_map_to[0] >> 1) & 0x0F);
				control_bank(info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 0, value);

				value |= 0x0F;
				control_bank(info.prg.rom.max.banks_16k)
				map_prg_rom_8k(2, 2, value);

				map_prg_rom_8k_update();

				value = FCGX.reg[slot];
			}
			if (bandaitmp.type == DATACH) {
				datach_set_scl((value << 2) & 0x20);
			}
			if (!mapper.write_vram) {
				control_bank(info.chr.rom.max.banks_1k)
				chr.bank_1k[slot] = chr_pnt(value << 10);
			}
			return;
		}
		case 0x8008:
			if (info.prg.rom.banks_16k >= 32) {
				value = ((mapper.rom_map_to[0] >> 1) & 0x10) | (value & 0x0F);
			}
			control_bank(info.prg.rom.max.banks_16k)
			map_prg_rom_8k(2, 0, value);
			map_prg_rom_8k_update();
			return;
		case 0x8009: {
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
		case 0x800A:
			FCGX.enabled = value & 0x01;
			FCGX.count = FCGX.reload;
			irq.high &= ~EXT_IRQ;
			return;
		case 0x800B:
			FCGX.reload = (FCGX.reload & 0xFF00) | value;
			return;
		case 0x800C:
			FCGX.reload = (FCGX.reload & 0x00FF) | (value << 8);
			return;
		case 0x800D:
			if (FCGX.e0.size) {
				e24C0x_set(value & 0x20, value & 0x40, &FCGX.e0);
				if (bandaitmp.type == DATACH) {
					datach_set_sda(value & 0x40);
				}
			}
			return;
	}
}
BYTE extcl_cpu_rd_mem_Bandai_FCGX(WORD address, BYTE openbus, UNUSED(BYTE before)) {
	if (!FCGX.e0.size || (address < 0x6000)) {
		return (openbus);
	}

	if (address & 0x0100) {
		BYTE value = FCGX.e0.output;

		if (bandaitmp.type == DATACH) {
			value &= FCGX.e1.output;
		}
		return (value);
	}

	return (openbus);
}
BYTE extcl_save_mapper_Bandai_FCGX(BYTE mode, BYTE slot, FILE *fp) {
	save_slot_ele(mode, slot, FCGX.reg);
	save_slot_ele(mode, slot, FCGX.enabled);
	save_slot_ele(mode, slot, FCGX.count);
	save_slot_ele(mode, slot, FCGX.reload);
	save_slot_ele(mode, slot, FCGX.delay);
	if (FCGX.e0.size) {
		e24C0x_save(FCGX.e0);
	}
	if (FCGX.e1.size) {
		e24C0x_save(FCGX.e1);
	}

	return (EXIT_OK);
}
void extcl_battery_io_Bandai_FCGX(BYTE mode, FILE *fp) {
	if ((mode == WR_BAT) && !fp) {
		return;
	}

	if (FCGX.e0.size || FCGX.e1.size) {
		if (tas.type == NOTAS) {
			if (FCGX.e0.size) {
				if (mode == WR_BAT) {
					if (fwrite(&FCGX.e0.eeprom[0], LENGTH(FCGX.e0.eeprom), 1, fp) < 1) {
						fprintf(stderr, "error on write battery memory\n");
					}
				} else {
					if (fread(&FCGX.e0.eeprom[0], LENGTH(FCGX.e0.eeprom), 1, fp) < 1) {
						fprintf(stderr, "error on read battery memory\n");
					}
				}
			}
			if (FCGX.e1.size) {
				if (mode == WR_BAT) {
					if (fwrite(&FCGX.e1.eeprom[0], LENGTH(FCGX.e1.eeprom), 1, fp) < 1) {
						fprintf(stderr, "error on write battery memory\n");
					}
				} else {
					if (fread(&FCGX.e1.eeprom[0], LENGTH(FCGX.e1.eeprom), 1, fp) < 1) {
						fprintf(stderr, "error on read battery memory\n");
					}
				}
			}
		}
	} else {
		if (mode == WR_BAT) {
			map_bat_wr_default(fp);
		} else {
			map_bat_rd_default(fp);
			/*
			 * ho notato che quando avvio per la prima volta
			 * "Famicom Jump II - Saikyou no 7 Nin (J) [!].nes", se questa zona di memoria
			 * non e' valorizzata almeno ad uno, il gioco va in loop. Con un reset (soft
			 * o hard non ha importanza) il gioco inizia a funzionare perche' questa
			 * locazione di memoria e' stata nel frattempo valorizzata dal gioco stesso.
			 */
			if (!fp && (info.id == FAMICOMJUMPII)) {
				prg.ram_battery[0xBBC] = 0x01;
			}
		}
	}
}
void extcl_cpu_every_cycle_Bandai_FCGX(void) {
	if (FCGX.delay && !(--FCGX.delay)) {
		irq.high |= EXT_IRQ;
	}

	if (!FCGX.enabled) {
		return;
	}

	if (FCGX.count && !(--FCGX.count)) {
		FCGX.delay = 1;
	}

}
void e24C0x_set(BYTE scl, BYTE sda, _FCGXeeprom *eeprom) {
	if (eeprom->scl && (sda < eeprom->sda)) {
		/* start */
		if (eeprom->size == 128) {
			eeprom->mode = MODE_ADDRESS;
			eeprom->bit = 0;
			eeprom->address = 0;
			eeprom->output = 0x10;
		} else {
			eeprom->mode = MODE_DATA;
			eeprom->bit = 0;
			eeprom->output = 0x10;
		}
	} else if (eeprom->scl && (sda > eeprom->sda)) {
		/* stop */
		eeprom->mode = MODE_IDLE;
		eeprom->output = 0x10;
	} else if (scl > eeprom->scl) {
		/*rise */
		BYTE bit = sda >> 6;

		if (bit <= 1) {
			if (eeprom->size == 128) {
				switch (eeprom->mode) {
					case MODE_ADDRESS:
						if (eeprom->bit < 7) {
							eeprom->address &= ~(0x01 << eeprom->bit);
							eeprom->address |= (bit << eeprom->bit++);
						} else if (eeprom->bit < 8) {
							eeprom->bit = 8;

							if (bit) {
								eeprom->next = MODE_READ;
								eeprom->data = eeprom->eeprom[eeprom->address];
							} else {
								eeprom->next = MODE_WRITE;
							}
						}
						break;
					case MODE_ACK:
						eeprom->output = 0x00;
						break;
					case MODE_READ:
						if (eeprom->bit < 8) {
							eeprom->output = (eeprom->data & 0x01 << eeprom->bit++) ? 0x10 : 0x00;
						}
						break;
					case MODE_WRITE:
						if (eeprom->bit < 8) {
							eeprom->data &= ~(0x01 << eeprom->bit);
							eeprom->data |= bit << eeprom->bit++;
						}
						break;
					case MODE_ACK_WAIT:
						if (!bit) {
							eeprom->next = MODE_IDLE;
						}
						break;
				}
			} else {
				switch (eeprom->mode) {
					case MODE_DATA:
						if (eeprom->bit < 8) {
							eeprom->data &= ~(0x01 << (7 - eeprom->bit));
							eeprom->data |= bit << (7 - eeprom->bit++);
						}
						break;
					case MODE_ADDRESS:
						if (eeprom->bit < 8) {
							eeprom->address &= ~(0x01 << (7 - eeprom->bit));
							eeprom->address |= bit << (7 - eeprom->bit++);
						}
						break;
					case MODE_READ:
						if (eeprom->bit < 8)
							eeprom->output =
									(eeprom->data & 0x01 << (7 - eeprom->bit++)) ? 0x10 : 0x00;
						break;
					case MODE_WRITE:
						if (eeprom->bit < 8) {
							eeprom->data &= ~(0x01 << (7 - eeprom->bit));
							eeprom->data |= bit << (7 - eeprom->bit++);
						}
						break;
					case MODE_NOT_ACK:
						eeprom->output = 0x10;
						break;
					case MODE_ACK:
						eeprom->output = 0x00;
						break;
					case MODE_ACK_WAIT:
						if (!bit) {
							eeprom->next = MODE_READ;
							eeprom->data = eeprom->eeprom[eeprom->address];
						}
						break;
				}
			}
		}
	} else if (scl < eeprom->scl) {
		/* fall */
		if (eeprom->size == 128) {
			switch (eeprom->mode) {
				case MODE_ADDRESS:
					if (eeprom->bit == 8) {
						eeprom->mode = MODE_ACK;
						eeprom->output = 0x10;
					}
					break;
				case MODE_ACK:
					eeprom->mode = eeprom->next;
					eeprom->bit = 0;
					eeprom->output = 0x10;
					break;
				case MODE_READ:
					if (eeprom->bit == 8) {
						eeprom->mode = MODE_ACK_WAIT;
						eeprom->address = (eeprom->address + 1) & 0x7F;
					}
					break;
				case MODE_WRITE:
					if (eeprom->bit == 8) {
						eeprom->mode = MODE_ACK;
						eeprom->next = MODE_IDLE;
						eeprom->eeprom[eeprom->address] = eeprom->data;
						eeprom->address = (eeprom->address + 1) & 0x7F;
					}
					break;
			}
		} else {
			switch (eeprom->mode) {
				case MODE_DATA:
					if (eeprom->bit == 8) {
						if ((eeprom->data & 0xA0) == 0xA0) {
							eeprom->bit = 0;
							eeprom->mode = MODE_ACK;
							eeprom->rw = eeprom->data & 0x01;
							eeprom->output = 0x10;
							if (eeprom->rw) {
								eeprom->next = MODE_READ;
								eeprom->data = eeprom->eeprom[eeprom->address];
							} else {
								eeprom->next = MODE_ADDRESS;
							}
						} else {
							eeprom->mode = MODE_NOT_ACK;
							eeprom->next = MODE_IDLE;
							eeprom->output = 0x10;
						}
					}
					break;
				case MODE_ADDRESS:
					if (eeprom->bit == 8) {
						eeprom->bit = 0;
						eeprom->mode = MODE_ACK;
						eeprom->next = (eeprom->rw ? MODE_IDLE : MODE_WRITE);
						eeprom->output = 0x10;
					}
					break;
				case MODE_READ:
					if (eeprom->bit == 8) {
						eeprom->mode = MODE_ACK_WAIT;
						eeprom->address = (eeprom->address + 1) & 0xFF;
					}
					break;
				case MODE_WRITE:
					if (eeprom->bit == 8) {
						eeprom->bit = 0;
						eeprom->mode = MODE_ACK;
						eeprom->next = MODE_WRITE;
						eeprom->eeprom[eeprom->address] = eeprom->data;
						eeprom->address = (eeprom->address + 1) & 0xFF;
					}
					break;
				case MODE_NOT_ACK:
					eeprom->mode = MODE_IDLE;
					eeprom->bit = 0;
					eeprom->output = 0x10;
					break;
				case MODE_ACK:
				case MODE_ACK_WAIT:
					eeprom->mode = eeprom->next;
					eeprom->bit = 0;
					eeprom->output = 0x10;
					break;
			}
		}
	}
	eeprom->scl = scl;
	eeprom->sda = sda;
}

