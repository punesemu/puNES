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

#ifndef WDGOPENGL_HPP_
#define WDGOPENGL_HPP_

// gui.h e' importante che stia in mezzo per non
// avere problemi di compilazione legati ai vari
// nested #include nella versione Windows
#include <QtWidgets/QWidget>
#include "gui.h"
#include <QtWidgets/QOpenGLWidget>
#include <QtCore/QElapsedTimer>

class wdgOpenGL : public QOpenGLWidget {
	Q_OBJECT

	private:
		struct _gui_fps {
			double count;
			double frequency;
			QElapsedTimer timer;
		} gfps;

	public:
		wdgOpenGL(QWidget *parent, int vsync);
		~wdgOpenGL();

	protected:
		void paintGL(void);

	public:
		void show(void);
		void hide(void);

	public:
		unsigned int framebuffer_id(void);

	private slots:
		void s_fps_frame_swapped(void);
};

#endif /* WDGOPENGL_HPP_ */
