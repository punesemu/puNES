/*
 * sbarWidget.cpp
 *
 *  Created on: 13/nov/2014
 *      Author: fhorse
 */

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QStylePainter>
#include <QtGui/QCommonStyle>
#else
#include <QtWidgets/QStylePainter>
#include <QtWidgets/QCommonStyle>
#endif
#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include "sbarWidget.moc"
#include "info.h"
#include "timeline.h"
#include "gfx.h"
#include "ppu.h"
#include "emu.h"
#include "save_slot.h"
#include "gui.h"

#define SPACING 2
#define DEC_LAB_TLINE 3

// -------------------------------- Statusbar -----------------------------------------

sbarWidget::sbarWidget(Ui::mainWindow *u, QWidget *parent) : QStatusBar(parent) {
	setObjectName(QString::fromUtf8("statusbar"));
	setSizeGripEnabled(false);

	layout()->setContentsMargins(QMargins(0,0,0,0));
	layout()->setMargin(0);
	layout()->setSpacing(0);

	//setStyleSheet("QStatusBar::item { border: 1px solid; border-radius: 1px; } ");

	spacer = new QWidget(this);
	addWidget(spacer);

	state = new stateWidget(u, this);
	addWidget(state);

	timeline = new timeLine(this);
	addWidget(timeline);
}
sbarWidget::~sbarWidget() {}
void sbarWidget::update_statusbar() {
	if (info.no_rom) {
		state->setEnabled(false);
		timeline->setEnabled(false);
	} else {
		state->setEnabled(true);
		timeline->setEnabled(true);
		state->slot->setCurrentIndex(save_slot.slot);
		state->update();
	}
}
void sbarWidget::update_width(int w) {
	setFixedWidth(w);

	w -= (2 + 2);
	w -= (timeline->isVisible() ? timeline->sizeHint().width() + 2 + 2 + 2 : 0);
	w -= (state->isVisible() ? state->sizeHint().width() + 2 + 2 + 2 : 0);

	spacer->setFixedWidth(w);
}

// -------------------------------- Slot Box ------------------------------------------

slotItemDelegate::slotItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}
slotItemDelegate::~slotItemDelegate() {}
void slotItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
		const QModelIndex &index) const {

	if (!save_slot.state[index.row()]) {
		QStyleOptionViewItem opt = option;

		opt.palette.setColor(QPalette::Text, Qt::gray);
		QStyledItemDelegate::paint(painter, opt, index);
	} else {
		QStyledItemDelegate::paint(painter, option, index);
	}
}

slotComboBox::slotComboBox(QWidget *parent = 0) : QComboBox(parent) {
	sid = new slotItemDelegate(this);

	for (int i = 0; i < SAVE_SLOTS; i++) {
		addItem(trUtf8("Slot %1").arg(i));
	}

	installEventFilter(parent);
	setItemDelegate(sid);
}
slotComboBox::~slotComboBox() {}
void slotComboBox::paintEvent(QPaintEvent *event) {
	QStylePainter painter(this);

	// disegno il frame del combobox
	QStyleOptionComboBox opt;
	initStyleOption(&opt);
	painter.drawComplexControl(QStyle::CC_ComboBox, opt);

	// disegno il testo
	if (!save_slot.state[currentIndex()]) {
		painter.setPen(Qt::gray);
		qobject_cast<const stateWidget *>(parent())->load->setEnabled(false);
	} else {
		qobject_cast<const stateWidget *>(parent())->load->setEnabled(true);
	}

	QCommonStyle cstyle;
	QRect editRect = cstyle.subControlRect(QStyle::CC_ComboBox, &opt,
			QStyle::SC_ComboBoxEditField, this);

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

stateWidget::stateWidget(Ui::mainWindow *u, QWidget *parent = 0) : QWidget(parent) {
	ui = u;

	hbox = new QHBoxLayout(this);
	hbox->setContentsMargins(QMargins(0, 0, 0, 0));
	hbox->setMargin(0);
	hbox->setSpacing(SPACING);

	setToolTip(trUtf8("Save/Load Slot Box"));
	setLayout(hbox);

#if defined (__linux__)
	QFrame *vline = new QFrame(this);
	vline->setFrameShape(QFrame::VLine);
	vline->setFrameShadow(QFrame::Plain);
	vline->setFixedWidth(vline->sizeHint().width());
	hbox->addWidget(vline);
#endif

	save = new QPushButton(this);
	save->setText(trUtf8("Save"));
	save->setFixedWidth(QLabel(save->text()).sizeHint().width() + 12);
	save->installEventFilter(this);
	connect(save, SIGNAL(clicked(bool)), this, SLOT(s_save_clicked(bool)));
	hbox->addWidget(save);

	slot = new slotComboBox(this);
	connect(slot, SIGNAL(activated(int)), this, SLOT(s_slot_activated(int)));
	hbox->addWidget(slot);

	load = new QPushButton(this);
	load->setText(trUtf8("Load"));
	load->setFixedWidth(QLabel(load->text()).sizeHint().width() + 12);
	load->installEventFilter(this);
	connect(load, SIGNAL(clicked(bool)), this, SLOT(s_load_clicked(bool)));
	hbox->addWidget(load);

#if defined (__linux__)
	setFixedWidth(vline->width() + SPACING +
			save->width() + SPACING +
			slot->sizeHint().width() + SPACING +
			load->width());
#else
	setFixedWidth(save->width() + SPACING +
			slot->sizeHint().width() + SPACING +
			load->width());
#endif
}
stateWidget::~stateWidget() {}
bool stateWidget::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		gui_set_focus();
	}

	return (QObject::eventFilter(obj, event));
}
void stateWidget::s_save_clicked(bool checked) {
	ui->action_Save_state->trigger();
	update();
}
void stateWidget::s_slot_activated(int index) {
	save_slot.slot = index;
}
void stateWidget::s_load_clicked(bool checked) {
	ui->action_Load_state->trigger();
}

// -------------------------------- Timeline -----------------------------------------

timelineSlider::timelineSlider(QWidget *parent = 0) : QSlider(Qt::Horizontal, parent) {
	setTickPosition(QSlider::TicksAbove);
	setTickInterval(1);
	setRange(0, TL_SNAPS - 1);
	setFixedWidth(155);
	installEventFilter(parent);

	QStyleOptionSlider opt;
	initStyleOption(&opt);

	szHandle = style()->subControlRect(QStyle::CC_Slider, &opt,
			QStyle::SC_SliderHandle, NULL).width();
}
timelineSlider::~timelineSlider() {}
int timelineSlider::sizeHandle() {
	return (szHandle);
}

timeLine::timeLine(QWidget *parent = 0) : QWidget(parent) {
	hbox = new QHBoxLayout(this);
	hbox->setContentsMargins(QMargins(0,0,0,0));
	hbox->setMargin(0);
	hbox->setSpacing(SPACING);

	lab_timeline = "%1 " + trUtf8("sec");

	setToolTip(trUtf8("Timeline"));
	setLayout(hbox);

#if defined (__linux__)
	QFrame *vline = new QFrame(this);
	vline->setFrameShape(QFrame::VLine);
	vline->setFrameShadow(QFrame::Plain);
	vline->setFixedWidth(vline->sizeHint().width());
	hbox->addWidget(vline);
#endif

	label = new QLabel(lab_timeline.arg(0, DEC_LAB_TLINE), this);
	label->setFixedWidth(QLabel(trUtf8("-00 sec")).sizeHint().width());
	hbox->addWidget(label);

	slider = new timelineSlider(this);
	connect(slider, SIGNAL(actionTriggered(int)), this, SLOT(s_action_triggered(int)));
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(s_value_changed(int)));
	connect(slider, SIGNAL(sliderPressed()), this, SLOT(s_slider_pressed()));
	connect(slider, SIGNAL(sliderReleased()), this, SLOT(s_slider_released()));
	hbox->addWidget(slider);

#if defined (__linux__)
	setFixedWidth(vline->width() + SPACING +
			label->width() + SPACING +
			slider->width());//+  (slider->sizeHandle() / 2));
#else
	setFixedWidth(label->width() + SPACING +
			slider->width());// +  (slider->sizeHandle() / 2));
#endif
}
timeLine::~timeLine() {}
int timeLine::value() {
	return (slider->value());
}
void timeLine::setValue(int value, bool s_action) {
	slider->setValue(value);
	if (s_action == true) {
		s_action_triggered(QAbstractSlider::SliderMove);
	}
}
void timeLine::timeline_pressed(BYTE *type) {
	emu_pause(TRUE);
	(*type) = TRUE;
	if (tl.snaps_fill) {
		// faccio lo screenshot dello screen attuale
		memcpy(tl.snaps[TL_SNAP_FREE] + tl.preview, screen.data, screen_size());
	}
}
void timeLine::timeline_released(BYTE *type) {
	BYTE snap = slider->value();

	if (snap > (tl.snaps_fill - 1)) {
		(*type) = FALSE;
		emu_pause(FALSE);
		timeline_update_label(snap);
		return;
	}

	if (tl.snaps_fill) {
		if (snap != (tl.snaps_fill - 1)) {
			timeline_back(TL_NORMAL, snap);
		}
	}
	gui_set_focus();
	(*type) = FALSE;
	emu_pause(FALSE);
	timeline_update_label(snap);
}
bool timeLine::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::MouseButtonPress) {
		gui_set_focus();
	}

	return (QObject::eventFilter(obj, event));
}
void timeLine::timeline_update_label(int value) {
	if (tl.button) {
		BYTE dec = 0;

		if (tl.snaps_fill) {
			dec = ((tl.snaps_fill - 1) - value) * TL_SNAP_SEC;
		}

		if (!dec) {
			label->setText(lab_timeline.arg(0, DEC_LAB_TLINE));
		} else {
			label->setText(lab_timeline.arg(-abs(((tl.snaps_fill - 1) - value) * TL_SNAP_SEC),
					DEC_LAB_TLINE));
		}
	} else {
		label->setText(lab_timeline.arg(value * TL_SNAP_SEC, DEC_LAB_TLINE));
	}
}
void timeLine::s_action_triggered(int action) {
	int value = slider->sliderPosition();

	switch (action) {
		case QAbstractSlider::SliderSingleStepAdd:
			value += slider->singleStep();
			break;
		case QAbstractSlider::SliderSingleStepSub:
			value -= slider->singleStep();
			break;
		case QAbstractSlider::SliderPageStepAdd:
		case QAbstractSlider::SliderPageStepSub:
			value = slider->value();
			break;
		case QAbstractSlider::SliderToMinimum:
			value = slider->minimum();
			break;
		case QAbstractSlider::SliderToMaximum:
			value = slider->maximum();
			break;
		case QAbstractSlider::SliderMove:
			if (!tl.snaps_fill) {
				value = 0;
				break;
			}

			// value non puo' essere mai maggiore del numero di snap effettuate
			if (value > (tl.snaps_fill - 1)) {
				value = (tl.snaps_fill - 1);
			}

			if (value == (tl.snaps_fill - 1)) {
				memcpy(screen.data, tl.snaps[TL_SNAP_FREE] + tl.preview, screen_size());
				gfx_draw_screen(TRUE);
				break;
			}

			timeline_preview(value);
			break;
		case QAbstractSlider::SliderNoAction:
			break;
	}

	slider->setSliderPosition(value);
	slider->setValue(slider->sliderPosition());
	timeline_update_label(value);
	gui_set_focus();
}
void timeLine::s_value_changed(int value) {
	timeline_update_label(value);
}
void timeLine::s_slider_pressed() {
	timeline_pressed(&tl.button);
}
void timeLine::s_slider_released() {
	timeline_released(&tl.button);
}
