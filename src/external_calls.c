/*
 * external_calls.c
 *
 *  Created on: 06/set/2011
 *      Author: fhorse
 */

#include "external_calls.h"

void extcl_init(void) {
	/* Mappers */
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
	/* snd */
	extcl_snd_start = NULL;
}
