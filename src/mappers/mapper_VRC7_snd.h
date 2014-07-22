/*
 * mapper_VRC7_snd.h
 *
 *  Created on: 26/gen/2012
 *      Author: fhorse
 */

#ifndef MAPPER_VRC7_SND_H_
#define MAPPER_VRC7_SND_H_

#include <stdio.h>
#include "common.h"

void opll_reset(uint32_t clk, uint32_t rate);
void opll_write_reg(uint32_t reg, uint8_t value);
BYTE opll_save(BYTE mode, BYTE slot, FILE *fp);
SWORD opll_calc(void);

#endif /* MAPPER_VRC7_SND_H_ */
