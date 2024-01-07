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

#include <string.h>
#include "input/zapper.h"
#include "gui.h"
#include "info.h"
#include "nes.h"
#include "input/mouse.h"
#include "tas.h"
#include "palette.h"

struct _zapper {
	BYTE data;
} zapper[PORT_MAX];

void input_init_zapper(void) {
	memset(&zapper, 0x00, sizeof(zapper));
}
void input_rd_zapper(BYTE nidx, BYTE *value, BYTE nport, UNUSED(BYTE shift)) {
	int x_zapper = -1, y_zapper = -1;
	int x_rect = 0, y_rect = 0;
	int count = 0;

	zapper[nport].data &= ~0x10;

	if (gmouse.left) {
		zapper[nport].data |= 0x10;
	}

	if (tas.type) {
		x_zapper = gmouse.x;
		y_zapper = gmouse.y;
	} else {
		if (!gmouse.right) {
			input_read_mouse_coords(&x_zapper, &y_zapper);
		}
	}

	if ((x_zapper <= 0) || (x_zapper >= SCR_COLUMNS) || (y_zapper <= 0) || (y_zapper >= SCR_ROWS)) {
		zapper[nport].data |= 0x08;
		(*value) |= zapper[nport].data;
		return;
	}

	if (!nes[nidx].p.ppu.vblank && nes[nidx].p.r2001.visible &&
		(nes[nidx].p.ppu.frame_y > nes[nidx].p.ppu_sclines.vint) && (nes[nidx].p.ppu.screen_y < SCR_ROWS)) {
		for (y_rect = (y_zapper - 8); y_rect < (y_zapper + 8); y_rect++) {
			if (y_rect < 0) {
				continue;
			}
			if (y_rect <= (nes[nidx].p.ppu.screen_y - 18)) {
				continue;
			}
			if (y_rect > nes[nidx].p.ppu.screen_y) {
				break;
			}

			for (x_rect = (x_zapper - 8); x_rect < (x_zapper + 8); x_rect++) {
				if (x_rect < 0) {
					continue;
				}
				if (x_rect > 255) {
					break;
				}
				{
					int brightness = 0;
					_color_RGB color = palette_RGB.in_use[nes[nidx].p.ppu_screen.wr->line[y_rect][x_rect]];

					brightness = (int)((color.r * 0.299) + (color.g * 0.587) + (color.b * 0.114));
					if (brightness > 0x80) {
						count++;
					}
				}
			}
		}
	}

	zapper[nport].data &= ~0x08;

	if (count < 0x40) {
		zapper[nport].data |= 0x08;
	}

	(*value) |= zapper[nport].data;
}

void input_rd_zapper_vs(BYTE nidx, BYTE *value, BYTE nport, UNUSED(BYTE shift)) {
	int x_zapper = -1, y_zapper = -1;
	int x_rect = 0, y_rect = 0;
	int count = 0;
	BYTE trigger = 0, light = 1;

	if (gmouse.left) {
		trigger = 1;
	}

	if (tas.type) {
		x_zapper = gmouse.x;
		y_zapper = gmouse.y;
	} else {
		if (!gmouse.right) {
			input_read_mouse_coords(&x_zapper, &y_zapper);
		}
	}

	if ((x_zapper > 0) && (x_zapper < SCR_COLUMNS) && (y_zapper > 0) && (y_zapper < SCR_ROWS)) {
		if (!nes[nidx].p.ppu.vblank && nes[nidx].p.r2001.visible &&
			(nes[nidx].p.ppu.frame_y > nes[nidx].p.ppu_sclines.vint) && (nes[nidx].p.ppu.screen_y < SCR_ROWS)) {
			for (y_rect = (y_zapper - 8); y_rect < (y_zapper + 8); y_rect++) {
				if (y_rect < 0) {
					continue;
				}
				if (y_rect <= (nes[nidx].p.ppu.screen_y - 18)) {
					continue;
				}
				if (y_rect > nes[nidx].p.ppu.screen_y) {
					break;
				}
				for (x_rect = (x_zapper - 8); x_rect < (x_zapper + 8); x_rect++) {
					if (x_rect < 0) {
						continue;
					}
					if (x_rect > 255) {
						break;
					}
					{
						int brightness = 0;
						_color_RGB color = palette_RGB.in_use[nes[nidx].p.ppu_screen.wr->line[y_rect][x_rect]];

						brightness = (int)((color.r * 0.299) + (color.g * 0.587) + (color.b * 0.114));
						if (brightness > 0x80) {
							count++;
						}
					}
				}
			}
		}
	}

	if (count < 0x40) {
		light = 0;
	}

	// Vs. System
	// This Zapper communicates with the same protocol as the standard controller, returning an
	// 8-bit report after being strobed: 0, 0, 0, 0, 1, 0, Light sense (inverted), Trigger
	// The "light sense" status corresponds to Left and the "trigger" to Right, and Up is always
	// pressed.
	// Unlike the NES/Famicom Zapper, the Vs. Zapper's "light sense" is 1 when detecting
	// and 0 when not detecting.
	switch (nes[nidx].c.input.pindex[nport]) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 5:
		default:
			(*value) = 0;
			break;
		case 4:
			(*value) = 1;
			break;
		case 6:
			(*value) = light;
			break;
		case 7:
			(*value) = trigger;
			break;
	}

	if (!(nes[nidx].c.input.r4016 & 0x01)) {
		if (++nes[nidx].c.input.pindex[nport] >= 8) {
			nes[nidx].c.input.pindex[nport] = 0;
		}
	}
}
