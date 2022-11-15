/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#ifndef WDGTOOLBAR_HPP_
#define WDGTOOLBAR_HPP_

#include <QtWidgets/QWidget>
#include <QtWidgets/QToolBar>
#include "wdgState.hpp"
#include "wdgRewind.hpp"
#include "wdgRotateScreen.hpp"

#define SPACING 2

typedef struct _toolbar_action_widget {
	QAction *separator;
	QAction *widget;
} _toolbar_action_widget;

class wdgToolBar : public QToolBar {
	Q_OBJECT

	public:
		wdgRotateScreen *rotate;
		wdgState *state;
		wdgRewind *rewind;
		Qt::ToolBarArea area;

	private:
		_toolbar_action_widget action_rotate{};
		_toolbar_action_widget action_state{};
		_toolbar_action_widget action_rewind{};
		bool mouse_pressed;

	public:
		explicit wdgToolBar(QWidget *parent = nullptr);
		~wdgToolBar() override;

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;

	public:
		void update_toolbar(void) const;
		void rotate_setVisible(bool visible) const;
		void state_setVisible(bool visible) const;
		void rewind_setVisible(bool visible) const;

	private slots:
		void s_toplevel_changed(bool toplevel);
};

#endif /* WDGTOOLBAR_HPP_ */
