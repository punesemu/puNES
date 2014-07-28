/*
 * mapper_Sunsoft.h
 *
 *  Created on: 18/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_SUNSOFT_H_
#define MAPPER_SUNSOFT_H_

#include "common.h"

enum {
	SUN1,
	SUN2A,
	SUN2B,
	SUN3,
	SUN4,
	FM7,
	MAHARAJA,
	BARCODEWORLD,
	DODGEDANPEI2
};

typedef struct {
	BYTE disable;
	BYTE step;
	WORD frequency;
	WORD timer;
	WORD volume;
	SWORD output;

/* ------------------------------------------------------- */
/* questi valori non e' necessario salvarli nei savestates */
/* ------------------------------------------------------- */
/* */ BYTE clocked;                                     /* */
/* ------------------------------------------------------- */
} _square_fm7;

struct _sunsoft3 {
	BYTE enable;
	BYTE toggle;
	WORD count;
	BYTE delay;
} s3;
struct _sunsoft4 {
	uint32_t chr_nmt[2];
	BYTE mode;
	BYTE mirroring;
} s4;
struct sunsoft_fm7 {
	BYTE address;
	BYTE prg_ram_enable;
	BYTE prg_ram_mode;
	uint32_t prg_ram_address;
	BYTE irq_enable_trig;
	BYTE irq_enable_count;
	WORD irq_count;
	BYTE irq_delay;
	BYTE snd_reg;
	_square_fm7 square[3];
} fm7;

void map_init_Sunsoft(BYTE model);

void extcl_cpu_wr_mem_Sunsoft_S1(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sunsoft_S2(WORD address, BYTE value);

void extcl_cpu_wr_mem_Sunsoft_S3(WORD address, BYTE value);
BYTE extcl_save_mapper_Sunsoft_S3(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Sunsoft_S3(void);

void extcl_cpu_wr_mem_Sunsoft_S4(WORD address, BYTE value);
BYTE extcl_save_mapper_Sunsoft_S4(BYTE mode, BYTE slot, FILE *fp);

void extcl_cpu_wr_mem_Sunsoft_FM7(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Sunsoft_FM7(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Sunsoft_FM7(BYTE mode, BYTE slot, FILE *fp);
void extcl_cpu_every_cycle_Sunsoft_FM7(void);
void extcl_apu_tick_Sunsoft_FM7(void);

#endif /* MAPPER_SUNSOFT_H_ */
