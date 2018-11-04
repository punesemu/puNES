/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#include <string.h>
#include "fps.h"
#include "clock.h"
#include "conf.h"

void fps_init(void) {
	if (machine.type == NTSC) {
		machine.fps = 60;
	} else {
		machine.fps = 50;
	}

	memset(&fps, 0x00, sizeof(fps));

	if (fps.fast_forward == FALSE) {
		fps_machine_ms(1.0f)
	}

	fps_normalize();
}
void fps_fast_forward(void) {
	fps.fast_forward = TRUE;
	fps.frame.estimated_ms = (int) (1000.0f / (machine.fps * cfg->ff_velocity));
}
void fps_normalize(void) {
	fps.frame.estimated_ms = machine.ms_frame;
	fps.fast_forward = FALSE;
}
