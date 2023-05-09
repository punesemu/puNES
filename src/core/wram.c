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
#include <stdlib.h>
#include <errno.h>
#include "info.h"
#include "mem_map.h"
#include "gui.h"
#include "tas.h"
#include "cpu.h"
#include "mappers.h"
#include "vs_system.h"

void wram_load_nvram_file(void);
void wram_file_nvram(uTCHAR *prg_ram_file);

BYTE wram_malloc(uint32_t size);

INLINE static void wram_wp_chunk(const WORD slot,
	const DBWORD value, BYTE *dst, const size_t size,
	BYTE type, const BYTE rd, const BYTE wr);
INLINE static void wram_wp_banks(const WORD address,
	const DBWORD value, BYTE *dst, const size_t size,
	BYTE type, const BYTE rd, const BYTE wr, const size_t bank_size);
INLINE static BYTE slot_from_address(const WORD address);
INLINE static size_t mask_address_with_size(const size_t value, const size_t size);
INLINE static unsigned int shift_from_size(const size_t size);

_wram wram;
_trainer trainer;

BYTE wram_init(void) {
	wram.shift = shift_from_size(WRAM_CHUNK_SIZE);

	// se non ci sono stati settaggi particolari della mapper
	// devono esserci banchi di PRG Ram extra allora li assegno.
	if ((info.reset == CHANGE_ROM) || (info.reset == POWER_UP)) {
		if (prg_wram_size() && !wram.data) {
			// alloco la memoria necessaria
			if (wram_malloc(prg_wram_size()) == EXIT_ERROR) {
				return (EXIT_ERROR);
			}

			// resetto la wram
			wram_reset();

			// inizializzo il pointer della ram
			wram.ram.pnt = prg_wram_nvram_size() ? &wram.data[prg_wram_nvram_size()] : NULL;
			// inizializzo il pointer della nvram
			wram.nvram.pnt = prg_wram_nvram_size() ? &wram.data[0] : NULL;
		}
		// se esiste nvram la carico
		wram_load_nvram_file();
	}
	if ((info.reset >= HARD) && prg_wram_size()) {
		// Note :
		// 1 - non devo inizializzare la nvram
		// 2 - Idemitsu - Space College - Kikenbutsu no Yasashii Butsuri to Kagaku (Japan).nes
		//     se la ram non è valorizzata a 0 da errore 3 al controllo della memoria
		memset(wram.data, 0x00, prg_wram_ram_size());
		//emu_initial_ram(wram.data, prg_wram_ram_size());
	}
	return (EXIT_OK);
}
void wram_quit(void) {
	wram_trainer_quit();
	wram_save_nvram_file();
	if (wram.data) {
		free(wram.data);
	}
	memset(&wram, 0x00, sizeof(wram));
}
void wram_set_ram_size(size_t size) {
	wram.ram.size = size;
	wram.size = prg_wram_ram_size() + prg_wram_nvram_size();
}
void wram_set_nvram_size(size_t size) {
	wram.nvram.size = size;
	wram.size = prg_wram_ram_size() + prg_wram_nvram_size();
}
BYTE wram_is_writable(WORD address) {
	return (wram.chunk[slot_from_address(address)].writable);
}
void wram_reset(void) {
	// disabilito la wram da 0x4000 a 0x5FFF
	wram_map_disable_8k(0x4000);
	// setto i primi 8k su 0x6000
	wram_map_auto_8k(0x6000, 0);
}

BYTE wram_rd(const WORD address, const BYTE before) {
	const int slot = slot_from_address(address);

	if (address < 0x6000) {
		if ((vs_system.enabled) && (address >= 0x4020)) {
			if ((address & 0x4020) == 0x4020) {
				vs_system_r4020_clock(rd, before)
			}
			if (vs_system.special_mode.r5e0x) {
				if (address == 0x5E00) {
					vs_system.special_mode.index = 0;
				} else if (address == 0x5E01) {
					return (vs_system.special_mode.r5e0x[(vs_system.special_mode.index++) & 0x1F]);
				}
			} else if (vs_system.special_mode.type == VS_SM_Super_Xevious) {
				if (address == 0x54FF) {
					return (0x05);
				} else if (address == 0x5678) {
					return (vs_system.special_mode.index ? 0x00 : 0x01);
				} else if (address == 0x578F) {
					return(vs_system.special_mode.index ? 0xD1 : 0x89);
				} else if (address == 0x5567) {
					vs_system.special_mode.index ^= 1;
					return (vs_system.special_mode.index ? 0x37 : 0x3E);
				}
			}
		}
	}
	if (!wram.chunk[slot].pnt) {
		// TODO : devo controllare se lo fa davvero
		if (vs_system.enabled && vs_system.shared_mem) {
			return (prg.ram.data[address & 0x07FF]);
		}
	} else if (wram.chunk[slot].readable) {
		return (wram.chunk[slot].pnt[address & wram.chunk[slot].mask]);
	}
	return (cpu.openbus);
}
void wram_wr(const WORD address, const BYTE value) {
	const int slot = slot_from_address(address);

	if (address < 0x6000) {
		if (vs_system.enabled) {
			if ((address >= 0x4020) && ((address & 0x4020) == 0x4020)) {
				vs_system_r4020_clock(wr, value)
			}
		}
	}
	if (!wram.chunk[slot].pnt) {
		// TODO : devo controllare se lo fa davvero
		if (vs_system.enabled && vs_system.shared_mem) {
			prg.ram.data[address & 0x07FF] = value;
		}
	} else if (wram.chunk[slot].writable) {
		wram.chunk[slot].pnt[address & wram.chunk[slot].mask] = value;
	}
}

BYTE wram_direct_rd(const WORD address, const BYTE openbus) {
	return (wram.data ? wram.data[mask_address_with_size(address, wram.size)] : openbus);
}
void wram_direct_wr(const WORD address, const BYTE value) {
	if (wram.data) {
		wram.data[mask_address_with_size(address, wram.size)] = value;
	}
}

// with permissions
INLINE static void wram_map_auto_wp(const WORD address, const DBWORD value,
	const BYTE rd, const BYTE wr, const size_t bank_size) {
	wram_wp_banks(address, value, wram.data, prg_wram_size(), WRAM_BANK_RAM, rd, wr, bank_size);
}
void wram_map_auto_wp_256b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_auto_wp(address, value, rd, wr, 0x0100);
}
void wram_map_auto_wp_512b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_auto_wp(address, value, rd, wr, 0x0200);
}
void wram_map_auto_wp_1k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_auto_wp(address, value, rd, wr, 0x0400);
}
void wram_map_auto_wp_2k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_auto_wp(address, value, rd, wr, 0x0800);
}
void wram_map_auto_wp_4k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_auto_wp(address, value, rd, wr, 0x1000);
}
void wram_map_auto_wp_8k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_auto_wp(address, value, rd, wr, 0x2000);
}
void wram_map_auto_wp_16k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_auto_wp(address, value, rd, wr, 0x4000);
}

// permissions : rd = TRUE, wr = TRUE
INLINE static void wram_map_auto(const WORD address, const DBWORD value, const size_t bank_size) {
	wram_map_auto_wp(address, value, TRUE, TRUE, bank_size);
}
void wram_map_auto_256b(const WORD address, const DBWORD value) {
	wram_map_auto(address, value, 0x0100);
}
void wram_map_auto_512b(const WORD address, const DBWORD value) {
	wram_map_auto(address, value, 0x0200);
}
void wram_map_auto_1k(const WORD address, const DBWORD value) {
	wram_map_auto(address, value, 0x0400);
}
void wram_map_auto_2k(const WORD address, const DBWORD value) {
	wram_map_auto(address, value, 0x0800);
}
void wram_map_auto_4k(const WORD address, const DBWORD value) {
	wram_map_auto(address, value, 0x1000);
}
void wram_map_auto_8k(const WORD address, const DBWORD value) {
	wram_map_auto(address, value, 0x2000);
}
void wram_map_auto_16k(const WORD address, const DBWORD value) {
	wram_map_auto(address, value, 0x4000);
}

// permissions : rd = TRUE, wr = FALSE
INLINE static void wram_map_prg_rom(const WORD address, const DBWORD value, const size_t bank_size) {
	const size_t prg_rom_size = (size_t)info.prg.rom.banks_8k * 0x2000;

	wram_wp_banks(address, value, prg_pnt(0), prg_rom_size, WRAM_BANK_PRGROM, TRUE, FALSE, bank_size);
}
void wram_map_prg_rom_256b(const WORD address, DBWORD value) {
	wram_map_prg_rom(address, value, 0x0100);
}
void wram_map_prg_rom_512b(const WORD address, DBWORD value) {
	wram_map_prg_rom(address, value, 0x0200);
}
void wram_map_prg_rom_1k(const WORD address, DBWORD value) {
	wram_map_prg_rom(address, value, 0x0400);
}
void wram_map_prg_rom_2k(const WORD address, const DBWORD value) {
	wram_map_prg_rom(address, value, 0x0800);
}
void wram_map_prg_rom_4k(const WORD address, const DBWORD value) {
	wram_map_prg_rom(address, value, 0x1000);
}
void wram_map_prg_rom_8k(const WORD address, const DBWORD value) {
	wram_map_prg_rom(address, value, 0x2000);
}
void wram_map_prg_rom_16k(const WORD address, const DBWORD value) {
	wram_map_prg_rom(address, value, 0x4000);
}

// permissions : rd = FALSE, wr = FALSE
INLINE static void wram_map_disable(const WORD address, const size_t bank_size) {
	wram_wp_banks(address, 0, NULL, 0, WRAM_BANK_NONE, FALSE, FALSE, bank_size);
}
void wram_map_disable_256b(const WORD address) {
	wram_map_disable(address, 0x0100);
}
void wram_map_disable_512b(const WORD address) {
	wram_map_disable(address, 0x0200);
}
void wram_map_disable_1k(const WORD address) {
	wram_map_disable(address, 0x0400);
}
void wram_map_disable_2k(const WORD address) {
	wram_map_disable(address, 0x0800);
}
void wram_map_disable_4k(const WORD address) {
	wram_map_disable(address, 0x1000);
}
void wram_map_disable_8k(const WORD address) {
	wram_map_disable(address, 0x2000);
}
void wram_map_disable_16k(const WORD address) {
	wram_map_disable(address, 0x4000);
}

// with permissions
INLINE static void wram_map_ram_wp(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr, const size_t bank_size) {
	wram_wp_banks(address, value, prg_wram_ram_pnt(), prg_wram_ram_size(), WRAM_BANK_RAM, rd, wr, bank_size);
}
void wram_map_ram_wp_256b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_ram_wp(address, value, rd, wr, 0x0100);
}
void wram_map_ram_wp_512b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_ram_wp(address, value, rd, wr, 0x0200);
}
void wram_map_ram_wp_1k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_ram_wp(address, value, rd, wr, 0x0400);
}
void wram_map_ram_wp_2k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_ram_wp(address, value, rd, wr, 0x0800);
}
void wram_map_ram_wp_4k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_ram_wp(address, value, rd, wr, 0x1000);
}
void wram_map_ram_wp_8k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_ram_wp(address, value, rd, wr, 0x2000);
}
void wram_map_ram_wp_16k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_ram_wp(address, value, rd, wr, 0x4000);
}

// with permissions
INLINE static void wram_map_nvram_wp(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr, const size_t bank_size) {
	wram_wp_banks(address, value, prg_wram_nvram_pnt(), prg_wram_nvram_size(), WRAM_BANK_RAM, rd, wr, bank_size);
}
void wram_map_nvram_wp_256b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_nvram_wp(address, value, rd, wr, 0x0100);
}
void wram_map_nvram_wp_512b(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_nvram_wp(address, value, rd, wr, 0x0200);
}
void wram_map_nvram_wp_1k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_nvram_wp(address, value, rd, wr, 0x0400);
}
void wram_map_nvram_wp_2k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_nvram_wp(address, value, rd, wr, 0x0800);
}
void wram_map_nvram_wp_4k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_nvram_wp(address, value, rd, wr, 0x1000);
}
void wram_map_nvram_wp_8k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_nvram_wp(address, value, rd, wr, 0x2000);
}
void wram_map_nvram_wp_16k(const WORD address, const DBWORD value, const BYTE rd, const BYTE wr) {
	wram_map_nvram_wp(address, value, rd, wr, 0x4000);
}

// with permissions
INLINE static void wram_map_other(const WORD address, DBWORD value, BYTE *dst,
	const size_t size, const BYTE rd, const BYTE wr, const size_t bank_size) {
	wram_wp_banks(address, value, dst, size, WRAM_BANK_OTHER, rd, wr, bank_size);
}
void wram_map_other_256b(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr) {
	wram_map_other(address, value, dst, size, rd, wr, 0x0100);
}
void wram_map_other_512b(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr) {
	wram_map_other(address, value, dst, size, rd, wr, 0x0200);
}
void wram_map_other_1k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr) {
	wram_map_other(address, value, dst, size, rd, wr, 0x0400);
}
void wram_map_other_2k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr) {
	wram_map_other(address, value, dst, size, rd, wr, 0x0800);
}
void wram_map_other_4k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr) {
	wram_map_other(address, value, dst, size, rd, wr, 0x1000);
}
void wram_map_other_8k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr) {
	wram_map_other(address, value, dst, size, rd, wr, 0x2000);
}
void wram_map_other_16k(const WORD address, DBWORD value, BYTE *dst, const size_t size, const BYTE rd, const BYTE wr) {
	wram_map_other(address, value, dst, size, rd, wr, 0x4000);
}

void wram_load_nvram_file(void) {
	if (prg_wram_nvram_size() || info.mapper.force_battery_io) {
		uTCHAR prg_ram_file[LENGTH_FILE_NAME_LONG];
		FILE *fp = NULL;

		// estraggo il nome del file
		wram_file_nvram(&prg_ram_file[0]);

		// provo ad aprire il file
		fp = ufopen(prg_ram_file, uL("rb"));
		if (fp) {
			if (tas.type == NOTAS) {
				if (prg_wram_nvram_pnt()) {
					// leggo il contenuto della nvram
					if (fread(prg_wram_nvram_pnt(), prg_wram_nvram_size(), 1, fp) < 1) {
						log_error(uL("mapper;error on read battery memory (%s)"), strerror(errno));
						fclose(fp);
						return;
					}
				}
				if (extcl_battery_io) {
					extcl_battery_io(RD_BAT, fp);
				}
			}
			// chiudo il file
			fclose(fp);
		}
	}
}
void wram_save_nvram_file(void) {
	if (prg_wram_nvram_size() || info.mapper.force_battery_io) {
		uTCHAR prg_ram_file[LENGTH_FILE_NAME_LONG];
		FILE *fp = NULL;

		// estraggo il nome del file
		wram_file_nvram(&prg_ram_file[0]);

		// apro il file
		fp = ufopen(prg_ram_file, uL("w+b"));
		if (fp) {
			if (tas.type == NOTAS) {
				if (prg_wram_nvram_pnt()) {
					// scrivo il contenuto della nvram
					if (fwrite(prg_wram_nvram_pnt(), prg_wram_nvram_size(), 1, fp) < 1) {
						log_error(uL("mapper;error on write battery memory (%s)"), strerror(errno));
						fclose(fp);
						return;
					}
				}
				if (extcl_battery_io) {
					extcl_battery_io(WR_BAT, fp);
				}
			}
			// forzo la scrittura del file
			fflush(fp);
			// chiudo
			fclose(fp);
		}
	}
}
void wram_file_nvram(uTCHAR *prg_ram_file) {
	uTCHAR basename[255], *fl = info.rom.file, *last_dot = NULL;

	gui_utf_basename(fl, basename, usizeof(basename));
	usnprintf(prg_ram_file, LENGTH_FILE_NAME_LONG, uL("" uPs("") PRB_FOLDER "/" uPs("")), gui_data_folder(), basename);

	// rintraccio l'ultimo '.' nel nome
	last_dot = ustrrchr(prg_ram_file, uL('.'));
	if (last_dot) {
		// elimino l'estensione
		(*last_dot) = 0x00;
	}
	// aggiungo l'estensione prb
	ustrcat(prg_ram_file, uL(".prb"));
}

BYTE wram_malloc(uint32_t size) {
	wram.size = size;
	wram.data = (BYTE *)malloc(prg_wram_size());
	if (!wram.data) {
		log_error(uL("prg wram malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

void wram_trainer_init(void) {
	if (info.reset >= HARD) {
		if (info.mapper.trainer) {
			BYTE *here = prg.ram.data + 0x1000;

			if (prg_wram_size() >= 0x2000) {
				here = wram.data + 0x1000;
			}
			if (trainer.where_to_copy) {
				here = trainer.where_to_copy;
			}
			memcpy(here, trainer.data, trainer.size);
		}
	}
}
void wram_trainer_quit(void) {
	if (trainer.data) {
		free(trainer.data);
		trainer.data = NULL;
	}
	trainer.size = 0;
	trainer.where_to_copy = NULL;
}
BYTE wram_trainer_malloc(uint32_t size) {
	trainer.size = size;
	trainer.data = malloc(trainer.size);
	if (!trainer.data) {
		log_error(uL("trainer malloc;out of memory"));
		return (EXIT_ERROR);
	}
	return (EXIT_OK);
}

INLINE static void wram_wp_chunk(const WORD slot,
	const DBWORD value, BYTE *dst, const size_t size,
	BYTE type, const BYTE rd, const BYTE wr) {
	wram.chunk[slot].type = !dst ? WRAM_BANK_NONE : type;
	wram.chunk[slot].writable = !dst ? FALSE : wr;
	wram.chunk[slot].readable = !dst ? FALSE : rd;
	wram.chunk[slot].pnt = !dst ? NULL : &dst[mask_address_with_size(value << wram.shift, size)];
	wram.chunk[slot].mask = !dst ? 0 : mask_address_with_size(WRAM_CHUNK_SIZE - 1, size);
}
INLINE static void wram_wp_banks(const WORD address,
	const DBWORD value, BYTE *dst, const size_t size,
	BYTE type, const BYTE rd, const BYTE wr, const size_t bank_size) {
	const size_t bank = value * (bank_size / WRAM_CHUNK_SIZE);
	const int slot = slot_from_address(address);

	for (int i = 0; i < (int)(bank_size / WRAM_CHUNK_SIZE); i++) {
		wram_wp_chunk(slot + i, bank | i, dst, size, type, rd, wr);
	}
}

INLINE static BYTE slot_from_address(const WORD address) {
	return ((address >> wram.shift) & (WRAM_CHUNK_BANKS - 1));
}
INLINE static size_t mask_address_with_size(const size_t value, const size_t size) {
	return (value & (size - 1));
}
INLINE static unsigned int shift_from_size(const size_t size) {
	unsigned int shift = 0, pot = 1;

	while (pot < size) {
		pot <<= 1;
		shift++;
	}
	return (shift);
}
