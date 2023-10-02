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

#ifndef EXTERNAL_CALLS_H_
#define EXTERNAL_CALLS_H_

#include <stdio.h>
#include "common.h"

// mappers
#define EXTCL_AFTER_MAPPER_INIT(n) extcl_after_mapper_init = extcl_after_mapper_init_##n
#define EXTCL_MAPPER_QUIT(n) extcl_mapper_quit = extcl_mapper_quit_##n
#define EXTCL_CPU_WR_MEM(n) extcl_cpu_wr_mem = extcl_cpu_wr_mem_##n
#define EXTCL_CPU_RD_MEM(n) extcl_cpu_rd_mem = extcl_cpu_rd_mem_##n
#define EXTCL_CPU_RD_RAM(n) extcl_cpu_rd_ram = extcl_cpu_rd_ram_##n
#define EXTCL_SAVE_MAPPER(n) extcl_save_mapper = extcl_save_mapper_##n
// CPU
#define EXTCL_CPU_INIT_PC(n) extcl_cpu_init_pc = extcl_cpu_init_pc_##n
#define EXTCL_CPU_EVERY_CYCLE(n) extcl_cpu_every_cycle = extcl_cpu_every_cycle_##n
#define EXTCL_CPU_WR_R4016(n) extcl_cpu_wr_r4016 = extcl_cpu_wr_r4016_##n
// PPU
#define EXTCL_PPU_000_TO_34X(n) extcl_ppu_000_to_34x = extcl_ppu_000_to_34x_##n
#define EXTCL_PPU_000_TO_255(n) extcl_ppu_000_to_255 = extcl_ppu_000_to_255_##n
#define EXTCL_PPU_256_TO_319(n) extcl_ppu_256_to_319 = extcl_ppu_256_to_319_##n
#define EXTCL_PPU_320_TO_34X(n) extcl_ppu_320_to_34x = extcl_ppu_320_to_34x_##n
#define EXTCL_UPDATE_R2006(n) extcl_update_r2006 = extcl_update_r2006_##n
#define EXTCL_RD_R2007(n) extcl_rd_r2007 = extcl_rd_r2007_##n
#define EXTCL_AFTER_RD_CHR(n) extcl_after_rd_chr = extcl_after_rd_chr_##n
#define EXTCL_WR_PPU_REG(n) extcl_wr_ppu_reg = extcl_wr_ppu_reg_##n
#define EXTCL_RD_PPU_MEM(n) extcl_rd_ppu_mem = extcl_rd_ppu_mem_##n
#define EXTCL_RD_NMT(n) extcl_rd_nmt = extcl_rd_nmt_##n
#define EXTCL_RD_CHR(n) extcl_rd_chr = extcl_rd_chr_##n
#define EXTCL_WR_NMT(n) extcl_wr_nmt = extcl_wr_nmt_##n
#define EXTCL_WR_CHR(n) extcl_wr_chr = extcl_wr_chr_##n
// APU
#define EXTCL_WR_APU(n) extcl_wr_apu = extcl_wr_apu_##n
#define EXTCL_RD_APU(n) extcl_rd_apu = extcl_rd_apu_##n
#define EXTCL_LENGTH_CLOCK(n) extcl_length_clock = extcl_length_clock_##n
#define EXTCL_ENVELOPE_CLOCK(n) extcl_envelope_clock = extcl_envelope_clock_##n
#define EXTCL_APU_TICK(n) extcl_apu_tick = extcl_apu_tick_##n
// irqA12
#define EXTCL_IRQ_A12_CLOCK(n) extcl_irq_A12_clock = extcl_irq_A12_clock_##n
// battery
#define EXTCL_BATTERY_IO(n) extcl_battery_io = extcl_battery_io_##n
// audio
#define EXTCL_AUDIO_SAMPLES_MOD(n) extcl_audio_samples_mod = extcl_audio_samples_mod_##n

// viene chiamata dopo il map_init()
extern void (*extcl_after_mapper_init)(void);
// viene chiamata dal mapper_quit()
extern void (*extcl_mapper_quit)(void);
extern void (*extcl_cpu_wr_mem)(BYTE nidx, WORD address, BYTE value);
extern BYTE (*extcl_cpu_rd_mem)(BYTE nidx, WORD address, BYTE openbus);
extern BYTE (*extcl_cpu_rd_ram)(BYTE nidx, WORD address, BYTE openbus);
extern BYTE (*extcl_save_mapper)(BYTE mode, BYTE slot, FILE *fp);
// CPU
// viene chimata subito dopo il cpu_init_pc
extern void (*extcl_cpu_init_pc)(BYTE nidx);
// viene chiamata ad ogni ciclo di cpu
extern void (*extcl_cpu_every_cycle)(BYTE nidx);
// viene chiamata ogni volta si scrive qualcosa nel registro $4016
extern void (*extcl_cpu_wr_r4016)(BYTE nidx, BYTE value);
// PPU
// viene chiamata sempre, ad ogni ciclo della PPU
extern void (*extcl_ppu_000_to_34x)(BYTE nidx);
// viene chiamata se (!ppu.vblank && (ppu.screen_y < SCR_ROWS))
// quindi per essere sicuri di essere durante il rendering della PPU
// nella funzione devo controllare anche se nes[nidx].p.r2001.visible non e' a zero.
extern void (*extcl_ppu_000_to_255)(BYTE nidx);
// vengono chiamate solo se la PPU e' in fase di rendering
// (!ppu.vblank && nes[nidx].p.r2001.visible && (ppu.screen_y < SCR_ROWS))
extern void (*extcl_ppu_256_to_319)(BYTE nidx);
extern void (*extcl_ppu_320_to_34x)(BYTE nidx);
// viene chiamata dopo ogni cambiamento del $2006 in cpu_inline.h
extern void (*extcl_update_r2006)(BYTE nidx, WORD new_r2006, WORD old_r2006);
// viene chiamata alla lettura del $2007 in cpu_inline.h
extern void (*extcl_rd_r2007)(BYTE nidx);
// vine chiamata in cpu_inline.h alla scrittura nei rigistri della ppu
extern BYTE (*extcl_wr_ppu_reg)(BYTE nidx, WORD address, BYTE *value);
// vengono chiamate in ppu_inline.h
extern void (*extcl_rd_ppu_mem)(BYTE nidx, WORD address);
extern BYTE (*extcl_rd_nmt)(BYTE nidx, WORD address);
extern BYTE (*extcl_rd_chr)(BYTE nidx, WORD address);
// viene chiamata dopo il FETCHB e dopo il fetch dello sprite
extern void (*extcl_after_rd_chr)(BYTE nidx, WORD address);
// viene chiamato quando si tenta di scrivere nella Nametable Ram
extern void (*extcl_wr_nmt)(BYTE nidx, WORD address, BYTE value);
// viene chiamato quando si tenta di scrivere nella CHR Ram
extern void (*extcl_wr_chr)(BYTE nidx, WORD address, BYTE value);
// APU
// vine chiamata in cpu_inline.h alla scrittura nei rigistri della apu
extern BYTE (*extcl_wr_apu)(BYTE nidx, WORD address, BYTE *value);
// vine chiamata in cpu_inline.h alla lettura dei rigistri della apu
extern BYTE (*extcl_rd_apu)(BYTE nidx, WORD address, BYTE openbus);
extern void (*extcl_length_clock)(void);
extern void (*extcl_envelope_clock)(void);
extern void (*extcl_apu_tick)(void);
// irqA12
extern void (*extcl_irq_A12_clock)(BYTE nidx);
// battery
extern void (*extcl_battery_io)(BYTE mode, FILE *fp);
// audio
extern void (*extcl_audio_samples_mod)(SWORD *samples, int count);

void extcl_init(void);

#endif /* EXTERNAL_CALLS_H_ */
