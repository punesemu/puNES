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

#include "wdgOpenGL.hpp"
#include "gfx.h"

extern "C" void opengl_draw_scene(void);

wdgOpenGL::wdgOpenGL(QWidget *parent, int vsync) : QOpenGLWidget(parent) {
	QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();

	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_OpaquePaintEvent);

	fmt.setRenderableType(QSurfaceFormat::OpenGL);
	fmt.setProfile(QSurfaceFormat::CompatibilityProfile);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	fmt.setRedBufferSize(8);
	fmt.setGreenBufferSize(8);
	fmt.setBlueBufferSize(8);
	fmt.setAlphaBufferSize(8);
	fmt.setSwapInterval(vsync);
	setFormat(fmt);
}
wdgOpenGL::~wdgOpenGL() {}

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
