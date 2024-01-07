/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

#ifndef NES_H_
#define NES_H_

#include <stdio.h>
#include "common.h"
#include "cpu.h"
#include "ppu.h"
#include "memmap.h"
#include "irqA12.h"
#include "irql2f.h"
#include "input.h"

typedef struct _nes {
	_cpu_data c;
	_ppu_data p;
	_memmap_data m;
	_irqA12 irqA12;
	_irql2f irql2f;
} _nes;

extern _nes nes[NES_CHIPS_MAX];
extern _ppu_alignment ppu_alignment;
extern _prgrom prgrom;
extern _chrrom chrrom;
extern _wram wram;
extern _miscrom miscrom;

#endif /* NES_H_ */
