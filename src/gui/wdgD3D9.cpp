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

#include <math.h>
#include "wdgD3D9.hpp"
#include "fps.h"
#include "nes.h"
#include "emu.h"
#include "video/gfx_thread.h"

extern "C" void d3d9_draw_scene(void);

wdgD3D9::wdgD3D9(QWidget *parent) : QWidget(parent) {
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);

	setMouseTracking(true);

	gfps.count = 0;
	gfps.frequency = 60;
	gfps.timer.start();
}
wdgD3D9::~wdgD3D9() = default;

QPaintEngine *wdgD3D9::paintEngine() const {
	return (nullptr);
}
void wdgD3D9::paintEvent(UNUSED(QPaintEvent *event)) {
	d3d9_draw_scene();

	if (++gfps.count > gfps.frequency) {
		qint64 ms = gfps.timer.elapsed();
		BYTE nidx = emu_active_nidx();
		double sec = (double)ms / 1000.0;

		gfx_ppu_thread_lock();
		fps.emu = lround(nes[nidx].p.fps / sec);
		nes[nidx].p.fps = 0;
		gfx_ppu_thread_unlock();
		fps.gfx = gfps.count / sec;
		gfps.count = 0;
		gfps.timer.start();
	}
}
