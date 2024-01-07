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

#ifndef NTSC_NESRGB_LMP88959_H_
#define NTSC_NESRGB_LMP88959_H_

#include "common.h"
#include "ntsc_lmp88959.h"

extern _ntsc_lmp88959_setup_t ntsc_nesrgb_lmp88959;

#if defined (__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

EXTERNC BYTE ntsc_nesrgb_lmp88959_init(void);
EXTERNC void ntsc_nesrgb_lmp88959_surface(BYTE nidx);

EXTERNC void ntsc_nesrgb_lmp88959_filter_parameters_changed(void);
EXTERNC void ntsc_nesrgb_lmp88959_filter_parameters_default(void);

EXTERNC void ntsc_nesrgb_lmp88959_filter_parameter_default(int index);
EXTERNC void ntsc_nesrgb_lmp88959_filter_parameter_sv_default(void);

#undef EXTERNC

#endif /* NTSC_NESRGB_LMP88959_H_ */
