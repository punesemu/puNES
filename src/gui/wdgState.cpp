/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <QtWidgets/QToolTip>
#include "wdgState.moc"
#include "wdgToolBar.hpp"
#include "mainWindow.hpp"
#include "save_slot.h"
#include "tas.h"

wdgState::wdgState(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	gridLayout->setHorizontalSpacing(SPACING);

	pushButton_save->installEventFilter(this);
	pushButton_load->installEventFilter(this);

	connect(pushButton_save, SIGNAL(clicked(bool)), this, SLOT(s_save_clicked(bool)));
	connect(wdgStateBar, SIGNAL(et_slot_actived(void)), this, SLOT(s_slot_actived(void)));
	connect(pushButton_load, SIGNAL(clicked(bool)), this, SLOT(s_load_clicked(bool)));
}
wdgState::~wdgState() {}

void wdgState::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgState::retranslateUi(wdgState *wdgState) {
	Ui::wdgState::retranslateUi(wdgState);
}

void wdgState::update_widget(void) {
	pushButton_load->setEnabled((tas.type == NOTAS) & save_slot.state[save_slot.slot]);
	update();
}

void wdgState::s_slot_actived(void) {
	update_widget();
}
void wdgState::s_save_clicked(UNUSED(bool checked)) {
	mainwin->action_Save_state->trigger();
	update_widget();
	gui_set_focus();
}
void wdgState::s_load_clicked(UNUSED(bool checked)) {
	mainwin->action_Load_state->trigger();
	gui_set_focus();
}

// ---------------------------------- stateBar --------------------------------------------

stateBar::stateBar(QWidget *parent) : QWidget(parent) {
	setMouseTracking(true);
	installEventFilter(this);
}
stateBar::~stateBar() {}

QSize stateBar::sizeHint() const {
	return (QSize(SAVE_SLOTS * 13, fontMetrics().boundingRect("Qqy").height()));
}
bool stateBar::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::ToolTip) {
		QHelpEvent *helpEvent = ((QHelpEvent *)event);
		int slot = slot_at(helpEvent->pos());

		if (slot != -1) {
			QToolTip::showText(helpEvent->globalPos(), mainwin->state_save_slot_action(slot)->toolTip());
		} else {
			QToolTip::hideText();
			event->ignore();
		}
		return (true);
	} else if (event->type() == QEvent::MouseButtonPress) {
		DBWORD slot = slot_at(((QMouseEvent *)event)->pos());

		if (slot != save_slot.slot) {
			gui_state_save_slot_set(slot, TRUE);
		}
	}
	return (QWidget::eventFilter(obj, event));
}
void stateBar::paintEvent(UNUSED(QPaintEvent *event)) {
	static const int padding = 2;
	static QPen pen;
	static QRect rect;
	int x, y, w, h;

	painter.begin(this);

	// disegno la riga di selezione dello slot
	w = (width() / SAVE_SLOTS);
	h = height();
	x = 0;;
	y = 0;

	if ((x + (SAVE_SLOTS * w)) < width()) {
		x += (width() - (x + (SAVE_SLOTS * w))) / 2;
	}

	for (unsigned int i = 0; i < SAVE_SLOTS; i++) {
		rect.setRect(x + (padding / 2), y, w - padding, h);

		if (save_slot.state[i]) {
			painter.fillRect(rect, Qt::green);
			pen.setColor(Qt::black);
		} else {
			painter.fillRect(rect, palette().mid().color());
			if (isEnabled()) {
				pen.setColor(palette().text().color());
			} else {
				pen.setColor(palette().brightText().color());
			}
		}

		if (save_slot.slot == i) {
			painter.save();
			painter.setOpacity(0.6);
			if (save_slot.state[i]) {
				painter.fillRect(rect, Qt::darkGreen);
				pen.setColor(Qt::white);
			} else {
				painter.fillRect(rect, palette().highlight().color());
				pen.setColor(palette().highlightedText().color());
			}
			painter.restore();
		}

		painter.setPen(pen);
		painter.drawText(rect, Qt::AlignHCenter | Qt::AlignVCenter, QString::number(i, 16).toUpper());

		x += w;
	}

	painter.end();
}

int stateBar::slot_at(QPoint pos) {
	int slot = pos.x() / (width() / SAVE_SLOTS);

	if (slot < SAVE_SLOTS) {
		return (slot);
	}
	return (-1);
}
