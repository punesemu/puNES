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

#include <QtCore/QEvent>
#include "wdgToolBar.hpp"
#include "mainWindow.hpp"
#include "save_slot.h"
#include "conf.h"
#include "emu_thread.h"

wdgToolBar::wdgToolBar(QWidget *parent) : QToolBar(parent) {
	QWidget *w;

	mouse_pressed = false;

	setFloatable(false);

	rotate = new wdgRotateScreen(this);
	action_rotate.widget = addWidget(rotate);
	action_rotate.separator = addSeparator();

	w = new QWidget();
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	addWidget(w);

	state = new wdgState(this);
	action_state.separator = nullptr;
	action_state.widget = addWidget(state);

	w = new QWidget();
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	addWidget(w);

	rewind = new wdgRewind(this);
	action_rewind.separator = addSeparator();
	action_rewind.widget = addWidget(rewind);

	switch (cfg->toolbar.area) {
		default:
		case TLB_TOP:
			area = Qt::TopToolBarArea;
			break;
		case TLB_LEFT:
			area = Qt::LeftToolBarArea;
			break;
		case TLB_BOTTOM:
			area = Qt::BottomToolBarArea;
			break;
		case TLB_RIGHT:
			area = Qt::RightToolBarArea;
			break;
	}

	connect(this, SIGNAL(topLevelChanged(bool)), this, SLOT(s_toplevel_changed(bool)));

	setStyleSheet(QString(toolbar_button_stylesheet()) + QString(toolbar_toolbutton_stylesheet()));

	installEventFilter(this);
}
wdgToolBar::~wdgToolBar() = default;

bool wdgToolBar::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		if (((QMouseEvent *)event)->button() == Qt::LeftButton) {
			mouse_pressed = true;
		}
	} else if (event->type() == QEvent::MouseButtonRelease) {
		if (((QMouseEvent *)event)->button() == Qt::LeftButton) {
			mouse_pressed = false;
		}
	}

	return (QObject::eventFilter(obj, event));
}

void wdgToolBar::update_toolbar(void) const {
	bool rw = true;

	if (info.no_rom | info.turn_off | nsf.state) {
		state->setEnabled(false);

		rw = false;
	} else {
		state->setEnabled(true);
		state->update_widget();

		if (cfg->rewind_minutes == RWND_0_MINUTES) {
			rw = false;
		}
	}
	rewind->setEnabled(rw);
	rotate->update_widget();
}

void wdgToolBar::rotate_setVisible(bool visible) const {
	if (action_rotate.separator) {
		action_rotate.separator->setVisible(visible);
	}
	action_rotate.widget->setVisible(visible);
}
void wdgToolBar::state_setVisible(bool visible) const {
	if (action_state.separator) {
		action_state.separator->setVisible(visible);
	}
	action_state.widget->setVisible(visible);
}
void wdgToolBar::rewind_setVisible(bool visible) const {
	if (action_rewind.separator) {
		action_rewind.separator->setVisible(visible);
	}
	action_rewind.widget->setVisible(visible);
}

void wdgToolBar::s_toplevel_changed(UNUSED(bool toplevel)) {
	if (toplevel) {
		this->setOrientation(Qt::Vertical);
	}

	area = mainwin->toolBarArea(this);

	switch (area) {
		default:
		case Qt::TopToolBarArea:
			cfg->toolbar.area = TLB_TOP;
			break;
		case Qt::LeftToolBarArea:
			cfg->toolbar.area = TLB_LEFT;
			break;
		case Qt::BottomToolBarArea:
			cfg->toolbar.area = TLB_BOTTOM;
			break;
		case Qt::RightToolBarArea:
			cfg->toolbar.area = TLB_RIGHT;
			break;
	}

	if (!mouse_pressed) {
		gui_set_window_size();
	}
}
