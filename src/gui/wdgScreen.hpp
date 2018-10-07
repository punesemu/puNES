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

#ifndef WDGSCREEN_HPP_
#define WDGSCREEN_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QWidget>
#else
#include <QtWidgets/QWidget>
#endif
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include "gui.h"

class wdgScreen : public QWidget {
		Q_OBJECT

	private:
#if defined (__WIN32__)
#if defined (WITH_OPENGL)
		struct _data {
			LONG_PTR WINAPI (*qt)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			LONG_PTR WINAPI (*sdl)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
			LONG_PTR WINAPI (*tmp)(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		} data;
#endif
		QCursor *target;
#endif

	public:
		wdgScreen(QWidget *parent);
		~wdgScreen();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
		void dragEnterEvent(QDragEnterEvent *event);
		void dropEvent(QDropEvent *event);

	public:
#if defined (__WIN32__)
#if defined (WITH_OPENGL)
		void controlEventFilter(void);
#endif
		void cursor_init(void);
		void cursor_set(void);
		void cursor_hide(BYTE hide);
#endif
};

#endif /* WDGSCREEN_HPP_ */
