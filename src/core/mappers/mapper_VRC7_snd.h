/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#ifndef MAPPER_VRC7_SND_H_
#define MAPPER_VRC7_SND_H_

#include <stdio.h>
#include "common.h"

void opll_reset(uint32_t clk, uint32_t rate);
void opll_write_reg(uint32_t reg, uint8_t value);
BYTE opll_save(BYTE mode, BYTE slot, FILE *fp);
SWORD opll_calc(void);

#endif /* MAPPER_VRC7_SND_H_ */
