/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef WDGSCREEN_HPP_
#define WDGSCREEN_HPP_

#include <QtCore/QtGlobal>
#include <QtWidgets/QWidget>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QResizeEvent>
#if defined (WITH_OPENGL)
#include "wdgOpenGL.hpp"
#elif defined (WITH_D3D9)
#include "wdgD3D9.hpp"
#endif
#include "gui.h"

class wdgScreen : public QWidget {
		Q_OBJECT

	public:
#if defined (WITH_OPENGL)
		wdgOpenGL *wogl;
#elif defined (WITH_D3D9)
		wdgD3D9 *wd3d9;
#endif

	private:
		QCursor *target;

	public:
		wdgScreen(QWidget *parent);
		~wdgScreen();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);
		void resizeEvent(QResizeEvent *event);

	protected:
		QPaintEngine *paintEngine() const { return 0; }

	public:
		void cursor_init(void);
		void cursor_set(void);
		void cursor_hide(BYTE hide);

	private slots:
		void s_cursor_set(void);
		void s_cursor_hide(int hide);

	signals:
		void et_cursor_set(void);
		void et_cursor_hide(int hide);
};

#endif /* WDGSCREEN_HPP_ */
