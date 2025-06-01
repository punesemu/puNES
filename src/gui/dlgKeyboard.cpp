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

#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QStylePainter>
#include "dlgKeyboard.hpp"
#include "objSettings.hpp"
#include "nscode.hpp"
#include "mainWindow.hpp"
#include "dlgSettings.hpp"
#include "gui.h"
#include "conf.h"
#include "tas.h"
#include "tape_data_recorder.h"

static dlgKeyboard *dlgkbd = nullptr;

void gui_nes_keyboard(void) {
	if (dlgkbd) {
		emit dlgkbd->et_nes_keyboard();
	}
}
void gui_nes_keyboard_paste_event(void) {
	if (dlgkbd->paste->enable) {
		dlgkbd->paste->parse_text();
	}
}
void gui_nes_keyboard_frame_finished(void) {
	if (dlgkbd->paste->enable) {
		dlgkbd->paste->parse_delay();
	}
}

// ----------------------------------------------------------------------------------------------

wdgDlgKeyboard::wdgDlgKeyboard(QWidget *parent) : wdgTitleBarDialog(parent) {
	wd = new dlgKeyboard(this);
	setWindowTitle(wd->windowTitle());
	setWindowIcon(QIcon(":/icon/icons/virtual_keyboard.svgz"));
	set_border_color(Qt::red);
	set_buttons(barButton::Close);
	set_permit_resize(false);
	add_widget(wd);
	is_in_desktop(&cfg->lg_nes_keyboard.x, &cfg->lg_nes_keyboard.y);
	init_geom_variable(cfg->lg_nes_keyboard);

	connect(wd, SIGNAL(et_adjust_size(void)), this, SLOT(s_adjust_size(void)));
}
wdgDlgKeyboard::~wdgDlgKeyboard() = default;

void wdgDlgKeyboard::resizeEvent(QResizeEvent *event) {
	// sotto wayland (almeno con GNOME) ogni tanto ricevo un evento di Resize
	// spontaneo (con dimensioni non corrette) che provo a filtrare
	if (gfx.wayland.enabled &&  event->spontaneous()) {
		setMaximumSize((event->oldSize()));
	}
	wdgTitleBarDialog::resizeEvent(event);
}
void wdgDlgKeyboard::closeEvent(QCloseEvent *event) {
	geom = geometry();
	wdgTitleBarDialog::closeEvent(event);
}
void wdgDlgKeyboard::hideEvent(QHideEvent *event) {
	geom = geometry();
	wdgTitleBarDialog::hideEvent(event);
	mainwin->wd->statusbar->keyb->update_tooltip();
}

void wdgDlgKeyboard::s_adjust_size(void) {
	adjustSize();
}

// ----------------------------------------------------------------------------------------------

dlgKeyboard::dlgKeyboard(QWidget *parent) : QWidget(parent) {
	setupUi(this);

	{
		QFont f8;

		f8.setPointSize(8);
		f8.setWeight(QFont::Light);
		setFont(f8);
	}

	mode = DK_VIRTUAL;
	paste = new pasteObject(this);

	font_point_size = font().pointSize();

	dlgkbd = this;
	reset();

	pushButton_Mode_virtual->setProperty("mtype", QVariant(DK_VIRTUAL));
	pushButton_Mode_setup->setProperty("mtype", QVariant(DK_SETUP));
	pushButton_Size_10x->setProperty("mtype", QVariant(VK_SIZE_10X));
	pushButton_Size_15x->setProperty("mtype", QVariant(VK_SIZE_15X));
	pushButton_Size_20x->setProperty("mtype", QVariant(VK_SIZE_20X));
	pushButton_Size_25x->setProperty("mtype", QVariant(VK_SIZE_25X));

	connect(pushButton_Mode_virtual, SIGNAL(toggled(bool)), this, SLOT(s_mode(bool)));
	connect(pushButton_Mode_setup, SIGNAL(toggled(bool)), this, SLOT(s_mode(bool)));
	connect(pushButton_Size_10x, SIGNAL(toggled(bool)), this, SLOT(s_size_factor(bool)));
	connect(pushButton_Size_15x, SIGNAL(toggled(bool)), this, SLOT(s_size_factor(bool)));
	connect(pushButton_Size_20x, SIGNAL(toggled(bool)), this, SLOT(s_size_factor(bool)));
	connect(pushButton_Size_25x, SIGNAL(toggled(bool)), this, SLOT(s_size_factor(bool)));
	connect(checkBox_Subor_Extende_Mode, SIGNAL(clicked(bool)), this, SLOT(s_subor_extended_mode(bool)));

	connect(this, SIGNAL(et_nes_keyboard()), this, SLOT(s_nes_keyboard()));

	switch_mode(DK_VIRTUAL);
	switch_size_factor(cfg_from_file.input.vk_size);

	checkBox_Subor_Extende_Mode->setChecked(cfg->input.vk_subor_extended_mode);

	installEventFilter(this);
}
dlgKeyboard::~dlgKeyboard() = default;

bool dlgKeyboard::event(QEvent *event) {
	QKeyEvent *keyevent = dynamic_cast<QKeyEvent*>(event);

	// disabilito la chiusura del dialog tramite il tasto ESC.
	if (keyevent && (keyevent->key() == Qt::Key_Escape)) {
		keyevent->accept();
		return (true);
	}
	return (QWidget::event(event));
}
bool dlgKeyboard::eventFilter(QObject *obj, QEvent *event) {
	if (process_event(event)) {
		return (true);
	}
	return (QWidget::eventFilter(obj, event));
}
void dlgKeyboard::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else if (event->type() == QEvent::ActivationChange) {
		// sotto window, a questo punto, isActiveWindow() mi restituisce true anche quando sono in uscita dall'activation
		// quindi eseguo il shortcut_toggle() con un po' di ritardo per avere il corretto stato della finestra.
		QTimer::singleShot(75, this, [this] {
			shortcut_toggle(this->isActiveWindow());
		});
	}
	QWidget::changeEvent(event);
}
void dlgKeyboard::showEvent(QShowEvent *event) {
	QWidget::showEvent(event);
	mainwin->wd->statusbar->keyb->update_tooltip();
}

void dlgKeyboard::retranslateUi(QWidget *dlgKeyboard) {
	Ui::dlgKeyboard::retranslateUi(dlgKeyboard);
	if (nes_keyboard.enabled) {
		setWindowTitle(keyboard->keyboard_name());
	}
}
void dlgKeyboard::reset(void) {
	memset(kbuttons, 0x00, sizeof(kbuttons));
	one_click.activies = FALSE;
	one_click.list.clear();
	last_press.nscode = 0;
	last_press.time = gui_get_ms();

	paste->reset();

	retranslateUi(this);
}
void dlgKeyboard::add_buttons(wdgKeyboard *wk, wdgKeyboard::_button buttons[], int totals) {
	QList<keyboardButton *> kb_list= wk->findChildren<keyboardButton *>();
	int i;

	for (i = 0; i < totals; i++) {
		wdgKeyboard::_button *btn = &buttons[i];
		int element = (btn->row * nes_keyboard.columns) + btn->column;

		foreach (keyboardButton *kb, kb_list) {
			if (!kb->objectName().compare(btn->object_name, Qt::CaseInsensitive)) {
				DBWORD nscode = settings_inp_nes_keyboard_nscode(uQStringCD(kb->objectName()));

				kb->set(nscode, (SWORD)(nes_keyboard.totals + i), (SBYTE)btn->row, (SBYTE)btn->column, (SWORD)element, btn->modifier, btn->clr, btn->labels);
				kbuttons[nes_keyboard.totals + i] = kb;
				break;
			}
		}
	}
	nes_keyboard.totals += totals;
	retranslateUi(this);
}
void dlgKeyboard::set_buttons(wdgKeyboard *wk, wdgKeyboard::_button buttons[], int totals) {
	reset();
	memset(nes_keyboard.matrix, 0x00, sizeof(nes_keyboard.matrix));
	nes_keyboard.totals = 0;
	add_buttons(wk, buttons, totals);
}

void dlgKeyboard::set_charset(wdgKeyboard::_charset charset, wdgKeyboard::_delay delay) const {
	paste->set_charset(charset, delay);
}
bool dlgKeyboard::process_event(QEvent *event) {
	if ((tas.type == NOTAS) && nes_keyboard.enabled && !dlgsettings->isActiveWindow() && (mode == DK_VIRTUAL)) {
		if (event->type() == QEvent::ShortcutOverride) {
			QKeyEvent *keyevent = (QKeyEvent *)event;

			if (!paste->enable) {
				key_event_press(keyevent, dlgKeyboard::KEVENT_NORMAL);
			} else if (keyevent->key() == Qt::Key_Escape) {
				paste->parse_break();
			}
			return (gui.capture_input);
		} else if (event->type() == QEvent::KeyRelease) {
			if (!paste->enable) {
				key_event_release((QKeyEvent *)event, dlgKeyboard::KEVENT_NORMAL);
			}
			return (gui.capture_input);
		} else if (event->type() == QEvent::Shortcut) {
			if (!paste->enable) {
				QKeySequence key = ((QShortcutEvent *)event)->key();

				if (gui.capture_input &&
					(key != mainwin->wd->shortcut[SET_INP_SC_TOGGLE_CAPTURE_INPUT]->key()) &&
					(key != mainwin->wd->shortcut[SET_INP_SC_TOGGLE_NES_KEYBOARD]->key())) {
					return (true);
				}
			}
		}
	}
	return (false);
}
void dlgKeyboard::shortcut_toggle(BYTE is_this) {
	QObject *parent = nullptr;

	if (is_this) {
		parent = this;
	} else {
		parent = mainwin;
	}
	mainwin->wd->shortcut[SET_INP_SC_TOGGLE_CAPTURE_INPUT]->setParent(parent);
	mainwin->wd->shortcut[SET_INP_SC_TOGGLE_NES_KEYBOARD]->setParent(parent);
}
void dlgKeyboard::button_press(keyboardButton *kb, keyevent_types type) {
	nes_keyboard.matrix[kb->element] = 0x80;

	switch (kb->modifier.type) {
		default:
		case keyboardButton::MODIFIERS_NONE: {
			QList<keyboardButton *>::iterator it = one_click.list.begin();

			while (it != one_click.list.end()) {
				keyboardButton *lkb = (*it);

				switch (lkb->modifier.type) {
					default:
					case keyboardButton::MODIFIERS_NONE:
					case keyboardButton::MODIFIERS_SWITCH:
						it++;
						break;
					case keyboardButton::MODIFIERS_ONE_CLICK:
						if (kb->modifier.state < 2) {
							it = one_click.list.erase(it);
							one_click_oneshot(lkb);
						}
						break;
				}
			}
			break;
		}
		case keyboardButton::MODIFIERS_ONE_CLICK:
			switch (type) {
				case KEVENT_NORMAL:
					kb->modifier.state = (kb->modifier.state + 1) % 3;
					switch (kb->modifier.state) {
						default:
						case 0:
						case 2:
							kb->modifier.state = 1;
							break;
						case 1:
							one_click_inc();
							break;
					}
					one_click_remove(kb);
					break;
				case KEVENT_VIRTUAL:
					kb->modifier.state = (kb->modifier.state + 1) % 3;
					switch (kb->modifier.state) {
						default:
						case 0:
							one_click_dec();
							one_click_remove(kb);
							one_click_oneshot(kb);
							break;
						case 1:
							one_click_inc();
							one_click_append(kb);
							break;
						case 2:
							one_click_remove(kb);
							break;
					}
					break;
			}
			break;
		case keyboardButton::MODIFIERS_SWITCH:
			kb->modifier.state = one_click.activies || kb->modifier.state ? 0x00 : 0x01;
			break;
	}
}
void dlgKeyboard::key_event_press(QKeyEvent *event, keyevent_types type) {
	int i;

	for (i = 0; i < nes_keyboard.totals; i++) {
		if (kbuttons[i]->nscode == (DBWORD)qkeycode::toKeycode(event)) {
			double now = gui_get_ms();

			if ((kbuttons[i]->nscode == last_press.nscode) && ((now - last_press.time) < 70)) {
				break;
			}

			button_press(kbuttons[i], type);
			kbuttons[i]->pressed = TRUE;
			kbuttons[i]->update();
			last_press.nscode = kbuttons[i]->nscode;
			last_press.time = now;
			break;
		}
	}
}
void dlgKeyboard::button_release(keyboardButton *kb, keyevent_types type) {
	switch (kb->modifier.type) {
		default:
		case keyboardButton::MODIFIERS_NONE:
		case keyboardButton::MODIFIERS_SWITCH:
			nes_keyboard.matrix[kb->element] = 0x00;
			break;
		case keyboardButton::MODIFIERS_ONE_CLICK:
			if (type == KEVENT_NORMAL) {
				nes_keyboard.matrix[kb->element] = 0x00;
				kb->modifier.state = 0;
				one_click_dec();
			}
			break;
	}
}
void dlgKeyboard::key_event_release(QKeyEvent *event, keyevent_types type) {
	int i;

	for (i = 0; i < nes_keyboard.totals; i++) {
		if (kbuttons[i]->nscode == (DBWORD)qkeycode::toKeycode(event)) {
			button_release(kbuttons[i], type);
			kbuttons[i]->pressed = FALSE;
			kbuttons[i]->update();
			break;
		}
	}
}
void dlgKeyboard::switch_mode(BYTE dk_mode) {
	QList<themePushButton *> btn_list = widget_Mode->findChildren<themePushButton *>();

	foreach (themePushButton *btn, btn_list) {
		int index = btn->property("mtype").toInt();

		if (index == dk_mode) {
			emit btn->toggled(true);
		}
	}
}

void dlgKeyboard::fake_keyboard(void) {
	paste->cs = Qt::CaseInsensitive;
	widget_Subor->setVisible(false);
	replace_keyboard(new wdgKeyboard(this));
}
void dlgKeyboard::family_basic_keyboard(void) {
	paste->cs = Qt::CaseInsensitive;
	widget_Subor->setVisible(false);
	replace_keyboard(new familyBasicKeyboard(this));
}
void dlgKeyboard::subor_keyboard(void) {
	paste->cs = Qt::CaseSensitive;
	widget_Subor->setVisible(true);
	replace_keyboard(new suborKeyboard(this));
}

void dlgKeyboard::replace_keyboard(wdgKeyboard *wk) {
	if (keyboard) {
		keyboard->hide();
	}
	layout_dlgKeyboard->replaceWidget(keyboard, wk);
	delete (keyboard);
	keyboard = wk;
	keyboard->ext_setup();
	keyboard->show();
	setWindowTitle(keyboard->keyboard_name());
	switch_size_factor(cfg_from_file.input.vk_size);
}
void dlgKeyboard::switch_size_factor(BYTE vk_size) {
	QList<themePushButton *> btn_list = widget_Size->findChildren<themePushButton *>();

	foreach (themePushButton *btn, btn_list) {
		int index = btn->property("mtype").toInt();

		if (index == vk_size) {
			emit btn->toggled(true);
		}
	}
}
BYTE dlgKeyboard::get_size_factor(void) {
	QList<themePushButton *> btn_list = widget_Size->findChildren<themePushButton *>();

	foreach (themePushButton *btn, btn_list) {
		if (btn->isChecked()) {
			return (BYTE)btn->property("mtype").toInt();
		}
	}
	return VK_SIZE_10X;
}
void dlgKeyboard::apply_size_factor(double size_factor) {
	QList<keyboardButton *> kb_list;
	QList<QWidget *> wdg_list = {};

	// opzioni
	wdg_list.append(groupBox_Options);
	wdg_list.append(groupBox_Options->findChildren<QWidget *>());
	foreach (QWidget *wdg, wdg_list) {
		QFont font = wdg->font();

		font.setPointSize(font_point_size * size_factor);
		wdg->setFont(font);
	}
	// keyboard
	kb_list.append(keyboard->findChildren<keyboardButton *>());
	foreach (keyboardButton *kb, kb_list) {
		kb->apply_size_factor(size_factor);
	}
}
bool dlgKeyboard::one_click_find(keyboardButton *kb) {
	QList<keyboardButton *>::iterator it = std::find(one_click.list.begin(), one_click.list.end(), kb);

	if (it == one_click.list.end()) {
		return (false);
	}
	return (true);
}
void dlgKeyboard::one_click_append(keyboardButton *kb) {
	if (!one_click_find(kb)) {
		one_click.list.append(kb);
	}
}
void dlgKeyboard::one_click_remove(keyboardButton *kb) {
	if (one_click_find(kb)) {
		one_click.list.removeOne(kb);
	}
}
void dlgKeyboard::one_click_oneshot(keyboardButton *kb) {
	QTimer::singleShot(75, this, [this, kb] {
		if (kb->modifier.state) {
			this->one_click_dec();
		}
		nes_keyboard.matrix[kb->element] = 0x00;
		kb->modifier.state = 0;
		kb->update();
	});
}
void dlgKeyboard::one_click_inc(void) {
	one_click.activies++;
}
void dlgKeyboard::one_click_dec(void) {
	if (one_click.activies) {
		one_click.activies--;
	}
}
void dlgKeyboard::resize_request(void) {
	QTimer::singleShot(0, this, [this] {
		emit et_adjust_size();
	});
}

void dlgKeyboard::s_nes_keyboard(void) {
	BYTE disable = !nes_keyboard.enabled;

	reset();

	if (nes_keyboard.enabled) {
		switch (cfg->input.expansion) {
			default:
				disable = TRUE;
				break;
			case CTRL_FAMILY_BASIC_KEYBOARD:
				family_basic_keyboard();
				break;
			case CTRL_SUBOR_KEYBOARD:
				subor_keyboard();
				break;
		}
	}
	if (disable) {
		if (!isHidden()) {
			close();
		}
		fake_keyboard();
		reset();
	}
	mainwin->wd->action_Virtual_Keyboard->setEnabled(!disable);
	mainwin->wd->statusbar->keyb->setEnabled(!disable);
	mainwin->wd->update_window();
}
void dlgKeyboard::s_mode(bool checked) {
	if (checked) {
		QList<themePushButton *> btn_list = widget_Mode->findChildren<themePushButton *>();

		foreach (themePushButton *btn, btn_list) {
			if ((themePushButton *)sender() == btn) {
				int index = btn->property("mtype").toInt();

				mode = index;
				update();
				qtHelper::pushbutton_set_checked(btn, true);
			} else {
				qtHelper::pushbutton_set_checked(btn, false);
			}
		}
	}
}
void dlgKeyboard::s_size_factor(bool checked) {
	QList<themePushButton *> btn_list = widget_Size->findChildren<themePushButton *>();

	foreach (themePushButton *btn, btn_list) {
		int index = btn->property("mtype").toInt();
		QPushButton *button = qobject_cast<themePushButton*>(sender());

		if (button == btn) {
			qtHelper::pushbutton_set_checked(btn, true);
			if (checked) {
				apply_size_factor(1.0f + ((double)index * 0.5f));
				resize_request();
				cfg_from_file.input.vk_size = index;
				settings_inp_save();
			}
		} else {
			qtHelper::pushbutton_set_checked(btn, false);
		}
	}
}
void dlgKeyboard::s_subor_extended_mode(UNUSED(bool checked)) {
	cfg->input.vk_subor_extended_mode = !cfg->input.vk_subor_extended_mode;
	settings_inp_save();
}

// ----------------------------------------------------------------------------------------------

wdgDlgCfgNSCode::wdgDlgCfgNSCode(QWidget *parent, keyboardButton *button) : wdgTitleBarDialog(parent) {
	setAttribute(Qt::WA_DeleteOnClose);
	wd = new dlgCfgNSCode(this, button);
	setWindowTitle(wd->windowTitle());
	setWindowIcon(QIcon(":/icon/icons/virtual_keyboard.svgz"));
	set_border_color(Qt::red);
	set_buttons(barButton::Close);
	set_permit_resize(false);
	add_widget(wd);

	connect(wd->pushButton_Discard, SIGNAL(clicked(bool)), this, SLOT(close(void)));
}
wdgDlgCfgNSCode::~wdgDlgCfgNSCode() = default;

// ----------------------------------------------------------------------------------------------

dlgCfgNSCode::dlgCfgNSCode(QWidget *parent, keyboardButton *button) : QWidget(parent) {
	this->button = button;
	nscode = button->nscode;

	setupUi(this);

	{
		QString title = "???";
		int i;

		for (i = 0; i < button->labels.count(); i++) {
			if (!i) {
				title = button->labels.at(i).label.simplified();
			}
			if (button->labels.at(i).bold) {
				title = button->labels.at(i).label.simplified();
				break;
			}
		}
		setWindowTitle(title);
	}

	label_Desc->setText(objInp::nscode_to_name(nscode));
	label_Desc->setFocus(Qt::ActiveWindowFocusReason);

	connect(pushButton_Desc_Default, SIGNAL(clicked(bool)), this, SLOT(s_default_clicked(bool)));
	connect(pushButton_Desc_Unset, SIGNAL(clicked(bool)), this, SLOT(s_unset_clicked(bool)));
	connect(pushButton_Apply, SIGNAL(clicked(bool)), this, SLOT(s_apply_clicked(bool)));

	installEventFilter(this);
}
dlgCfgNSCode::~dlgCfgNSCode() = default;

bool dlgCfgNSCode::eventFilter(QObject *obj, QEvent *event) {
	switch (event->type()) {
		case QEvent::KeyPress:
			return (keypress((QKeyEvent *)event));
		default:
			break;
	}
	return (QWidget::eventFilter(obj, event));
}

bool dlgCfgNSCode::keypress(QKeyEvent *event) {
	nscode = (DBWORD)qkeycode::toKeycode(event);
	label_Desc->setText(objInp::nscode_to_name(nscode));
	return (true);
}

void dlgCfgNSCode::s_default_clicked(UNUSED(bool checked)) {
	nscode = settings_inp_nes_keyboard_nscode_default(uQStringCD(button->objectName()));
	label_Desc->setText(objInp::nscode_to_name(nscode));
}
void dlgCfgNSCode::s_unset_clicked(UNUSED(bool checked)) {
	nscode = 0;
	label_Desc->setText("NULL");
}
void dlgCfgNSCode::s_apply_clicked(UNUSED(bool checked)) {
	button->nscode = nscode;
	settings_inp_nes_keyboard_set_nscode(uQStringCD(button->objectName()), nscode);
	settings_inp_save();
	pushButton_Discard->click();
}

// ----------------------------------------------------------------------------------------------

keyboardButton::keyboardButton(QWidget *parent) : QPushButton(parent) {
	nscode = 0;
	minw = -1;
	minh = -1;
	size_factor = 1.0f;

	reset();

	setText("");
	setFocusPolicy(Qt::NoFocus);
	setAutoDefault(FALSE);
}
keyboardButton::~keyboardButton() = default;

void keyboardButton::paintEvent(QPaintEvent *event) {
	if (!this->isEnabled()) {
		QPushButton::paintEvent(event);
		return;
	}

	{
		QPainter painter(this);
		QStylePainter spainter(this);
		QStyleOptionButton option;
		qreal x, y, w, h;
		QFont font = dlgkbd->font();
		int i, corner = 4;

		initStyleOption(&option);

		if (nes_keyboard.matrix[element] && pressed) {
			option.state |= QStyle::State_Sunken;
		}

		font.setPointSizeF(font.pointSizeF() * size_factor);

		spainter.setFont(font);
		spainter.drawControl(QStyle::CE_PushButton, option);

		painter.setFont(font);
		painter.setRenderHint(QPainter::Antialiasing);

		x = 0;
		y = 0;
		w = (qreal)rect().width();
		h = (qreal)rect().height();

		if (dlgkbd->mode == dlgKeyboard::DK_SETUP) {
			painter.save();

			for (i = 0; i < labels.count(); i++) {
				if (labels.at(i).bold) {
					QFont bold = font;

					bold.setBold(true);
					painter.setFont(bold);

					painter.drawText(QRectF(x, y, w, h - corner),  Qt::AlignHCenter | Qt::AlignBottom, labels.at(i).label);
					break;
				}
			}

			font.setPointSizeF(font.pointSizeF() - 1.0f);

			painter.setFont(font);
			painter.drawText(QRectF(x, y + corner, w, h), Qt::AlignHCenter | Qt::AlignTop,
				objInp::nscode_to_name(nscode).replace("NSCODE_", ""));

			painter.restore();
		} else {
			// disegno le labels
			for (i = 0; i < labels.count(); i++) {
				qreal lx = x, ly = y, lw = w, lh = h;
				int flags;

				painter.save();

				switch (labels.at(i).position) {
					default:
					case LP_CENTER:
						flags = Qt::AlignCenter;
						break;
					case LP_LEFT_TOP:
						flags = Qt::AlignLeft | Qt::AlignTop;
						lx = x + corner;
						ly = y + corner;
						break;
					case LP_LEFT:
						flags = Qt::AlignLeft | Qt::AlignVCenter;
						lx = x + corner;
						break;
					case LP_LEFT_BOTTOM:
						flags = Qt::AlignLeft | Qt::AlignBottom;
						lx = x + corner;
						lh = h - corner;
						break;
					case LP_BOTTOM:
						flags = Qt::AlignHCenter | Qt::AlignBottom;
						lh = h - corner;
						break;
					case LP_RIGHT_BOTTOM:
						flags = Qt::AlignRight | Qt::AlignBottom;
						lw = w - corner;
						lh = h - corner;
						break;
					case LP_RIGHT:
						flags = Qt::AlignRight | Qt::AlignVCenter;
						lw = w - corner;
						break;
					case LP_RIGHT_TOP:
						flags = Qt::AlignRight | Qt::AlignTop;
						ly = y + corner;
						lw = w - corner;
						break;
					case LP_TOP:
						flags = Qt::AlignHCenter | Qt::AlignTop;
						ly = y + corner;
						break;
				}

				if (labels.at(i).bold) {
					QFont bold = font;

					bold.setBold(true);
					painter.setFont(bold);
				}

				painter.drawText(QRectF(lx, ly, lw, lh), flags, labels.at(i).label);
				painter.restore();
			}

			// disegno il simbolo del modifier con il colore appropriato
			if (modifier.type != MODIFIERS_NONE) {
				qreal radius = (4.0f * size_factor);
				QBrush brush(Qt::black);

				x = (qreal)rect().width() - (radius * 2.0f);
				y = (qreal)rect().height() - (radius * 2.0f);

				switch (modifier.type) {
					case MODIFIERS_SWITCH:
						if (nes_keyboard.matrix[element] | modifier.state) {
							brush.setColor(Qt::green);
						}
						break;
					case MODIFIERS_ONE_CLICK:
						switch (modifier.state) {
							default:
							case 0:
								break;
							case 1:
								brush.setColor(Qt::green);
								break;
							case 2:
								brush.setColor(Qt::yellow);
								break;
						}
						break;
					default:
						break;
				}

				painter.setPen(QPen(Qt::black, (2.0f * size_factor)));
				painter.setBrush(brush);
				painter.drawEllipse(QPointF(x, y), radius, radius);
			}
		}
	}
}

void keyboardButton::mousePressEvent(QMouseEvent *event) {
	if (dlgkbd->mode == dlgKeyboard::DK_VIRTUAL) {
		dlgkbd->button_press(this, dlgKeyboard::KEVENT_VIRTUAL);
	}
	QPushButton::mousePressEvent(event);
}
void keyboardButton::mouseReleaseEvent(QMouseEvent *event) {
	if (dlgkbd->mode == dlgKeyboard::DK_SETUP) {
		QTimer::singleShot(75, this, [this] {
			wdgDlgCfgNSCode *cfg = new wdgDlgCfgNSCode(dlgkeyb, this);

			cfg->show();
			update();
		});
	} else {
		dlgkbd->button_release(this, dlgKeyboard::KEVENT_VIRTUAL);
	}
	QPushButton::mouseReleaseEvent(event);
}

void keyboardButton::setMinimumSize(const QSize &s) {
	if (minw < 0) {
		minw = s.width();
	}
	if (minh < 0) {
		minh = s.height();
	}
	QPushButton::setMinimumSize(s);
}

void keyboardButton::apply_size_factor(double factor) {
	int min_width = (int)((double)minw * factor), min_heigth = (int)((double)minh * factor);
	QSize ms = minimumSize();

	if ((min_width > 0) && (min_width != ms.width())) {
		setMinimumWidth(min_width);
	}
	if ((min_heigth > 0) && (min_heigth != ms.height())) {
		setMinimumHeight(min_heigth);
	}
	size_factor = factor;
}
void keyboardButton::set(DBWORD nscode, SWORD index, SBYTE row, SBYTE column, SWORD element, modifier_types mtype,
	const _color &clr, QList<_label> labels) {
	this->row = row;
	this->column = column;
	this->nscode = nscode;
	this->labels = labels;
	this->index = index;
	this->element = element;
	modifier.type = mtype;
	modifier.state = 0;
	pressed = FALSE;
	setEnabled(true);
	{
		QString style =
			"keyboardButton {"\
			"	background-color: %0;"\
			"	border: 1px groove %1;"\
			"	padding: 5px;"\
			"	padding-left: 10px;"\
			"	padding-right: 10px;"\
			"	color: #000;"\
			"}"\
			"keyboardButton:hover {"\
			"	background-color: %2;"\
			"	border: 1px groove %3;"\
			"}"\
			"keyboardButton:pressed {"\
			"	background-color: %4;"\
			"	border: 1px inset %5;"\
			"	padding: 5px;"\
			"	padding-left: 10px;"\
			"	padding-right: 10px;"\
			"}";

		setStyleSheet(
			style
			.arg(clr.bck.isEmpty() ? keyboardButton::_color().bck : clr.bck)
			.arg(clr.bck_border.isEmpty() ? keyboardButton::_color().bck_border : clr.bck_border)
			.arg(clr.hover.isEmpty() ? keyboardButton::_color().hover : clr.hover)
			.arg(clr.hover_border.isEmpty() ? keyboardButton::_color().hover_border : clr.hover_border)
			.arg(clr.press.isEmpty() ? keyboardButton::_color().press : clr.press)
			.arg(clr.press_border.isEmpty() ? keyboardButton::_color().press_border : clr.press_border));
	}
}
void keyboardButton::reset(void) {
	row = -1;
	column = -1;
	index = -1;
	element = -1;
	modifier.type = MODIFIERS_NONE;
	modifier.state = 0;
	labels.clear();
	pressed = FALSE;
	setEnabled(false);
	setStyleSheet("");
}

// ----------------------------------------------------------------------------------------------

wdgKeyboard::wdgKeyboard(QWidget *parent) : QWidget(parent) {
	rows = 0;
	columns = 0;
	delay.set = 2;
	delay.unset = 1;

	setLayoutDirection(Qt::LeftToRight);
}
wdgKeyboard::~wdgKeyboard() = default;

QSize wdgKeyboard::sizeHint(void) const {
	return (QSize(0, 0));
}
QSize wdgKeyboard::minimumSizeHint(void) const {
	return (sizeHint());
}

void wdgKeyboard::init(void) {
	nes_keyboard.enabled = TRUE;
	nes_keyboard.rows = rows;
	nes_keyboard.columns = columns;
	set_buttons();
	set_charset();
}
void wdgKeyboard::set_buttons(void) {}
void wdgKeyboard::set_charset(void) {}

QString wdgKeyboard::keyboard_name(void) {
	return (tr("Virtual Keyboard"));
}
void wdgKeyboard::ext_setup(void) {}
SBYTE wdgKeyboard::calc_element(BYTE row, BYTE column) {
	return ((SBYTE)((row * columns) + column));
}
QList<QList<SBYTE>> wdgKeyboard::parse_character(_character *ch) {
	QList<QList<SBYTE>> elements;
	QList<SBYTE> element;
	int i;

	for (i = 0; i < 4; i++) {
		if (ch->elements[i] > 0) {
			element.append(ch->elements[i]);
		}
	}
	elements.append(element);
	return (elements);
}

// ----------------------------------------------------------------------------------------------

pasteObject::pasteObject(QObject *parent) : QObject(parent) {
	reset();
}
pasteObject::~pasteObject() = default;

void pasteObject::reset(void) {
	enable = FALSE;
	delay.set = 0;
	delay.unset = 0;
	character = nullptr;
	charset.clear();
	parse_reset();
}
void pasteObject::set_charset(wdgKeyboard::_charset charset, wdgKeyboard::_delay delay) {
	int i;

	this->delay.set = delay.set;
	this->delay.unset = delay.unset;

	this->charset.clear();

	for (i = 0; i < charset.length; i++) {
		wdgKeyboard::_character *c = &charset.set[i];

		this->charset.append({ c->string, { c->elements[0], c->elements[1], c->elements[2], c->elements[3] } });
	}
}
void pasteObject::set_text(const QString &text) {
	if (text.isEmpty()) {
		return;
	}
	parse_reset();
	string = text;
	enable = TRUE;
	gui_update_tape_menu();
}
void pasteObject::parse_delay(void) {
	if (delay.counter) {
		delay.counter--;
	}
}
void pasteObject::parse_text(void) {
	int i;

	if (delay.counter) {
		return;
	}

	if (type == PASTE_SET) {
		if (element_index == elements.length()) {
			character = nullptr;

			while (character == nullptr) {
				QString actual;

				if (break_process || (string_index >= string.length())) {
					if (characters_processed > 0) {
						gui_max_speed_stop();
					}
					enable = FALSE;
					gui_update_tape_menu();
					parse_reset();
					return;
				}

				actual = string.mid(string_index, 1);

				if ((actual.compare(QString("\\"), Qt::CaseInsensitive) == 0) && ((string_index + 1) < string.length())) {
					QString next = string.mid(string_index + 1, 1);
					static const QString escape_sequences[] = {
						"n",  // newline
						"t",  // horizontal tab
						"v",  // vertical tab
						"b",  // backspace
						"\"", // double quote
						"r",  // carriage return
						"0",  // the null character
						"f"   // form feed
					};

					for (i = 0; i < (int)LENGTH(escape_sequences); i++) {
						if (!next.compare(escape_sequences[i], Qt::CaseInsensitive)) {
							actual = string.mid(string_index, 2);
							string_index++;
							break;
						}
					}
				}

				for (i = 0; i < charset.count(); i++) {
					BYTE found = FALSE;
					int a;

					for (a = 0; a < charset[i].string.length(); a++) {
						if (!charset[i].string[a].compare(actual, cs)) {
							character = &charset[i];
							elements = dlgkbd->keyboard->parse_character(character);
							found = TRUE;
							break;
						}
					}
					if (found) {
						break;
					}
				}
				string_index++;
			}
			element_index = 0;
		}

		if (characters_processed == 0) {
			gui_max_speed_start();
		}

		characters_processed++;

		for (i = 0; i < elements.at(element_index).length(); i++) {
			nes_keyboard.matrix[elements.at(element_index).at(i)] = 0x80;
		}

		delay.counter = delay.set;
		type = PAST_UNSET;
	} else {
		for (i = 0; i < elements.at(element_index).length(); i++) {
			nes_keyboard.matrix[elements.at(element_index).at(i)] = 0x00;
		}
		element_index++;
		delay.counter = delay.unset;
		type = PASTE_SET;
	}
}
void pasteObject::parse_break(void) {
	break_process = TRUE;
}

void pasteObject::parse_reset(void) {
	type = PASTE_SET;
	string = "";
	string_index = 0;
	characters_processed = 0;
	delay.counter = 0;
	break_process = FALSE;
	elements.clear();
	element_index = 0;
}
void pasteObject::set_elements(BYTE value) {
	if (character) {
		int i;

		for (i = 0; i < 4; i++) {
			if (character->elements[i] > 0) {
				nes_keyboard.matrix[character->elements[i]] = value;
			}
		}
	}
}

// ----------------------------------------------------------------------------------------------

// |-----------|-------------------------------------|-----------------------------------|
// |           |              Column 0               |              Column 1             |
// |-----------|-------------------------------------|-----------------------------------|
// | $4017 bit |    4   |    3   |    2   |    1     |    4   |    3   |    2   |    1   |
// |-----------|-------------------------------------|-----------------------------------|
// |   Row 0   |    ]   |    [   | RETURN |    F8    |  STOP  |    ¥   | RSHIFT |  KANA  |
// |   Row 1   |    ;   |    :   |    @   |    F7    |    ^   |    -   |    /   |    ␣   |
// |   Row 2   |    K   |    L   |    O   |    F6    |    0   |    P   |    ,   |    .   |
// |   Row 3   |    J   |    U   |    I   |    F5    |    8   |    9   |    N   |    M   |
// |   Row 4   |    H   |    G   |    Y   |    F4    |    6   |    7   |    V   |    B   |
// |   Row 5   |    D   |    R   |    T   |    F3    |    4   |    5   |    C   |    F   |
// |   Row 6   |    A   |    S   |    W   |    F2    |    3   |    E   |    Z   |    X   |
// |   Row 7   |   CTR  |    Q   |   ESC  |    F1    |    2   |    1   |  GRPH  | LSHIFT |
// |   Row 8   |  LEFT  | RIGHT  |    UP  | CLR HOME |   INS  |   DEL  | SPACE  |  DOWN  |
// ---------------------------------------------------------------------------------------

familyBasicKeyboard::familyBasicKeyboard(QWidget *parent) : wdgKeyboard(parent) {
	setupUi(this);

	rows = 9;
	columns = 8;
	init();
}
familyBasicKeyboard::~familyBasicKeyboard() {
	tape_data_recorder.enabled = FALSE;
	tape_data_recorder_quit();
}

void familyBasicKeyboard::set_buttons(void) {
	_button buttons[] = {
		// Row 0 - Column 0
		{ "kButton_F8", 0, 0, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F8", true }
			})
		},
		{ "kButton_Return", 0, 1, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "RETURN", true }
			})
		},
		{ "kButton_BracketLeft", 0, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "[", true },
				{ keyboardButton::LP_BOTTOM, "ロ", false },
				{ keyboardButton::LP_RIGHT, "「", false }
			})
		},
		{ "kButton_BracketRight", 0, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "]", true },
				{ keyboardButton::LP_BOTTOM, "。", false },
				{ keyboardButton::LP_RIGHT, "」", false }
			})
		},

		// Row 0 - Column 1
		{ "kButton_Kana", 0, 4, keyboardButton::MODIFIERS_SWITCH, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Kana", true }
			})
		},
		{ "kButton_RShift", 0, 5, keyboardButton::MODIFIERS_ONE_CLICK, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "SHIFT", true }
			})
		},
		{ "kButton_Yen", 0, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "¥", true },
				{ keyboardButton::LP_BOTTOM, "ル", false }
			})
		},
		{ "kButton_Stop", 0, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "STOP", true }
			})
		},

		// Row 1 - Column 0
		{ "kButton_F7", 1, 0, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F7", true }
			})
		},
		{ "kButton_At", 1, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "@", true },
				{ keyboardButton::LP_BOTTOM, "レ", false }
			})
		},
		{ "kButton_Colon", 1, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, ":", true },
				{ keyboardButton::LP_TOP, "*", false },
				{ keyboardButton::LP_BOTTOM, "ー", false }
			})
		},
		{ "kButton_Semicolon", 1, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, ";", true },
				{ keyboardButton::LP_TOP, "+", false },
				{ keyboardButton::LP_BOTTOM, "モ", false }
			})
		},

		// Row 1 - Column 1
		{ "kButton_OpenBox", 1, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_TOP, "␣", true },
				{ keyboardButton::LP_BOTTOM, "ン", false }
			})
		},
		{ "kButton_Slash", 1, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "/", true },
				{ keyboardButton::LP_TOP, "?", false },
				{ keyboardButton::LP_BOTTOM, "ヲ", false }
			})
		},
		{ "kButton_Minus", 1, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "-", true },
				{ keyboardButton::LP_TOP, "=", false },
				{ keyboardButton::LP_BOTTOM, "ラ", false }
			})
		},
		{ "kButton_Circumflex", 1, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "^", true },
				{ keyboardButton::LP_BOTTOM, "リ", false }
			})
		},

		// Row 2 - Column 0
		{ "kButton_F6", 2, 0, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F6", true }
			})
		},
		{ "kButton_O", 2, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "O", true },
				{ keyboardButton::LP_BOTTOM, "ヘ", false },
				{ keyboardButton::LP_RIGHT, "ペ", false }
			})
		},
		{ "kButton_L", 2, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "L", true },
				{ keyboardButton::LP_BOTTOM, "メ", false }
			})
		},
		{ "kButton_K", 2, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "K", true },
				{ keyboardButton::LP_BOTTOM, "ム", false }
			})
		},

		// Row 2 - Column 1
		{ "kButton_Period", 2, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, ".", true },
				{ keyboardButton::LP_TOP, ">", false },
				{ keyboardButton::LP_BOTTOM, "ワ", false }
			})
		},
		{ "kButton_Comma", 2, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, ",", true },
				{ keyboardButton::LP_TOP, "<", false },
				{ keyboardButton::LP_BOTTOM, "ヨ", false },
				{ keyboardButton::LP_RIGHT, "ョ", false }
			})
		},
		{ "kButton_P", 2, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "P", true },
				{ keyboardButton::LP_BOTTOM, "ホ", false },
				{ keyboardButton::LP_RIGHT, "ポ", false }
			})
		},
		{ "kButton_0", 2, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "0", true },
				{ keyboardButton::LP_BOTTOM, "ノ", false }
			})
		},

		// Row 3 - Column 0
		{ "kButton_F5", 3, 0, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F5", true }
			})
		},
		{ "kButton_I", 3, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "I", true },
				{ keyboardButton::LP_BOTTOM, "フ", false },
				{ keyboardButton::LP_RIGHT, "プ", false }
			})
		},
		{ "kButton_U", 3, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "U", true },
				{ keyboardButton::LP_BOTTOM, "ヒ", false },
				{ keyboardButton::LP_RIGHT, "ピ", false }
			})
		},
		{ "kButton_J", 3, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "J", true },
				{ keyboardButton::LP_BOTTOM, "ミ", false }
			})
		},

		// Row 3 - Column 1
		{ "kButton_M", 3, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "M", true },
				{ keyboardButton::LP_BOTTOM, "ユ", false },
				{ keyboardButton::LP_RIGHT, "ュ", false }
			})
		},
		{ "kButton_N", 3, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "N", true },
				{ keyboardButton::LP_BOTTOM, "ヤ", false },
				{ keyboardButton::LP_RIGHT, "ャ", false }
			})
		},
		{ "kButton_9", 3, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "9", true },
				{ keyboardButton::LP_TOP, ")", false },
				{ keyboardButton::LP_BOTTOM, "ネ", false }
			})
		},
		{ "kButton_8", 3, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "8", true },
				{ keyboardButton::LP_TOP, "(", false },
				{ keyboardButton::LP_BOTTOM, "ヌ", false }
			})
		},

		// Row 4 - Column 0
		{ "kButton_F4", 4, 0, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F4", true }
			})
		},
		{ "kButton_Y", 4, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "Y", true },
				{ keyboardButton::LP_BOTTOM, "ハ", false },
				{ keyboardButton::LP_RIGHT, "パ", false }
			})
		},
		{ "kButton_G", 4, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "G", true },
				{ keyboardButton::LP_BOTTOM, "ソ", false }
			})
		},
		{ "kButton_H", 4, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "H", true },
				{ keyboardButton::LP_BOTTOM, "マ", false }
			})
		},

		// Row 4 - Column 1
		{ "kButton_B", 4, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "B", true },
				{ keyboardButton::LP_BOTTOM, "ト", false }
			})
		},
		{ "kButton_V", 4, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "V", true },
				{ keyboardButton::LP_BOTTOM, "テ", false }
			})
		},
		{ "kButton_7", 4, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "7", true },
				{ keyboardButton::LP_TOP, "'", false },
				{ keyboardButton::LP_BOTTOM, "ニ", false }
			})
		},
		{ "kButton_6", 4, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "6", true },
				{ keyboardButton::LP_TOP, "&", false },
				{ keyboardButton::LP_BOTTOM, "ナ", false }
			})
		},

		// Row 5 - Column 0
		{ "kButton_F3", 5, 0, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F3", true }
			})
		},
		{ "kButton_T", 5, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "T", true },
				{ keyboardButton::LP_BOTTOM, "コ", false }
			})
		},
		{ "kButton_R", 5, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "R", true },
				{ keyboardButton::LP_BOTTOM, "ケ", false }
			})
		},
		{ "kButton_D", 5, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "D", true },
				{ keyboardButton::LP_BOTTOM, "ス", false }
			})
		},

		// Row 5 - Column 1
		{ "kButton_F", 5, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "F", true },
				{ keyboardButton::LP_BOTTOM, "セ", false }
			})
		},
		{ "kButton_C", 5, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "C", true },
				{ keyboardButton::LP_BOTTOM, "ツ", false },
				{ keyboardButton::LP_RIGHT, "ッ", false }
			})
		},
		{ "kButton_5", 5, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "5", true },
				{ keyboardButton::LP_TOP, "%", false },
				{ keyboardButton::LP_BOTTOM, "オ", false },
				{ keyboardButton::LP_RIGHT, "ォ", false }
			})
		},
		{ "kButton_4", 5, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "4", true },
				{ keyboardButton::LP_TOP, "$", false },
				{ keyboardButton::LP_BOTTOM, "エ", false },
				{ keyboardButton::LP_RIGHT, "ェ", false }
			})
		},

		// Row 6 - Column 0
		{ "kButton_F2", 6, 0, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F2", true }
			})
		},
		{ "kButton_W", 6, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "W", true },
				{ keyboardButton::LP_BOTTOM, "キ", false }
			})
		},
		{ "kButton_S", 6, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "S", true },
				{ keyboardButton::LP_BOTTOM, "シ", false }
			})
		},
		{ "kButton_A", 6, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "A", true },
				{ keyboardButton::LP_BOTTOM, "サ", false }
			})
		},

		// Row 6 - Column 1
		{ "kButton_X", 6, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "X", true },
				{ keyboardButton::LP_BOTTOM, "チ", false }
			})
		},
		{ "kButton_Z", 6, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "Z", true },
				{ keyboardButton::LP_BOTTOM, "タ", false }
			})
		},
		{ "kButton_E", 6, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "E", true },
				{ keyboardButton::LP_BOTTOM, "ク", false }
			})
		},
		{ "kButton_3", 6, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "3", true },
				{ keyboardButton::LP_TOP, "#", false },
				{ keyboardButton::LP_BOTTOM, "ウ", false },
				{ keyboardButton::LP_RIGHT, "ゥ", false }
			})
		},

		// Row 7 - Column 0
		{ "kButton_F1", 7, 0, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F1", true }
			})
		},
		{ "kButton_Esc", 7, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "ESC", true }
			})
		},
		{ "kButton_Q", 7, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "Q", true },
				{ keyboardButton::LP_BOTTOM, "カ", false }
			})
		},
		{ "kButton_Ctr", 7, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "CTR", true }
			})
		},

		// Row 7 - Column 1
		{ "kButton_LShift", 7, 4, keyboardButton::MODIFIERS_ONE_CLICK, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "SHIFT", true }
			})
		},
		{ "kButton_Grph", 7, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "GRPH", true }
			})
		},
		{ "kButton_1", 7, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "1", true },
				{ keyboardButton::LP_TOP, "!", false },
				{ keyboardButton::LP_BOTTOM, "ア", false },
				{ keyboardButton::LP_RIGHT, "ァ", false }
			})
		},
		{ "kButton_2", 7, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "2", true },
				{ keyboardButton::LP_TOP, "\"", false },
				{ keyboardButton::LP_BOTTOM, "イ", false },
				{ keyboardButton::LP_RIGHT, "ィ", false }
			})
		},

		// Row 8 - Column 0
		{ "kButton_ClrHome", 8, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "CLR\nHOME", true }
			})
		},
		{ "kButton_Up", 8, 1, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "▲", true }
			})
		},
		{ "kButton_Right", 8, 2, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "▶", true }
			})
		},
		{ "kButton_Left", 8, 3, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "◀", true }
			})
		},

		// Row 8 - Column 1
		{ "kButton_Down", 8, 4, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "▼", true }
			})
		},
		{ "kButton_Space", 8, 5, keyboardButton::MODIFIERS_NONE, red_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "SPACE", true }
			})
		},
		{ "kButton_Del", 8, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "DEL", true }
			})
		},
		{ "kButton_Ins", 8, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "INS", true }
			})
		}
	};

	dlgkbd->set_buttons(this, buttons, LENGTH(buttons));
}
void familyBasicKeyboard::set_charset(void) {
	_character charset[] {
		// Row 0 - Column 0
		{ QList<QString> { "\n" }, { calc_element(0, 1), -1, -1, -1 } },
		{ QList<QString> { "[", "［" }, { calc_element(0, 2), -1, -1, -1 } },
		{ QList<QString> { "ロ" }, { calc_element(0, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "「" }, { calc_element(0, 2), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "]", "］"}, { calc_element(0, 3), -1, -1, -1 } },
		{ QList<QString> { "。" }, { calc_element(0, 3), calc_kana(), -1, -1 } },
		{ QList<QString> { "」" }, { calc_element(0, 3), calc_kana(), calc_shift(), -1 } },

		// Row 0 - Column 1
		{ QList<QString> { "¥", "￥" }, { calc_element(0, 6), -1, -1, -1 } },
		{ QList<QString> { "ル" }, { calc_element(0, 6), calc_kana(), -1, -1 } },

		// Row 1 - Column 0
		{ QList<QString> { "@", "＠" }, { calc_element(1, 1), -1, -1, -1 } },
		{ QList<QString> { "レ" }, { calc_element(1, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { ":", "：" }, { calc_element(1, 2), -1, -1, -1 } },
		{ QList<QString> { "*", "＊" }, { calc_element(1, 2), calc_shift(), -1, -1 } },
		{ QList<QString> { "ー" }, { calc_element(1, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { ";", "；" }, { calc_element(1, 3), -1, -1, -1 } },
		{ QList<QString> { "+", "＋" }, { calc_element(1, 3), calc_shift(), -1, -1 } },
		{ QList<QString> { "モ" }, { calc_element(1, 3), calc_kana(), -1, -1 } },

		// Row 1 - Column 1
		{ QList<QString> { "_", "␣" }, { calc_element(1, 4), calc_shift(), -1, -1 } },
		{ QList<QString> { "ン" }, { calc_element(1, 4), calc_kana(), -1, -1 } },
		{ QList<QString> {"/", "／" }, { calc_element(1, 5), -1, -1, -1 } },
		{ QList<QString> { "?", "？" }, { calc_element(1, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "ヲ" }, { calc_element(1, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "-", "－" }, { calc_element(1, 6), -1, -1, -1 } },
		{ QList<QString> { "=", "＝" }, { calc_element(1, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "ラ" }, { calc_element(1, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "^", "＾" }, { calc_element(1, 7), -1, -1, -1 } },
		{ QList<QString> { "リ" }, { calc_element(1, 7), calc_kana(), -1, -1 } },

		// Row 2 - Column 0
		{ QList<QString> { "O", "Ｏ" }, { calc_element(2, 1), -1, -1, -1 } },
		{ QList<QString> { "ヘ" }, { calc_element(2, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "ベ" }, { calc_element(2, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "ペ" }, { calc_element(2, 1), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "L", "Ｌ" }, { calc_element(2, 2), -1, -1, -1 } },
		{ QList<QString> { "メ" }, { calc_element(2, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "K", "Ｋ" }, { calc_element(2, 3), -1, -1, -1 } },
		{ QList<QString> { "ム" }, { calc_element(2, 3), calc_kana(), -1, -1 } },

		// Row 2 - Column 1
		{ QList<QString> { ".", "．" }, { calc_element(2, 4), -1, -1, -1 } },
		{ QList<QString> { ">", "〉" }, { calc_element(2, 4), calc_shift(), -1, -1 } },
		{ QList<QString> { "ワ" }, { calc_element(2, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { ",", "，" }, { calc_element(2, 5), -1, -1, -1 } },
		{ QList<QString> { "<", "〈" }, { calc_element(2, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "ヨ" }, { calc_element(2, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "ョ" }, { calc_element(2, 5), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "P", "Ｐ" }, { calc_element(2, 6), -1, -1, -1 } },
		{ QList<QString> { "ホ" }, { calc_element(2, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "ボ" }, { calc_element(2, 6), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "ポ" }, { calc_element(2, 6), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "0", "０" }, { calc_element(2, 7), -1, -1, -1 } },
		{ QList<QString> { "ノ" }, { calc_element(2, 7), calc_kana(), -1, -1 } },

		// Row 3 - Column 0
		{ QList<QString> { "I", "Ｉ" }, { calc_element(3, 1), -1, -1, -1 } },
		{ QList<QString> { "フ" }, { calc_element(3, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "ブ" }, { calc_element(3, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "プ" }, { calc_element(3, 1), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "U", "Ｕ" }, { calc_element(3, 2), -1, -1, -1 } },
		{ QList<QString> { "ヒ" }, { calc_element(3, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ビ" }, { calc_element(3, 2), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "ピ" }, { calc_element(3, 2), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "J", "Ｊ" }, { calc_element(3, 3), -1, -1, -1 } },
		{ QList<QString> { "ミ" }, { calc_element(3, 3), calc_kana(), -1, -1 } },

		// Row 3 - Column 1
		{ QList<QString> { "M", "Ｍ" }, { calc_element(3, 4), -1, -1, -1 } },
		{ QList<QString> { "ユ" }, { calc_element(3, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { "ュ" }, { calc_element(3, 4), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "N", "Ｎ" }, { calc_element(3, 5), -1, -1, -1 } },
		{ QList<QString> { "ヤ" }, { calc_element(3, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "ャ" }, { calc_element(3, 5), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "9", "９" }, { calc_element(3, 6), -1, -1, -1 } },
		{ QList<QString> { ")", "）" }, { calc_element(3, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "ネ" }, { calc_element(3, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "8", "８" }, { calc_element(3, 7), -1, -1, -1 } },
		{ QList<QString> { "(", "（" }, { calc_element(3, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "ヌ" }, { calc_element(3, 7), calc_kana(), -1, -1 } },

		// Row 4 - Column 0
		{ QList<QString> { "Y", "Ｙ" }, { calc_element(4, 1), -1, -1, -1 } },
		{ QList<QString> { "ハ" }, { calc_element(4, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "バ" }, { calc_element(4, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "パ" }, { calc_element(4, 1), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "G", "Ｇ" }, { calc_element(4, 2), -1, -1, -1 } },
		{ QList<QString> { "ソ" }, { calc_element(4, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゾ" }, { calc_element(4, 2), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "H", "Ｈ" }, { calc_element(4, 3), -1, -1, -1 } },
		{ QList<QString> { "マ" }, { calc_element(4, 3), calc_kana(), -1, -1 } },

		// Row 4 - Column 1
		{ QList<QString> { "B", "Ｂ" }, { calc_element(4, 4), -1, -1, -1 } },
		{ QList<QString> { "ト" }, { calc_element(4, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { "ド" }, { calc_element(4, 4), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "V", "Ｖ" }, { calc_element(4, 5), -1, -1, -1 } },
		{ QList<QString> { "テ" }, { calc_element(4, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "デ" }, { calc_element(4, 5), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "7", "７" }, { calc_element(4, 6), -1, -1, -1 } },
		{ QList<QString> { "'",  "＇" }, { calc_element(4, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "ニ" }, { calc_element(4, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "6", "６" }, { calc_element(4, 7), -1, -1, -1 } },
		{ QList<QString> { "&", "＆" }, { calc_element(4, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "ナ" }, { calc_element(4, 7), calc_kana(), -1, -1 } },

		// Row 5 - Column 0
		{ QList<QString> { "T", "Ｔ" }, { calc_element(5, 1), -1, -1, -1 } },
		{ QList<QString> { "コ" }, { calc_element(5, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゴ" }, { calc_element(5, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "R", "Ｒ" }, { calc_element(5, 2), -1, -1, -1 } },
		{ QList<QString> { "ケ" }, { calc_element(5, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゲ" }, { calc_element(5, 2), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "D", "Ｄ" }, { calc_element(5, 3), -1, -1, -1 } },
		{ QList<QString> { "ス" }, { calc_element(5, 3), calc_kana(), -1, -1 } },
		{ QList<QString> { "ズ" }, { calc_element(5, 3), calc_kana(), calc_grph(), -1 } },

		// Row 5 - Column 1
		{ QList<QString> { "F", "Ｆ" }, { calc_element(5, 4), -1, -1, -1 } },
		{ QList<QString> { "セ" }, { calc_element(5, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゼ" }, { calc_element(5, 4), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "C", "Ｃ" }, { calc_element(5, 5), -1, -1, -1 } },
		{ QList<QString> { "ツ" }, { calc_element(5, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "ヅ" }, { calc_element(5, 5), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "ッ" }, { calc_element(5, 5), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "5", "５" }, { calc_element(5, 6), -1, -1, -1 } },
		{ QList<QString> { "%", "％" }, { calc_element(5, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "オ" }, { calc_element(5, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "ォ" }, { calc_element(5, 6), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "4", "４" }, { calc_element(5, 7), -1, -1, -1 } },
		{ QList<QString> { "$", "＄" }, { calc_element(5, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "エ" }, { calc_element(5, 7), calc_kana(), -1, -1 } },
		{ QList<QString> { "ェ" }, { calc_element(5, 7), calc_kana(), calc_shift(), -1 } },

		// Row 6 - Column 0
		{ QList<QString> { "W", "Ｗ" }, { calc_element(6, 1), -1, -1, -1 } },
		{ QList<QString> { "キ" }, { calc_element(6, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "ギ" }, { calc_element(6, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "S", "Ｓ" }, { calc_element(6, 2), -1, -1, -1 } },
		{ QList<QString> { "シ" }, { calc_element(6, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ジ" }, { calc_element(6, 2), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "A", "Ａ" }, { calc_element(6, 3), -1, -1, -1 } },
		{ QList<QString> { "サ" }, { calc_element(6, 3), calc_kana(), -1, -1 } },
		{ QList<QString> { "ザ" }, { calc_element(6, 3), calc_kana(), calc_grph(), -1 } },

		// Row 6 - Column 1
		{ QList<QString> { "X", "Ｘ" }, { calc_element(6, 4), -1, -1, -1 } },
		{ QList<QString> { "チ" }, { calc_element(6, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { "ヂ" }, { calc_element(6, 4), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "Z", "Ｚ" }, { calc_element(6, 5), -1, -1, -1 } },
		{ QList<QString> { "タ" }, { calc_element(6, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "ダ" }, { calc_element(6, 5), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "E", "Ｅ" }, { calc_element(6, 6), -1, -1, -1 } },
		{ QList<QString> { "ク" }, { calc_element(6, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "グ" }, { calc_element(6, 6), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "3", "３" }, { calc_element(6, 7), -1, -1, -1 } },
		{ QList<QString> { "#", "＃" }, { calc_element(6, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "ウ" }, { calc_element(6, 7), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゥ" }, { calc_element(6, 7), calc_kana(), calc_shift(), -1 } },

		// Row 7 - Column 0
		{ QList<QString> { "Q", "Ｑ" }, { calc_element(7, 2), -1, -1, -1 } },
		{ QList<QString> { "カ" }, { calc_element(7, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ガ" }, { calc_element(7, 2), calc_kana(), calc_grph(), -1 } },

		// Row 7 - Column 1
		{ QList<QString> { "1", "１" }, { calc_element(7, 6), -1, -1, -1 } },
		{ QList<QString> { "!", "！" }, { calc_element(7, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "ア" }, { calc_element(7, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "ァ" }, { calc_element(7, 6), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "2", "２" }, { calc_element(7, 7), -1, -1, -1 } },
		{ QList<QString> { "\"", "＂" }, { calc_element(7, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "イ" }, { calc_element(7, 7), calc_kana(), -1, -1 } },
		{ QList<QString> { "ィ" }, { calc_element(7, 7), calc_kana(), calc_shift(), -1 } },

		// Row 8 - Column 0
		// Row 8 - Column 1
		{ QList<QString> { " ", "　" }, { calc_element(8, 5), -1, -1, -1 } },
	};

	dlgkbd->set_charset({ &charset[0], LENGTH(charset) }, delay);
}

QString familyBasicKeyboard::keyboard_name(void) {
	return (tr("Family Basic Keyboard"));
}
void familyBasicKeyboard::ext_setup(void) {
	tape_data_recorder.enabled = TRUE;
	gui_update_tape_menu();
}
QList<QList<SBYTE>> familyBasicKeyboard::parse_character(wdgKeyboard::_character *ch) {
	QList<QList<SBYTE>> elements;
	QList<SBYTE> element;
	BYTE kana_found = FALSE;
	int i;

	for (i = 0; i < 4; i++) {
		if (ch->elements[i] > 0) {
			if (ch->elements[i] == calc_kana()) {
				kana_found = TRUE;
				continue;
			}
			element.append(ch->elements[i]);
		}
	}
	if (kana_found) {
		QList<SBYTE> kana;

		// CTR + V abilita il kana
		kana.append(calc_ctr());
		kana.append(calc_v());
		elements.append(kana);

		elements.append(element);

		// CTR + W lo disabilita
		kana.clear();
		kana.append(calc_ctr());
		kana.append(calc_w());
		elements.append(kana);
	} else {
		elements.append(element);
	}
	return (elements);
}

keyboardButton::_color familyBasicKeyboard::red_button(void) {
	keyboardButton::_color red;

	red.bck = "#F5BABA";
	red.hover = "#FB9B9B";
	red.press = "#FF7676";
	return (red);
}
SBYTE familyBasicKeyboard::calc_kana(void) {
	return (calc_element(0, 4));
}
SBYTE familyBasicKeyboard::calc_shift(void) {
	return (calc_element(0, 5));
}
SBYTE familyBasicKeyboard::calc_ctr(void) {
	return (calc_element(7, 3));
}
SBYTE familyBasicKeyboard::calc_grph(void) {
	return (calc_element(7, 5));
}
SBYTE familyBasicKeyboard::calc_w(void) {
	return (calc_element(6, 1));
}
SBYTE familyBasicKeyboard::calc_v(void) {
	return (calc_element(4, 5));
}

// ----------------------------------------------------------------------------------------------

// |-----------|-------------------------------------------|--------------------------------------|
// |           |                 Column 0                  |               Column 1               |
// |-----------|-------------------------------------------|--------------------------------------|
// | $4017 bit |    4   |     3     |     2     |    1     |     4    |    3   |    2    |    1   |
// |-----------|-------------------------------------------|--------------------------------------|
// |   Row 0   |    C   |     F     |     G     |    4     |     V    |    5   |    E    |   F2   |
// |   Row 1   |   END  |     S     |     D     |    2     |     X    |    3   |    W    |   F1   |
// |   Row 2   |  RIGHT | PAGE DOWN | BACKSPACE |   INS    |   HOME   | DELETE | PAGE UP |   F8   |
// |   Row 3   |    ,   |     L     |     I     |    9     |     .    |    0   |    O    |   F5   |
// |   Row 4   |  LEFT  |     UP    |  RETURN   |    ]     |   DOWN   |    \   |    [    |   F7   |
// |   Row 5   | PAUSE??|     Z     | CAPSLOCK  |    Q     | LCONTROL |    1   |    A    |  ESC   |
// |   Row 6   |    M   |     K     |     Y     |    7     |     J    |    8   |    U    |   F4   |
// |   Row 7   |    /   |     '     |     ;     |    -     |  LSHIFT  |    =   |    P    |   F6   |
// |   Row 8   |  SPACE |     N     |     H     |    T     |     B    |    6   |    R    |   F3   |
// |   Row 9   | NULL ??|   NULL  ??|   NULL  ??| ALWAYS1  |   NULL ??| NULL ??|  NULL ??| NULL ??|
// |   Row 10  |  F11   |    NP7    |    NP4    |  BREAK ??|    NP8   |  NP2   |   NP1   |  F12   |
// |   Row 11  |  NP9   |    NP*    |    NP+    |   NP-    |  NUMLCK  |  NP/   |   NP5   |  F10   |
// |   Row 12  |  TAB   |    ALT    |    NP6    |    `     |    NP0   |  NP.   |   NP3   |   F9   |
// ------------------------------------------------------------------------------------------------

suborKeyboard::suborKeyboard(QWidget *parent) : wdgKeyboard(parent) {
	setupUi(this);

	rows = 13;
	columns = 8;
	init();
}
suborKeyboard::~suborKeyboard() {
	tape_data_recorder.enabled = FALSE;
	tape_data_recorder_quit();
}

void suborKeyboard::set_buttons(void) {
	_button buttons[] = {
		// Row 0 - Column 0
		{ "kButton_4", 0, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "4", true },
				{ keyboardButton::LP_TOP, "$", false }
			})
		},
		{ "kButton_G", 0, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "G", true }
			})
		},
		{ "kButton_F", 0, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F", true }
			})
		},
		{ "kButton_C", 0, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "C", true }
			})
		},

		// Row 0 - Column 1
		{ "kButton_F2", 0, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F2", true }
			})
		},
		{ "kButton_E", 0, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "E", true }
			})
		},
		{ "kButton_5", 0, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "5", true },
				{ keyboardButton::LP_TOP, "%", false }
			})
		},
		{ "kButton_V", 0, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "V", true }
			})
		},

		// Row 1 - Column 0
		{ "kButton_2", 1, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "2", true },
				{ keyboardButton::LP_TOP, "@", false }
			})
		},
		{ "kButton_D", 1, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "D", true }
			})
		},
		{ "kButton_S", 1, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "S", true }
			})
		},
		{ "kButton_End", 1, 3, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "End", true }
			})
		},

		// Row 1 - Column 1
		{ "kButton_F1", 1, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F1", true }
			})
		},
		{ "kButton_W", 1, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "W", true }
			})
		},
		{ "kButton_3", 1, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "3", true },
				{ keyboardButton::LP_TOP, "#", false }
			})
		},
		{ "kButton_X", 1, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "X", true }
			})
		},

		// Row 2 - Column 0
		{ "kButton_Insert", 2, 0, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Insert", true }
			})
		},
		{ "kButton_Backspace", 2, 1, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "←", true }
			})
		},
		{ "kButton_PageDown", 2, 2, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Page\nDown", true }
			})
		},
		{ "kButton_Right", 2, 3, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "→", true }
			})
		},

		// Row 2 - Column 1
		{ "kButton_F8", 2, 4, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F8", true }
			})
		},
		{ "kButton_PageUp", 2, 5, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Page\nUp", true }
			})
		},
		{ "kButton_Delete", 2, 6, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Delete", true }
			})
		},
		{ "kButton_Home", 2, 7, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Home", true }
			})
		},

		// Row 3 - Column 0
		{ "kButton_9", 3, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "9", true },
				{ keyboardButton::LP_TOP, "(", false }
			})
		},
		{ "kButton_I", 3, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "I", true }
			})
		},
		{ "kButton_L", 3, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "L", true }
			})
		},
		{ "kButton_Comma", 3, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, ",", true },
				{ keyboardButton::LP_TOP, "<", false }
			})
		},

		// Row 3 - Column 1
		{ "kButton_F5", 3, 4, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F5", true }
			})
		},
		{ "kButton_O", 3, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "O", true }
			})
		},
		{ "kButton_0", 3, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "0", true },
				{ keyboardButton::LP_TOP, ")", false }
			})
		},
		{ "kButton_Period", 3, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, ".", true },
				{ keyboardButton::LP_TOP, ">", false }
			})
		},

		// Row 4 - Column 0
		{ "kButton_BracketRight", 4, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "]", true },
				{ keyboardButton::LP_TOP, "}", false }
			})
		},
		{ "kButton_Return", 4, 1, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Enter", true }
			})
		},
		{ "kButton_Enter", 4, 1, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "⏎", true }
			})
		},
		{ "kButton_Up", 4, 2, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "↑", true }
			})
		},
		{ "kButton_Left", 4, 3, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "←", true }
			})
		},

		// Row 4 - Column 1
		{ "kButton_F7", 4, 4, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F7", true }
			})
		},
		{ "kButton_BracketLeft", 4, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "[", true },
				{ keyboardButton::LP_TOP, "{", false }
			})
		},
		{ "kButton_Backslash", 4, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "\\", true },
				{ keyboardButton::LP_TOP, "|", false }
			})
		},
		{ "kButton_Down", 4, 7, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "↓", true }
			})
		},

		// Row 5 - Column 0
		{ "kButton_Q", 5, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Q", true }
			})
		},
		{ "kButton_CapsLock", 5, 1, keyboardButton::MODIFIERS_SWITCH, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Caps", true }
			})
		},
		{ "kButton_Z", 5, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Z", true }
			})
		},
		{ "kButton_Pause", 5, 3, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Pause", true }
			})
		},

		// Row 5 - Column 1
		{ "kButton_Esc", 5, 4, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Esc", true }
			})
		},
		{ "kButton_A", 5, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "A", true }
			})
		},
		{ "kButton_1", 5, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "1", true },
				{ keyboardButton::LP_TOP, "!", false }
			})
		},
		{ "kButton_LControl", 5, 7, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Ctrl", true }
			})
		},
		{ "kButton_RControl", 5, 7, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Ctrl", true }
			})
		},

		// Row 6 - Column 0
		{ "kButton_7", 6, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "7", true },
				{ keyboardButton::LP_TOP, "&", false }
			})
		},
		{ "kButton_Y", 6, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Y", true }
			})
		},
		{ "kButton_K", 6, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "K", true }
			})
		},
		{ "kButton_M", 6, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "M", true }
			})
		},

		// Row 6 - Column 1
		{ "kButton_F4", 6, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F4", true }
			})
		},
		{ "kButton_U", 6, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "U", true }
			})
		},
		{ "kButton_8", 6, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "8", true },
				{ keyboardButton::LP_TOP, "*", false }
			})
		},
		{ "kButton_J", 6, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "J", true }
			})
		},

		// Row 7 - Column 0
		{ "kButton_Minus", 7, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "-", true },
				{ keyboardButton::LP_TOP, "_", false }
			})
		},
		{ "kButton_Semicolon", 7, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, ";", true },
				{ keyboardButton::LP_TOP, ":", false }
			})
		},
		{ "kButton_Apostrophe", 7, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "'", true },
				{ keyboardButton::LP_TOP, "\"", false }
			})
		},
		{ "kButton_Slash", 7, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "/", true },
				{ keyboardButton::LP_TOP, "?", false }
			})
		},

		// Row 7 - Column 1
		{ "kButton_F6", 7, 4, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F6", true }
			})
		},
		{ "kButton_P", 7, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "P", true }
			})
		},
		{ "kButton_Equal", 7, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "=", true },
				{ keyboardButton::LP_TOP, "+", false }
			})
		},
		{ "kButton_LShift", 7, 7, keyboardButton::MODIFIERS_ONE_CLICK, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Shift", true }
			})
		},
		{ "kButton_RShift", 7, 7, keyboardButton::MODIFIERS_ONE_CLICK, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Shift", true }
			})
		},

		// Row 8 - Column 0
		{ "kButton_T", 8, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "T", true }
			})
		},
		{ "kButton_H", 8, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "H", true }
			})
		},
		{ "kButton_N", 8, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "N", true }
			})
		},
		{ "kButton_Space", 8, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Space", true }
			})
		},

		// Row 8 - Column 1
		{ "kButton_F3", 8, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F3", true }
			})
		},
		{ "kButton_R", 8, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "R", true }
			})
		},
		{ "kButton_6", 8, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "6", true },
				{ keyboardButton::LP_TOP, "^", false }
			})
		},
		{ "kButton_B", 8, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "B", true }
			})
		},

		// Row 10 - Column 0
		// Break ???????
		{ "kButton_Break", 10, 0, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Break", true }
			})
		},
		{ "kButton_K4", 10, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "←", false },
				{ keyboardButton::LP_TOP, "4", true }
			})
		},
		{ "kButton_K7", 10, 2, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "Home", false },
				{ keyboardButton::LP_TOP, "7", true }
			})
		},
		{ "kButton_F11", 10, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F11", true }
			})
		},

		// Row 10 - Column 1
		{ "kButton_F12", 10, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F12", true }
			})
		},
		{ "kButton_K1", 10, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "End", false },
				{ keyboardButton::LP_TOP, "1", true }
			})
		},
		{ "kButton_K2", 10, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "↓", false },
				{ keyboardButton::LP_TOP, "2", true }
			})
		},
		{ "kButton_K8", 10, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "↑", false },
				{ keyboardButton::LP_TOP, "8", true }
			})
		},

		// Row 11 - Column 0
		{ "kButton_KMinus", 11, 0, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "-", true }
			})
		},
		{ "kButton_KPlus", 11, 1, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "+", true }
			})
		},
		{ "kButton_KAsterisk", 11, 2, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "*", true }
			})
		},
		{ "kButton_K9", 11, 3, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "PgUp", false },
				{ keyboardButton::LP_TOP, "9", true }
			})
		},

		// Row 11 - Column 1
		{ "kButton_F10", 11, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F10", true }
			})
		},
		{ "kButton_K5", 11, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_TOP, "5", true }
			})
		},
		{ "kButton_KSlash", 11, 6, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "/", true }
			})
		},
		{ "kButton_NumLock", 11, 7, keyboardButton::MODIFIERS_SWITCH, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_LEFT, "Num\nLock", true }
			})
		},

		// Row 12 - Column 0
		{ "kButton_QuoteLeft", 12, 0, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "`", true },
				{ keyboardButton::LP_TOP, "~", false }
			})
		},
		{ "kButton_K6", 12, 1, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "→", false },
				{ keyboardButton::LP_TOP, "6", true }
			})
		},
		{ "kButton_Alt", 12, 2, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Alt", true }
			})
		},
		{ "kButton_AltGr", 12, 2, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Alt", true }
			})
		},
		{ "kButton_Tab", 12, 3, keyboardButton::MODIFIERS_NONE, gray_button(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "Tab", true }
			})
		},

		// Row 12 - Column 1
		{ "kButton_F9", 12, 4, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_CENTER, "F9", true }
			})
		},
		{ "kButton_K3", 12, 5, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "PgDn", false },
				{ keyboardButton::LP_TOP, "3", true }
			})
		},
		{ "kButton_KPeriod", 12, 6, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "Del", false },
				{ keyboardButton::LP_TOP, ".", true }
			})
		},
		{ "kButton_K0", 12, 7, keyboardButton::MODIFIERS_NONE, keyboardButton::_color(),
			QList<keyboardButton::_label>
			({
				{ keyboardButton::LP_BOTTOM, "Ins", false },
				{ keyboardButton::LP_TOP, "0", true }
			})
		}
	};

	dlgkbd->set_buttons(this, buttons, LENGTH(buttons));
}
void suborKeyboard::set_charset(void) {
	_character charset[] {
		// Row 0 - Column 0
		{ QList<QString> { "4", "４" }, { calc_element(0, 0), -1, -1, -1 } },
		{ QList<QString> { "$", "＄" }, { calc_element(0, 1), calc_shift(), -1, -1 } },
		{ QList<QString> { "g" }, { calc_element(0, 1), -1, -1, -1 } },
		{ QList<QString> { "G", "Ｇ" }, { calc_element(0, 1), calc_shift(), -1, -1 } },
		{ QList<QString> { "f" }, { calc_element(0, 2), -1, -1, -1 } },
		{ QList<QString> { "F", "Ｆ" }, { calc_element(0, 2), calc_shift(), -1, -1 } },
		{ QList<QString> { "c" }, { calc_element(0, 3), -1, -1, -1 } },
		{ QList<QString> { "C", "Ｃ" }, { calc_element(0, 3), calc_shift(), -1, -1 } },

		// Row 0 - Column 1
		{ QList<QString> { "e" }, { calc_element(0, 5), -1, -1, -1 } },
		{ QList<QString> { "E", "Ｅ" }, { calc_element(0, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "5", "５" }, { calc_element(0, 6), -1, -1, -1 } },
		{ QList<QString> { "%", "％" }, { calc_element(0, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "v" }, { calc_element(0, 7), -1, -1, -1 } },
		{ QList<QString> { "V", "Ｖ" }, { calc_element(0, 7), calc_shift(), -1, -1 } },

		// Row 1 - Column 0
		{ QList<QString> { "2", "２" }, { calc_element(1, 0), -1, -1, -1 } },
		{ QList<QString> { "@", "＠" }, { calc_element(1, 0), calc_shift(), -1, -1 } },
		{ QList<QString> { "d" }, { calc_element(1, 1), -1, -1, -1 } },
		{ QList<QString> { "D", "Ｄ" }, { calc_element(1, 1), calc_shift(), -1, -1 } },
		{ QList<QString> { "s" }, { calc_element(1, 2), -1, -1, -1 } },
		{ QList<QString> { "S", "Ｓ" }, { calc_element(1, 2), calc_shift(), -1, -1 } },

		// Row 1 - Column 1
		{ QList<QString> { "w" }, { calc_element(1, 5), -1, -1, -1 } },
		{ QList<QString> { "W", "Ｗ" }, { calc_element(1, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "3", "３" }, { calc_element(1, 6), -1, -1, -1 } },
		{ QList<QString> { "#", "＃" }, { calc_element(1, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "x" }, { calc_element(1, 7), -1, -1, -1 } },
		{ QList<QString> { "X", "Ｘ" }, { calc_element(1, 7), calc_shift(), -1, -1 } },

		// Row 2 - Column 0
		// Row 2 - Column 1

		// Row 3 - Column 0
		{ QList<QString> { "9", "９" }, { calc_element(3, 0), -1, -1, -1 } },
		{ QList<QString> { "(", "（" }, { calc_element(3, 0), calc_shift(), -1, -1 } },
		{ QList<QString> { "i" }, { calc_element(3, 1), -1, -1, -1 } },
		{ QList<QString> { "I", "Ｉ" }, { calc_element(3, 1), calc_shift(), -1, -1 } },
		{ QList<QString> { "l" }, { calc_element(3, 2), -1, -1, -1 } },
		{ QList<QString> { "L", "Ｌ" }, { calc_element(3, 2), calc_shift(), -1, -1 } },
		{ QList<QString> { ",", "，" }, { calc_element(3, 3), -1, -1, -1 } },
		{ QList<QString> { "<", "〈" }, { calc_element(3, 3), calc_shift(), -1, -1 } },

		// Row 3 - Column 1
		{ QList<QString> { "o" }, { calc_element(3, 5), -1, -1, -1 } },
		{ QList<QString> { "O", "Ｏ" }, { calc_element(3, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "0", "０" }, { calc_element(3, 6), -1, -1, -1 } },
		{ QList<QString> { ")", "）" }, { calc_element(3, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { ".", "．" }, { calc_element(3, 7), -1, -1, -1 } },
		{ QList<QString> { ">", "〉" }, { calc_element(3, 7), calc_shift(), -1, -1 } },

		// Row 4 - Column 0
		{ QList<QString> { "]", "］"}, { calc_element(4, 0), -1, -1, -1 } },
		{ QList<QString> { "}" }, { calc_element(4, 0), calc_shift(), -1, -1 } },
		{ QList<QString> { "\n" }, { calc_element(4, 1), -1, -1, -1 } },

		// Row 4 - Column 1
		{ QList<QString> { "[", "［" }, { calc_element(4, 5), -1, -1, -1 } },
		{ QList<QString> { "{" }, { calc_element(4, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "\\" }, { calc_element(4, 6), -1, -1, -1 } },
		{ QList<QString> { "|" }, { calc_element(4, 6), calc_shift(), -1, -1 } },

		// Row 5 - Column 0
		{ QList<QString> { "q" }, { calc_element(5, 0), -1, -1, -1 } },
		{ QList<QString> { "Q", "Ｑ" }, { calc_element(5, 0), calc_shift(), -1, -1 } },
		{ QList<QString> { "z" }, { calc_element(5, 2), -1, -1, -1 } },
		{ QList<QString> { "Z", "Ｚ" }, { calc_element(5, 2), calc_shift(), -1, -1 } },

		// Row 5 - Column 1
		{ QList<QString> { "a" }, { calc_element(5, 5), -1, -1, -1 } },
		{ QList<QString> { "A", "Ａ" }, { calc_element(5, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "1", "１" }, { calc_element(5, 6), -1, -1, -1 } },
		{ QList<QString> { "!", "！" }, { calc_element(5, 6), calc_shift(), -1, -1 } },

		// Row 6 - Column 0
		{ QList<QString> { "7", "７" }, { calc_element(6, 0), -1, -1, -1 } },
		{ QList<QString> { "&", "＆" }, { calc_element(6, 0), calc_shift(), -1, -1 } },
		{ QList<QString> { "y" }, { calc_element(6, 1), -1, -1, -1 } },
		{ QList<QString> { "Y", "Ｙ" }, { calc_element(6, 1), calc_shift(), -1, -1 } },
		{ QList<QString> { "k" }, { calc_element(6, 2), -1, -1, -1 } },
		{ QList<QString> { "K", "Ｋ" }, { calc_element(6, 2), calc_shift(), -1, -1 } },
		{ QList<QString> { "m" }, { calc_element(6, 3), -1, -1, -1 } },
		{ QList<QString> { "M", "Ｍ" }, { calc_element(6, 3), calc_shift(), -1, -1 } },

		// Row 6 - Column 1
		{ QList<QString> { "u" }, { calc_element(6, 5), -1, -1, -1 } },
		{ QList<QString> { "U", "Ｕ" }, { calc_element(6, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "8", "８" }, { calc_element(6, 6), -1, -1, -1 } },
		{ QList<QString> { "*", "＊" }, { calc_element(6, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "j" }, { calc_element(6, 7), -1, -1, -1 } },
		{ QList<QString> { "J", "Ｊ" }, { calc_element(6, 7), calc_shift(), -1, -1 } },

		// Row 7 - Column 0
		{ QList<QString> { "-", "－" }, { calc_element(7, 0), -1, -1, -1 } },
		{ QList<QString> { "_", "␣" }, { calc_element(7, 0), calc_shift(), -1, -1 } },
		{ QList<QString> { ";", "；" }, { calc_element(7, 1), -1, -1, -1 } },
		{ QList<QString> { ":", "：" }, { calc_element(7, 1), calc_shift(), -1, -1 } },
		{ QList<QString> { "'",  "＇" }, { calc_element(7, 2), -1, -1, -1 } },
		{ QList<QString> { "\"", "＂" }, { calc_element(7, 2), calc_shift(), -1, -1 } },
		{ QList<QString> {"/", "／" }, { calc_element(7, 3), -1, -1, -1 } },
		{ QList<QString> { "?", "？" }, { calc_element(7, 3), calc_shift(), -1, -1 } },

		// Row 7 - Column 1
		{ QList<QString> { "p" }, { calc_element(7, 5), -1, -1, -1 } },
		{ QList<QString> { "P", "Ｐ" }, { calc_element(7, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "=", "＝" }, { calc_element(7, 6), -1, -1, -1 } },
		{ QList<QString> { "+", "＋" }, { calc_element(7, 6), calc_shift(), -1, -1 } },

		// Row 8 - Column 0
		{ QList<QString> { "t" }, { calc_element(8, 0), -1, -1, -1 } },
		{ QList<QString> { "T", "Ｔ" }, { calc_element(8, 0), calc_shift(), -1, -1 } },
		{ QList<QString> { "h" }, { calc_element(8, 1), -1, -1, -1 } },
		{ QList<QString> { "H", "Ｈ" }, { calc_element(8, 1), calc_shift(), -1, -1 } },
		{ QList<QString> { "n" }, { calc_element(8, 2), -1, -1, -1 } },
		{ QList<QString> { "N", "Ｎ" }, { calc_element(8, 2), calc_shift(), -1, -1 } },
		{ QList<QString> { " ", "　" }, { calc_element(8, 3), -1, -1, -1 } },

		// Row 8 - Column 1
		{ QList<QString> { "r" }, { calc_element(8, 5), -1, -1, -1 } },
		{ QList<QString> { "R", "Ｒ" }, { calc_element(8, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "6", "６" }, { calc_element(8, 6), -1, -1, -1 } },
		{ QList<QString> { "^", "＾" }, { calc_element(8, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "b" }, { calc_element(8, 7), -1, -1, -1 } },
		{ QList<QString> { "B", "Ｂ" }, { calc_element(8, 7), calc_shift(), -1, -1 } },

		// Row 9 - Column 0
		// Row 9 - Column 1
		// Row 10 - Column 0
		// Row 10 - Column 1
		// Row 11 - Column 0
		// Row 11 - Column 1

		// Row 12 - Column 0
		{ QList<QString> { "`" }, { calc_element(12, 0), -1, -1, -1 } },
		{ QList<QString> { "~" }, { calc_element(12, 0), calc_shift(), -1, -1 } },

		// Row 12 - Column 1
	};

	dlgkbd->set_charset({ &charset[0], LENGTH(charset) }, delay);
}

QString suborKeyboard::keyboard_name(void) {
	return (tr("Subor Keyboard"));
}
void suborKeyboard::ext_setup(void) {
	tape_data_recorder.enabled = TRUE;
	gui_update_tape_menu();
}

keyboardButton::_color suborKeyboard::gray_button(void) {
	keyboardButton::_color red;

	red.bck = "#BEC2BA";
	red.hover = "#9DA198";
	red.press = "#90948C";
	return (red);
}
SBYTE suborKeyboard::calc_shift(void) {
	return (calc_element(7, 7));
}
