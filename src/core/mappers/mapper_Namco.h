/*
 * mapper_Namco.h
 *
 *  Created on: 13/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_NAMCO_H_
#define MAPPER_NAMCO_H_

#include "common.h"

enum {
	N163,
	N3416,
	N3425,
	N3433,
	N3446,
	N3453,
	NAMCO_HARD_WIRED_V,
	NAMCO_HARD_WIRED_H,
	MINDSEEKER
};

typedef struct {
	BYTE enabled;
	BYTE active;
	WORD address;
	DBWORD freq;
	DBWORD cycles_reload;
	DBWORD cycles;
	BYTE length;
	BYTE step;
	WORD volume;
	SWORD output;
} _n163_snd_ch;
struct _n163 {
	uint32_t nmt_bank[4][2];
	BYTE irq_delay;
	DBWORD irq_count;
	BYTE snd_ram[0x80];
	BYTE snd_adr;
	BYTE snd_auto_inc;
	BYTE snd_ch_start;
	BYTE snd_wave[0x100];
	_n163_snd_ch ch[8];
} n163;
struct _n3425 {
	BYTE bank_to_update;
} n3425;
struct _n3446 {
	BYTE bank_to_update;
	BYTE prg_rom_mode;
} n3446;

void map_init_Namco(BYTE model);

void extcl_cpu_wr_mem_Namco_163(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Namco_163(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Namco_163(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Namco_163(void);
void extcl_apu_tick_Namco_163(void);

void extcl_cpu_wr_mem_Namco_3425(WORD address, BYTE value);
BYTE extcl_save_mapper_Namco_3425(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Namco_3446(WORD address, BYTE value);
BYTE extcl_save_mapper_Namco_3446(BYTE mode, BYTE slot, FILE *fp);

#endif /* MAPPER_NAMCO_H_ */
