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

#include <QtWidgets/QStylePainter>
#include <QtWidgets/QCommonStyle>
#include "wdgRotateScreen.moc"
#include "wdgToolBar.hpp"
#include "mainWindow.hpp"
#include "conf.h"
#include "emu_thread.h"
#include "video/gfx.h"

wdgRotateScreen::wdgRotateScreen(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	gridLayout->setHorizontalSpacing(SPACING);

	connect(pushButton_left, SIGNAL(clicked(bool)), this, SLOT(s_rotate_to_left(bool)));
	connect(pushButton_right, SIGNAL(clicked(bool)), this, SLOT(s_rotate_to_right(bool)));

	label_desc->setFixedWidth(QLabel("00000").sizeHint().width());
}
wdgRotateScreen::~wdgRotateScreen() {}

void wdgRotateScreen::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		Ui::wdgRotateScreen::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgRotateScreen::paintEvent(UNUSED(QPaintEvent *event)) {
	QStyleOption opt;
	QPainter p(this);

	opt.init(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void wdgRotateScreen::update_widget(void) {
	QString desc;

	switch (cfg->screen_rotation) {
		default:
			desc = "AAA";
			break;
		case ROTATE_0:
			desc = "0°";
			break;
		case ROTATE_90:
			desc = "90°";
			break;
		case ROTATE_180:
			desc = "180°";
			break;
		case ROTATE_270:
			desc = "270°";
			break;
	}
	label_desc->setText(desc);
}

void wdgRotateScreen::s_rotate_to_left(UNUSED(bool checked)) {
	if (--cfg->screen_rotation >= ROTATE_MAX) {
		cfg->screen_rotation = ROTATE_270;
	}
	emu_thread_pause();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	gui_set_focus();
	emu_thread_continue();
}
void wdgRotateScreen::s_rotate_to_right(UNUSED(bool checked)) {
	if (++cfg->screen_rotation >= ROTATE_MAX) {
		cfg->screen_rotation = ROTATE_0;
	}
	emu_thread_pause();
	gfx_set_screen(NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, NO_CHANGE, TRUE, FALSE);
	gui_set_focus();
	emu_thread_continue();
}
