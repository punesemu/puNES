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

#include <QtCore/QFileInfo>
#include <QtCore/QEvent>
#include <QtWidgets/QLabel>
#include "wdgStatusBar.moc"
#include "mainWindow.hpp"
#include "conf.h"
#include "patcher.h"
#include "recording.h"

// -------------------------------- Statusbar -----------------------------------------

wdgStatusBar::wdgStatusBar(QWidget *parent) : QStatusBar(parent) {
	setObjectName("statusbar");
	setSizeGripEnabled(false);

	layout()->setContentsMargins(QMargins(0,0,0,0));
	layout()->setMargin(0);
	layout()->setSpacing(0);

	//setStyleSheet("QStatusBar::item { border: 1px solid; border-radius: 3px; } ");

	info = new infoStatusBar(this);
	addWidget(info, 1);

	rec = new recStatusBar(this);
	addWidget(rec, 0);

	installEventFilter(this);
}
wdgStatusBar::~wdgStatusBar() {}

bool wdgStatusBar::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		gui_set_focus();
	}

	return (QObject::eventFilter(obj, event));
}
void wdgStatusBar::showEvent(QShowEvent *event) {
	rec->setFixedHeight(info->height());
	QStatusBar::showEvent(event);
}

void wdgStatusBar::update_statusbar(void) {
	info->update_label();
}

// ---------------------------------- Info --------------------------------------------

infoStatusBar::infoStatusBar(QWidget *parent) : QWidget(parent) {
	hbox = new QHBoxLayout(this);
	hbox->setContentsMargins(QMargins(0,0,0,0));
	hbox->setMargin(0);
	hbox->setSpacing(SPACING);

	setLayout(hbox);

	label = new QLabel(this);
	label->setText("[no rom]");
	hbox->addWidget(label);
}
infoStatusBar::~infoStatusBar() {}

void infoStatusBar::update_label(void) {
	BYTE patch = FALSE;
	uTCHAR *rom;

	if ((cfg->cheat_mode == GAMEGENIE_MODE) && (gamegenie.phase == GG_EXECUTE)) {
		rom = gamegenie.rom;
		if (gamegenie.patch) {
			patch = TRUE;
		}
	} else {
		rom = info.rom.file;
	}

	if (patcher.patched == TRUE) {
		patch = TRUE;
	}

	if (info.no_rom | info.turn_off) {
		label->setText("");
		label->setToolTip("");
	} else {
		QFileInfo fileinfo = QFileInfo(uQString(rom));

		if (patch == TRUE) {
			label->setText("*" + fileinfo.fileName());
		} else {
			label->setText(fileinfo.fileName());
		}
		label->setToolTip(fileinfo.filePath());
	}
}

// ----------------------------------- Rec --------------------------------------------

recStatusBar::recStatusBar(QWidget *parent) : QFrame(parent) {
	QHBoxLayout *layout = new QHBoxLayout();

	setFrameShape(QFrame::Panel);
	setFrameShadow(QFrame::Sunken);

	layout->setContentsMargins(QMargins(0,0,0,0));
	layout->setMargin(0);

	desc = new QLabel();
	layout->addWidget(desc);

	icon = new QLabel();
	layout->addWidget(icon);

	setLayout(layout);

	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(s_blink_icon()));
	connect(this, SIGNAL(et_blink_icon()), this, SLOT(s_et_blink_icon()));

	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(s_context_menu(const QPoint&)));
}
recStatusBar::~recStatusBar() {}

void recStatusBar::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		desc_text();
	} else {
		QWidget::changeEvent(event);
	}
}
void recStatusBar::closeEvent(QCloseEvent *event) {
	timer->stop();
	QWidget::closeEvent(event);
}
void recStatusBar::mousePressEvent(QMouseEvent *event) {
	if (event->button() == Qt::LeftButton) {
#if defined (WITH_FFMPEG)
		if (desc->text() == tr("Audio")) {
			mainwin->action_Start_Stop_Audio_recording->trigger();
		} else {
			mainwin->action_Start_Stop_Video_recording->trigger();
		}
#else
		mainwin->action_Start_Stop_Audio_recording->trigger();
#endif
	}
}

void recStatusBar::desc_text(void) {
#if defined (WITH_FFMPEG)
	if (cfg->recording.last_type == REC_FORMAT_AUDIO) {
		desc->setText(tr("Audio"));
	} else {
		desc->setText(tr("Video"));
	}
#else
	desc->setText(tr("Audio"));
#endif
	if (info.recording_on_air) {
		desc->setEnabled(true);
	} else {
		desc->setEnabled(false);
	}
}
void recStatusBar::icon_pixmap(QIcon::Mode mode) {
	icon->setPixmap(QIcon(":/icon/icons/recording_red.svg").pixmap(height(), height() - 4,  mode));
}

void recStatusBar::s_et_blink_icon(void) {
	if (info.recording_on_air) {
		if (timer->isActive() == false) {
			desc_text();
			timer->start(500);
		}
	} else {
		timer->stop();
		desc_text();
		icon_pixmap(QIcon::Disabled);
	}
}
void recStatusBar::s_blink_icon(void) {
	QIcon::Mode mode = QIcon::Disabled;
	static bool state = true;

	if (info.recording_on_air) {
		if (state) {
			mode = QIcon::Normal;
		}
		state = !state;
	}
	icon_pixmap(mode);
}
void recStatusBar::s_context_menu(const QPoint &pos) {
	QPoint global_pos = mapToGlobal(pos);
	QMenu menu;

	menu.addAction(mainwin->action_Start_Stop_Audio_recording);
#if defined (WITH_FFMPEG)
	menu.addAction(mainwin->action_Start_Stop_Video_recording);
#endif
	menu.exec(global_pos);
}
