/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

// viene chiamata dopo il map_init()
void (*extcl_after_mapper_init)(void);
// viene chiamata dal mapper_quit()
void (*extcl_mapper_quit)(void);
void (*extcl_cpu_wr_mem)(BYTE nidx, WORD address, BYTE value);
BYTE (*extcl_cpu_rd_mem)(BYTE nidx, WORD address, BYTE openbus);
BYTE (*extcl_cpu_rd_ram)(BYTE nidx, WORD address, BYTE openbus);
BYTE (*extcl_save_mapper)(BYTE mode, BYTE slot, FILE *fp);
// CPU
// viene chimata subito dopo il cpu_init_pc
void (*extcl_cpu_init_pc)(BYTE nidx);
// viene chiamata ad ogni ciclo di cpu
void (*extcl_cpu_every_cycle)(BYTE nidx);
// viene chiamata ogni volta si scrive qualcosa nel registro $4016
void (*extcl_cpu_wr_r4016)(BYTE nidx, BYTE value);
// PPU
// viene chiamata sempre, ad ogni ciclo della PPU
void (*extcl_ppu_000_to_34x)(BYTE nidx);
// viene chiamata se (!nes[nidx].p.ppu.vblank && (nes[nidx].p.ppu.screen_y < SCR_ROWS))
// quindi per essere sicuri di essere durante il rendering della PPU
// nella funzione devo controllare anche se nes[nidx].p.r2001.visible non e' a zero.
void (*extcl_ppu_000_to_255)(BYTE nidx);
// vengono chiamate solo se la PPU e' in fase di rendering
// (!nes[nidx].p.ppu.vblank && nes[nidx].p.r2001.visible && (nes[nidx].p.ppu.screen_y < SCR_ROWS))
void (*extcl_ppu_256_to_319)(BYTE nidx);
void (*extcl_ppu_320_to_34x)(BYTE nidx);
// viene chiamata dopo ogni cambiamento del $2006 in cpu_inline.h
void (*extcl_update_r2006)(BYTE nidx, WORD new_r2006, WORD old_r2006);
// viene chiamata alla lettura del $2007 in cpu_inline.h
void (*extcl_rd_r2007)(BYTE nidx);
// vine chiamata in cpu_inline.h alla scrittura nei rigistri della ppu
BYTE (*extcl_wr_ppu_reg)(BYTE nidx, WORD address, BYTE *value);
// vengono chiamate in ppu_inline.h
void (*extcl_rd_ppu_mem)(BYTE nidx, WORD address);
BYTE (*extcl_rd_nmt)(BYTE nidx, WORD address);
BYTE (*extcl_rd_chr)(BYTE nidx, WORD address);
// viene chiamata dopo il FETCHB e dopo il fetch dello sprite
void (*extcl_after_rd_chr)(BYTE nidx, WORD address);
// viene chiamato quando si tenta di scrivere nella Nametable Ram
void (*extcl_wr_nmt)(BYTE nidx, WORD address, BYTE value);
// viene chiamato quando si tenta di scrivere nella CHR Ram
void (*extcl_wr_chr)(BYTE nidx, WORD address, BYTE value);
// APU
// vine chiamata in cpu_inline.h alla scrittura nei rigistri della apu
BYTE (*extcl_wr_apu)(BYTE nidx, WORD address, BYTE *value);
// vine chiamata in cpu_inline.h alla lettura dei rigistri della apu
BYTE (*extcl_rd_apu)(BYTE nidx, WORD address, BYTE openbus);
void (*extcl_length_clock)(void);
void (*extcl_envelope_clock)(void);
void (*extcl_apu_tick)(void);
// irqA12
void (*extcl_irq_A12_clock)(BYTE nidx);
// battery
void (*extcl_battery_io)(BYTE mode, FILE *fp);
// audio
void (*extcl_audio_samples_mod)(SWORD *samples, int count);

void extcl_init(void) {
	/* Mappers */
	extcl_after_mapper_init = NULL;
	extcl_mapper_quit = NULL;
	extcl_cpu_wr_mem = NULL;
	extcl_cpu_rd_mem = NULL;
	extcl_cpu_rd_ram = NULL;
	extcl_save_mapper = NULL;
	/* CPU */
	extcl_cpu_init_pc = NULL;
	extcl_cpu_every_cycle = NULL;
	extcl_cpu_wr_r4016 = NULL;
	/* PPU */
	extcl_ppu_000_to_34x = NULL;
	extcl_ppu_000_to_255 = NULL;
	extcl_ppu_256_to_319 = NULL;
	extcl_ppu_320_to_34x = NULL;
	extcl_update_r2006 = NULL;
	extcl_rd_r2007 = NULL;
	extcl_after_rd_chr = NULL;
	extcl_wr_ppu_reg = NULL;
	extcl_rd_ppu_mem = NULL;
	extcl_rd_nmt = NULL;
	extcl_rd_chr = NULL;
	extcl_wr_nmt = NULL;
	extcl_wr_chr = NULL;
	/* APU */
	extcl_wr_apu = NULL;
	extcl_rd_apu = NULL;
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
