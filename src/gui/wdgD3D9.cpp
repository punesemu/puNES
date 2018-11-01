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

#include "wdgD3D9.hpp"
#include "gfx.h"
#include "fps.h"

extern "C" void d3d9_draw_scene(void);

wdgD3D9::wdgD3D9(QWidget *parent) : QWidget(parent) {
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);

	gfps.count = 0;
	gfps.frequency = 60;
	gfps.timer.start();
}
wdgD3D9::~wdgD3D9() {}

void wdgD3D9::paintEvent(QPaintEvent *event) {
	d3d9_draw_scene();

	if (++gfps.count > gfps.frequency) {
		qint64 ms = gfps.timer.elapsed();
		double sec = ms / 1000.0f;

		fps.gfx = gfps.count / sec;
		gfps.count = 0;
		gfps.timer.start();
	}
}
