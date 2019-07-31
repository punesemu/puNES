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
#include <QtCore/QFileInfo>
#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include "wdgStatusBar.moc"
#include "mainWindow.hpp"
#include "info.h"
#include "video/gfx.h"
#include "ppu.h"
#include "emu.h"
#include "save_slot.h"
#include "text.h"
#include "conf.h"
#include "cheat.h"
#include "patcher.h"

#define SPACING 2
#define DEC_LAB_TLINE 3

// -------------------------------- Statusbar -----------------------------------------

wdgStatusBar::wdgStatusBar(QWidget *parent) : QStatusBar(parent) {
	setObjectName("statusbar");
	setSizeGripEnabled(false);

	layout()->setContentsMargins(QMargins(0,0,0,0));
	layout()->setMargin(0);
	layout()->setSpacing(0);

	//setStyleSheet("QStatusBar::item { border: 1px solid; border-radius: 1px; } ");

	infosb = new infoStatusBar(this);
	addWidget(infosb);

#if defined (__unix__)
	vlineState = new QFrame(this);
	vlineState->setFrameShape(QFrame::VLine);
	vlineState->setFrameShadow(QFrame::Plain);
	vlineState->setFixedWidth(vlineState->sizeHint().width());
	addWidget(vlineState);
#endif

	state = new wdgState(this);
	addWidget(state);

#if defined (__unix__)
	vlineRewind = new QFrame(this);
	vlineRewind->setFrameShape(QFrame::VLine);
	vlineRewind->setFrameShadow(QFrame::Plain);
	vlineRewind->setFixedWidth(vlineRewind->sizeHint().width());
	addWidget(vlineRewind);
#endif

	rewind = new wdgRewind(this);
	addWidget(rewind);

	installEventFilter(this);
}
wdgStatusBar::~wdgStatusBar() {}

bool wdgStatusBar::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		gui_set_focus();
	}

	return (QObject::eventFilter(obj, event));
}
void wdgStatusBar::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		state->retranslateUi();
		update_width(gfx.w[VIDEO_MODE]);
	} else {
		QWidget::changeEvent(event);
	}
}

void wdgStatusBar::update_statusbar(void) {
	infosb->update_label();

	if (info.no_rom | info.turn_off | nsf.state) {
#if defined (__unix__)
		vlineState->setEnabled(false);
#endif
		state->setEnabled(false);

#if defined (__unix__)
		vlineRewind->setEnabled(false);
#endif
		rewind->setEnabled(false);
	} else {
#if defined (__unix__)
		vlineState->setEnabled(true);
#endif
		state->setEnabled(true);

#if defined (__unix__)
		vlineRewind->setEnabled(true);
#endif
		rewind->setEnabled(true);
		state->slot->setCurrentIndex(save_slot.slot);
		state->update();
	}
}
void wdgStatusBar::update_width(int w) {
	setFixedWidth(w);

	w -= (2 + 2);

#if defined (__unix__)
	w -= (state->isVisible() ? vlineState->sizeHint().width() + 2 + 2 : 0);
#endif
	w -= (state->isVisible() ? state->sizeHint().width() + 2 + 2 + 2 : 0);

#if defined (__unix__)
	w -= (rewind->isVisible() ? vlineRewind->sizeHint().width() + 2 + 2 : 0);
#endif
	w -= (rewind->isVisible() ? rewind->sizeHint().width() + 2 + 2 + 2 : 0);

	if (w >= 0) {
		infosb->setFixedWidth(w);
	}
}
void wdgStatusBar::state_setVisible(bool visible) {
#if defined (__unix__)
	vlineState->setVisible(visible);
#endif
	state->setVisible(visible);
}
void wdgStatusBar::rewind_setVisible(bool visible) {
#if defined (__unix__)
	vlineRewind->setVisible(visible);
#endif
	rewind->setVisible(visible);
}

// ---------------------------------- Info --------------------------------------------

infoStatusBar::infoStatusBar(QWidget *parent) : QWidget(parent) {
	hbox = new QHBoxLayout(this);
	hbox->setContentsMargins(QMargins(0,0,0,0));
	hbox->setMargin(0);
	hbox->setSpacing(SPACING);

	setLayout(hbox);

	label = new QLabel(this);
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
	} else {
		if (patch == TRUE) {
			label->setText("*" + QFileInfo(uQString(rom)).fileName());
		} else {
			label->setText(QFileInfo(uQString(rom)).fileName());
		}
	}
}

// -------------------------------- Slot Box ------------------------------------------

slotItemDelegate::slotItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
slotItemDelegate::~slotItemDelegate() {}

void slotItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if (!save_slot.state[index.row()]) {
		QStyleOptionViewItem opt = option;

		opt.palette.setColor(QPalette::Text, Qt::gray);
		QStyledItemDelegate::paint(painter, opt, index);
	} else {
		QStyledItemDelegate::paint(painter, option, index);
	}
}

slotComboBox::slotComboBox(QWidget *parent) : QComboBox(parent) {
	sid = new slotItemDelegate(this);

	for (int i = 0; i < SAVE_SLOTS; i++) {
		addItem(QString("Slot %1").arg(i));
	}

	installEventFilter(parent);
	setItemDelegate(sid);
}
slotComboBox::~slotComboBox() {}
void slotComboBox::paintEvent(UNUSED(QPaintEvent *event)) {
	QStylePainter painter(this);

	// disegno il frame del combobox
	QStyleOptionComboBox opt;
	initStyleOption(&opt);
	painter.drawComplexControl(QStyle::CC_ComboBox, opt);

	// disegno il testo
	if (!save_slot.state[currentIndex()]) {
		painter.setPen(Qt::gray);
		((const wdgState *)parent())->load->setEnabled(false);
	} else {
		((const wdgState *)parent())->load->setEnabled(true);
	}

	QCommonStyle cstyle;
	QRect editRect = cstyle.subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);

	painter.save();
	painter.setClipRect(editRect);
	if (!opt.currentText.isEmpty() && !opt.editable) {
		cstyle.drawItemText(&painter, editRect.adjusted(1, 0, -1, 0),
			cstyle.visualAlignment(opt.direction, Qt::AlignLeft | Qt::AlignVCenter),
			opt.palette, opt.state & QStyle::State_Enabled, opt.currentText);
	}
	painter.restore();

	//painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

wdgState::wdgState(QWidget *parent) : QWidget(parent) {
	hbox = new QHBoxLayout(this);
	hbox->setContentsMargins(QMargins(0, 0, 0, 0));
	hbox->setMargin(0);
	hbox->setSpacing(SPACING);

	setLayout(hbox);

	save = new QPushButton(this);
	save->installEventFilter(this);
	connect(save, SIGNAL(clicked(bool)), this, SLOT(s_save_clicked(bool)));
	hbox->addWidget(save);

	slot = new slotComboBox(this);
	connect(slot, SIGNAL(activated(int)), this, SLOT(s_slot_activated(int)));
	hbox->addWidget(slot);

	load = new QPushButton(this);
	load->installEventFilter(this);
	connect(load, SIGNAL(clicked(bool)), this, SLOT(s_load_clicked(bool)));
	hbox->addWidget(load);

	retranslateUi();
}
wdgState::~wdgState() {}

void wdgState::retranslateUi(void) {
	setToolTip(tr("Save/Load Slot Box"));

	{
		QStyle *pStyle = style();
		QStyleOptionComboBox opt;

		for (int i = 0; i < SAVE_SLOTS; i++) {
			slot->setItemText(i, tr("Slot %1").arg(i));
		}

		opt.initFrom(this);
		QRect rc = pStyle->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);
		slot->setFixedWidth(QLabel(slot->itemText(0)).sizeHint().width() + 12 + rc.width());
	}

	save->setText(tr("Save"));
	save->setFixedWidth(QLabel(save->text()).sizeHint().width() + 12);

	load->setText(tr("Load"));
	load->setFixedWidth(QLabel(load->text()).sizeHint().width() + 12);

	setFixedWidth(save->width() + SPACING +
		slot->width() + SPACING +
		load->width());
}
void wdgState::s_save_clicked(UNUSED(bool checked)) {
	mainwin->action_Save_state->trigger();
	update();
	gui_set_focus();
}
void wdgState::s_slot_activated(int index) {
	save_slot.slot = index;
	text_save_slot(SAVE_SLOT_INCDEC);
	gui_set_focus();
}
void wdgState::s_load_clicked(UNUSED(bool checked)) {
	mainwin->action_Load_state->trigger();
	gui_set_focus();
}
