/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include "external_calls.h"

// viene chiamata dopo il map_init(), map_prg_ram_init() e map_chr_ram_init()
void (*extcl_after_mapper_init)(void);
void (*extcl_cpu_wr_mem)(WORD address, BYTE value);
BYTE (*extcl_cpu_rd_mem)(WORD address, BYTE openbus, BYTE before);
BYTE (*extcl_save_mapper)(BYTE mode, BYTE slot, FILE *fp);
// CPU
void (*extcl_cpu_every_cycle)(void);
// viene chiamata ogni volta si scrive qualcosa nel registro $4016
void (*extcl_cpu_wr_r4016)(BYTE value);
// PPU
// viene chiamata sempre, ad ogni ciclo della PPU
void (*extcl_ppu_000_to_34x)(void);
// viene chiamata se (!ppu.vblank && (ppu.screen_y < SCR_ROWS))
// quindi per essere sicuri di essere durante il rendering della PPU
// nella funzione devo controllare anche se r2001.visible non e' a zero.
void (*extcl_ppu_000_to_255)(void);
// vengono chiamate solo se la PPU e' in fase di rendering
// (!ppu.vblank && r2001.visible && (ppu.screen_y < SCR_ROWS))
void (*extcl_ppu_256_to_319)(void);
void (*extcl_ppu_320_to_34x)(void);
// viene chiamata ogni volta viene modificato ppu.screen_y
void (*extcl_ppu_update_screen_y)(void);
// viene chiamata dopo ogni cambiamento del $2006 in cpu_inline.h
void (*extcl_update_r2006)(WORD new_r2006, WORD old_r2006);
// viene chiamata alla lettura del $2007 in cpu_inline.h
void (*extcl_rd_r2007)(void);
// vengono chiamate in ppu_inline.h
void (*extcl_rd_ppu)(WORD address);
BYTE (*extcl_rd_nmt)(WORD address);
BYTE (*extcl_rd_chr)(WORD address);
// viene chiamata dopo il FETCHB e dopo il fetch dello sprite
void (*extcl_after_rd_chr)(WORD address);
// viene chiamato quando si tenta di scrivere nella Nametable Ram
void (*extcl_wr_nmt)(WORD address, BYTE value);
// viene chiamato quando si tenta di scrivere nella CHR Ram
void (*extcl_wr_chr)(WORD address, BYTE value);
// APU
void (*extcl_length_clock)(void);
void (*extcl_envelope_clock)(void);
void (*extcl_apu_tick)(void);
// irqA12
void (*extcl_irq_A12_clock)(void);
// battery
void (*extcl_battery_io)(BYTE mode, FILE *fp);
// audio
void (*extcl_audio_samples_mod)(SWORD *samples, int count);

void extcl_init(void) {
	/* Mappers */
	extcl_after_mapper_init = NULL;
	extcl_cpu_wr_mem = NULL;
	extcl_cpu_rd_mem = NULL;
	extcl_save_mapper = NULL;
	/* CPU */
	extcl_cpu_every_cycle = NULL;
	extcl_cpu_wr_r4016 = NULL;
	/* PPU */
	extcl_ppu_000_to_34x = NULL;
	extcl_ppu_000_to_255 = NULL;
	extcl_ppu_256_to_319 = NULL;
	extcl_ppu_320_to_34x = NULL;
	extcl_ppu_update_screen_y = NULL;
	extcl_update_r2006 = NULL;
	extcl_rd_r2007 = NULL;
	extcl_after_rd_chr = NULL;
	extcl_rd_ppu = NULL;
	extcl_rd_nmt = NULL;
	extcl_rd_chr = NULL;
	extcl_wr_nmt = NULL;
	extcl_wr_chr = NULL;
	/* APU */
	extcl_length_clock = NULL;
	extcl_envelope_clock = NULL;
	extcl_apu_tick = NULL;
	/* irqA12 */
	extcl_irq_A12_clock = NULL;
	/* battery */
	extcl_battery_io = NULL;
	/* audio */
	extcl_audio_samples_mod = NULL;
}
