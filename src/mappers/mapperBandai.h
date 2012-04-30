/*
 * mapperBandai.h
 *
 *  Created on: 22/set/2011
 *      Author: fhorse
 */

#ifndef MAPPERBANDAI_H_
#define MAPPERBANDAI_H_

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
	BYTE chrRomBank;
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

void mapInit_Bandai(BYTE model);

void extclCpuWrMem_Bandai_161x02x74(WORD address, BYTE value);
BYTE extclSaveMapper_Bandai_161x02x74(BYTE mode, BYTE slot, FILE *fp);
void extcl2006Update_Bandai_161x02x74(WORD r2006Old);
BYTE extclRdNmt_Bandai_161x02x74(WORD address);

void extclCpuWrMem_Bandai_FCGX(WORD address, BYTE value);
BYTE extclCpuRdMem_Bandai_FCGX(WORD address, BYTE openbus, BYTE before);
BYTE extclSaveMapper_Bandai_FCGX(BYTE mode, BYTE slot, FILE *fp);
void extclBatteryIO_Bandai_FCGX(BYTE mode, FILE *fp);
void extclCPUEveryCycle_Bandai_FCGX(void);

#endif /* MAPPERBANDAI_H_ */
