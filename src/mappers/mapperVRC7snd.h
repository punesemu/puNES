/*
 * mapperVRC7snd.h
 *
 *  Created on: 26/gen/2012
 *      Author: fhorse
 */

#ifndef MAPPERVRC7SND_H_
#define MAPPERVRC7SND_H_

#include "common.h"

void opll_reset(uint32_t clk, uint32_t rate);
void opll_write_reg(uint32_t reg, uint8_t value);
BYTE opll_save(BYTE mode, BYTE slot, FILE *fp);
SWORD opll_calc(void);

#endif /* MAPPERVRC7SND_H_ */
