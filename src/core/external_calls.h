/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef EXTERNAL_CALLS_H_
#define EXTERNAL_CALLS_H_

#include <stdio.h>
#include "common.h"

/* mappers */
#define EXTCL_AFTER_MAPPER_INIT(n) extcl_after_mapper_init = extcl_after_mapper_init_##n
#define EXTCL_CPU_WR_MEM(n) extcl_cpu_wr_mem = extcl_cpu_wr_mem_##n
#define EXTCL_CPU_RD_MEM(n) extcl_cpu_rd_mem = extcl_cpu_rd_mem_##n
#define EXTCL_SAVE_MAPPER(n) extcl_save_mapper = extcl_save_mapper_##n

/* CPU */
#define EXTCL_CPU_EVERY_CYCLE(n) extcl_cpu_every_cycle = extcl_cpu_every_cycle_##n
#define EXTCL_CPU_WR_R4016(n) extcl_cpu_wr_r4016 = extcl_cpu_wr_r4016_##n

/* PPU */
#define EXTCL_PPU_000_TO_34X(n) extcl_ppu_000_to_34x = extcl_ppu_000_to_34x_##n
#define EXTCL_PPU_000_TO_255(n) extcl_ppu_000_to_255 = extcl_ppu_000_to_255_##n
#define EXTCL_PPU_256_TO_319(n) extcl_ppu_256_to_319 = extcl_ppu_256_to_319_##n
#define EXTCL_PPU_320_TO_34X(n) extcl_ppu_320_to_34x = extcl_ppu_320_to_34x_##n
#define EXTCL_PPU_UPDATE_SCREEN_Y(n) extcl_ppu_update_screen_y = extcl_ppu_update_screen_y_##n
#define EXTCL_UPDATE_R2006(n) extcl_update_r2006 = extcl_update_r2006_##n
#define EXTCL_AFTER_RD_CHR(n) extcl_after_rd_chr = extcl_after_rd_chr_##n
#define EXTCL_RD_PPU(n) extcl_rd_ppu = extcl_rd_ppu_##n
#define EXTCL_RD_NMT(n) extcl_rd_nmt = extcl_rd_nmt_##n
#define EXTCL_RD_CHR(n) extcl_rd_chr = extcl_rd_chr_##n
#define EXTCL_WR_NMT(n) extcl_wr_nmt = extcl_wr_nmt_##n
#define EXTCL_WR_CHR(n) extcl_wr_chr = extcl_wr_chr_##n

/* APU */
#define EXTCL_LENGTH_CLOCK(n) extcl_length_clock = extcl_length_clock_##n
#define EXTCL_ENVELOPE_CLOCK(n) extcl_envelope_clock = extcl_envelope_clock_##n
#define EXTCL_APU_TICK(n) extcl_apu_tick = extcl_apu_tick_##n

/* irqA12 */
#define EXTCL_IRQ_A12_CLOCK(n) extcl_irq_A12_clock = extcl_irq_A12_clock_##n

/* battery */
#define EXTCL_BATTERY_IO(n) extcl_battery_io = extcl_battery_io_##n

/* snd */
#define EXTCL_SND_START(n) extcl_snd_start = extcl_snd_start_##n

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC void extcl_init(void);

/* mappers */
/* viene chiamata dopo il map_init(), map_prg_ram_init() e map_chr_ram_init() */
EXTERNC void (*extcl_after_mapper_init)(void);
/* */
EXTERNC void (*extcl_cpu_wr_mem)(WORD address, BYTE value);
EXTERNC BYTE (*extcl_cpu_rd_mem)(WORD address, BYTE openbus, BYTE before);
EXTERNC BYTE (*extcl_save_mapper)(BYTE mode, BYTE slot, FILE *fp);

/* CPU */
EXTERNC void (*extcl_cpu_every_cycle)(void);
/* viene chiamata ogni volta si scrive qualcosa nel registro $4016 */
EXTERNC void (*extcl_cpu_wr_r4016)(BYTE value);

/* PPU */
/* viene chiamata sempre, ad ogni ciclo della PPU */
EXTERNC void (*extcl_ppu_000_to_34x)(void);
/*
 * viene chiamata se (!r2002.vblank && (ppu.screen_y < SCR_LINES))
 * quindi per essere sicuri di essere durante il rendering della PPU
 * nella funzione devo controllare anche se r2001.visible non e' a zero.
 */
EXTERNC void (*extcl_ppu_000_to_255)(void);
/*
 * vengono chiamate solo se la PPU e' in fase di rendering
 * (!r2002.vblank && r2001.visible && (ppu.screen_y < SCR_LINES))
 */
EXTERNC void (*extcl_ppu_256_to_319)(void);
EXTERNC void (*extcl_ppu_320_to_34x)(void);
/* viene chiamata ogni volta viene modificato ppu.screen_y */
EXTERNC void (*extcl_ppu_update_screen_y)(void);
/* viene chiamata dopo ogni cambiamento del $2006 in cpu_inline.h */
EXTERNC void (*extcl_update_r2006)(WORD new_r2006, WORD old_r2006);
/* vengono chiamate in ppu_inline.h */
EXTERNC void (*extcl_rd_ppu)(WORD address);
EXTERNC BYTE (*extcl_rd_nmt)(WORD address);
EXTERNC BYTE (*extcl_rd_chr)(WORD address);
/* viene chiamata dopo il FETCHB e dopo il fetch dello sprite */
EXTERNC void (*extcl_after_rd_chr)(WORD address);
/* viene chiamato quando si tenta di scrivere nella Nametable Ram */
EXTERNC void (*extcl_wr_nmt)(WORD address, BYTE value);
/* viene chiamato quando si tenta di scrivere nella CHR Ram */
EXTERNC void (*extcl_wr_chr)(WORD address, BYTE value);

/* APU */
EXTERNC void (*extcl_length_clock)(void);
EXTERNC void (*extcl_envelope_clock)(void);
EXTERNC void (*extcl_apu_tick)(void);

/* irqA12 */
EXTERNC void (*extcl_irq_A12_clock)(void);

/* battery */
EXTERNC void (*extcl_battery_io)(BYTE mode, FILE *fp);

/* snd */
EXTERNC void (*extcl_snd_start)(WORD samplarate);

#undef EXTERNC

#endif /* EXTERNAL_CALLS_H_ */
