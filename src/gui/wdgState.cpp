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

#include <QtWidgets/QStylePainter>
#include <QtWidgets/QCommonStyle>
#include "wdgState.moc"
#include "wdgToolBar.hpp"
#include "mainWindow.hpp"
#include "save_slot.h"

wdgState::wdgState(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	gridLayout->setHorizontalSpacing(SPACING);

	pushButton_save->installEventFilter(this);
	pushButton_load->installEventFilter(this);

	connect(pushButton_save, SIGNAL(clicked(bool)), this, SLOT(s_save_clicked(bool)));
	connect(slotComboBox_slot, SIGNAL(activated(int)), this, SLOT(s_slot_activated(int)));
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
void wdgState::paintEvent(UNUSED(QPaintEvent *event)) {
	QStyleOption opt;
	QPainter p(this);

	opt.init(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void wdgState::retranslateUi(wdgState *wdgState) {
	QStyle *pStyle = style();
	QStyleOptionComboBox opt;

	Ui::wdgState::retranslateUi(wdgState);

	for (int i = 0; i < SAVE_SLOTS; i++) {
		slotComboBox_slot->setItemText(i, tr("Slot %1").arg(i));
	}

	opt.initFrom(this);
	QRect rc = pStyle->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxArrow, this);
	slotComboBox_slot->setFixedWidth(QLabel(slotComboBox_slot->itemText(0)).sizeHint().width() + 12 + rc.width());
}

void wdgState::s_save_clicked(UNUSED(bool checked)) {
	mainwin->action_Save_state->trigger();
	update();
	gui_set_focus();
}
void wdgState::s_slot_activated(int index) {
	save_slot.slot = index;
	gui_overlay_enable_save_slot(SAVE_SLOT_INCDEC);
	gui_set_focus();
}
void wdgState::s_load_clicked(UNUSED(bool checked)) {
	mainwin->action_Load_state->trigger();
	gui_set_focus();
}

// ---------------------------------- slotItemDelegate --------------------------------------------

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

// ------------------------------------ slotComboBox ----------------------------------------------

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
		((const wdgState *)parent())->pushButton_load->setEnabled(false);
	} else {
		((const wdgState *)parent())->pushButton_load->setEnabled(true);
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
