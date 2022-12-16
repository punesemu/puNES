/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
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

#include "wdgOpenGL.hpp"
#include "fps.h"

extern "C" void opengl_draw_scene(void);

wdgOpenGL::wdgOpenGL(QWidget *parent) : QOpenGLWidget(parent) {
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);

	setMouseTracking(true);

	gfps.count = 0;
	gfps.frequency = 60;
	gfps.timer.start();

	connect(this, SIGNAL(frameSwapped()), this, SLOT(s_fps_frame_swapped()));
}
wdgOpenGL::~wdgOpenGL() = default;

void wdgOpenGL::paintGL(void) {
	opengl_draw_scene();
}

void wdgOpenGL::show(void) {
	QOpenGLWidget::show();
	update();
}
void wdgOpenGL::hide(void) {
	QOpenGLWidget::hide();
}

unsigned int wdgOpenGL::framebuffer_id(void) {
	return (defaultFramebufferObject());
}

void wdgOpenGL::s_fps_frame_swapped(void) {
	if (++gfps.count > gfps.frequency) {
		qint64 ms = gfps.timer.elapsed();
		double sec = (double)ms / 1000.0f;

		fps.gfx = gfps.count / sec;
		gfps.count = 0;
		gfps.timer.start();
	}
}
