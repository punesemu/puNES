/*
 * mapper_Bandai.h
 *
 *  Created on: 22/set/2011
 *      Author: fhorse
 */

#ifndef MAPPER_BANDAI_H_
#define MAPPER_BANDAI_H_

#include "common.h"

enum {
	B161X02X74 = 2,
	FCGx = 3,
	E24C01 = 4,
	E24C02 = 5,
	DATACH = 6,
	FAMICONJUMPII = 100
};

typedef struct {
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

void map_init_Bandai(BYTE model);

void extcl_cpu_wr_mem_Bandai_161x02x74(WORD address, BYTE value);
BYTE extcl_save_mapper_Bandai_161x02x74(BYTE mode, BYTE slot, FILE *fp);
void extcl_update_r2006_Bandai_161x02x74(WORD old_r2006);
BYTE extcl_rd_nmt_Bandai_161x02x74(WORD address);

void extcl_cpu_wr_mem_Bandai_FCGX(WORD address, BYTE value);
BYTE extcl_cpu_rd_mem_Bandai_FCGX(WORD address, BYTE openbus, BYTE before);
BYTE extcl_save_mapper_Bandai_FCGX(BYTE mode, BYTE slot, FILE *fp);
void extcl_battery_io_Bandai_FCGX(BYTE mode, FILE *fp);
void extcl_cpu_every_cycle_Bandai_FCGX(void);

#endif /* MAPPER_BANDAI_H_ */
