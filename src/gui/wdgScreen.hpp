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

#ifndef WDGSCREEN_HPP_
#define WDGSCREEN_HPP_

#include <QtCore/QtGlobal>
#include <QtCore/QMutex>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMenu>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QResizeEvent>
#if defined (WITH_OPENGL)
#include "wdgOpenGL.hpp"
#elif defined (WITH_D3D9)
#include "wdgD3D9.hpp"
#endif
#include "gui.h"

typedef struct _wdgScreen_keyboard_event {
	_wdgScreen_keyboard_event(BYTE md, BYTE ar, DBWORD ev, BYTE ty)
	: mode(md), autorepeat(ar), event(ev), type(ty) {}
	BYTE mode;
	BYTE autorepeat;
	DBWORD event;
	BYTE type;
} _wdgScreen_keyboard_event;
typedef struct _wdgScreen_mouse_event {
	_wdgScreen_mouse_event(QEvent::Type ty, Qt::MouseButton bt, int xmov, int ymov)
	: type(ty), button(bt), x(xmov), y(ymov) {}
	QEvent::Type type;
	Qt::MouseButton button;
	int x, y;
} _wdgScreen_mouse_event;

class wdgScreen : public QWidget {
	Q_OBJECT

	public:
#if defined (WITH_OPENGL)
		wdgOpenGL *wogl;
#elif defined (WITH_D3D9)
		wdgD3D9 *wd3d9;
#endif
		struct _events {
			// mutex per la gestione degli eventi
			QMutex mutex;
			// lista degli eventi di input da processare
			QList<_wdgScreen_keyboard_event> keyb;
			QList<_wdgScreen_mouse_event> mouse;
		} events;

	private:
		QCursor *target;

	public:
		explicit wdgScreen(QWidget *parent = nullptr);
		~wdgScreen() override;

	signals:
		void et_cursor_set(void);
		void et_cursor_hide(int hide);

	protected:
		QPaintEngine *paintEngine() const override;
		bool eventFilter(QObject *obj, QEvent *event) override;
		void dragEnterEvent(QDragEnterEvent *event) override;
		void dropEvent(QDropEvent *event) override;
		void resizeEvent(QResizeEvent *event) override;

	public:
		void cursor_init(void);
		void cursor_set(void);
		void cursor_hide(BYTE hide);

	private:
		void menu_copy(QMenu *src, QMenu *dst, bool src_as_root);

	private slots:
		void s_cursor_set(void);
		void s_cursor_hide(int hide);
		void s_paste_event(void);
		void s_capture_input_event(void);
		void s_context_menu(const QPoint &pos);
};

#endif /* WDGSCREEN_HPP_ */
