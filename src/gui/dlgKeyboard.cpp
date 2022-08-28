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

#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QStylePainter>
#include "dlgKeyboard.moc"
#include "qNativeScanCode.hpp"
#include "mainWindow.hpp"
#include "dlgSettings.hpp"
#include "gui.h"
#include "conf.h"
#include "tas.h"

static dlgKeyboard *dlgkbd = NULL;

void gui_nes_keyboard(void) {
	if (dlgkbd) {
		BYTE disable = !nes_keyboard.enabled;

		if (dlgkeyb->keyboard) {
			delete (dlgkeyb->keyboard);
			dlgkeyb->keyboard = NULL;
		}
		dlgkeyb->reset();

		if (nes_keyboard.enabled) {
			switch (nes_keyboard.type) {
				default:
					disable = TRUE;
					break;
				case CTRL_FAMILY_BASIC_KEYBOARD:
					dlgkeyb->family_basic_keyboard();
					break;
			}
		}
		if (disable) {
			if (!dlgkeyb->isHidden()) {
				dlgkeyb->close();
			}
			dlgkeyb->fake_keyboard();
			dlgkeyb->reset();
		}
		mainwin->action_Virtual_Keyboard->setEnabled(!disable);
		mainwin->statusbar->keyb->setEnabled(!disable);
		mainwin->update_window();
	}
}
void gui_nes_keyboard_paste_event(void) {
	if (dlgkbd->paste->enable) {
		dlgkbd->paste->parse_text();
	}
}

// dlgKeyboard -------------------------------------------------------------------------------------------------------------------

dlgKeyboard::dlgKeyboard(QWidget *parent) : QDialog(parent) {
	QFont f8;

	f8.setPointSize(8);
	f8.setWeight(QFont::Light);
	setFont(f8);

	setupUi(this);

	keyboard = new keyboardObject(this);
	paste = new pasteObject(this);

	{
		QList<keyboardButton *> kb_list = findChildren<keyboardButton *>();

		foreach (keyboardButton *kb, kb_list) {
			if (kb->minimumWidth() == 46) {
				kb->setMinimumWidth(keyboardButton::MIN_W);
			}
			if (kb->minimumHeight() == 46) {
				kb->setMinimumHeight(keyboardButton::MIN_H);
			}
		}
	}

	geom.setX(cfg->lg_nes_keyboard.x);
	geom.setY(cfg->lg_nes_keyboard.y);
	geom.setWidth(cfg->lg_nes_keyboard.w);
	geom.setHeight(cfg->lg_nes_keyboard.h);

	pushButton_Toggle_Keypad->setFocusPolicy(Qt::NoFocus);
	pushButton_Toggle_Keypad->setAutoDefault(FALSE);

	kButton_Escape->scancode = NSCODE_Escape;
	kButton_F1->scancode = NSCODE_F1;
	kButton_F2->scancode = NSCODE_F2;
	kButton_F3->scancode = NSCODE_F3;
	kButton_F4->scancode = NSCODE_F4;
	kButton_F5->scancode = NSCODE_F5;
	kButton_F6->scancode = NSCODE_F6;
	kButton_F7->scancode = NSCODE_F7;
	kButton_F8->scancode = NSCODE_F8;
	kButton_F9->scancode = NSCODE_F9;
	kButton_F10->scancode = NSCODE_F10;
	kButton_F11->scancode = NSCODE_F11;
	kButton_F12->scancode = NSCODE_F12;
	kButton_Print->scancode = NSCODE_Print;
	kButton_ScrollLock->scancode = NSCODE_ScrollLock;
	kButton_Pause->scancode = NSCODE_Pause;

	kButton_QuoteLeft->scancode = NSCODE_QuoteLeft;
	kButton_1->scancode = NSCODE_1;
	kButton_2->scancode = NSCODE_2;
	kButton_3->scancode = NSCODE_3;
	kButton_4->scancode = NSCODE_4;
	kButton_5->scancode = NSCODE_5;
	kButton_6->scancode = NSCODE_6;
	kButton_7->scancode = NSCODE_7;
	kButton_8->scancode = NSCODE_8;
	kButton_9->scancode = NSCODE_9;
	kButton_0->scancode = NSCODE_0;
	kButton_Minus->scancode = NSCODE_Minus;
	kButton_Equal->scancode = NSCODE_Equal;
	kButton_Backspace->scancode = NSCODE_Backspace;
	kButton_Insert->scancode = NSCODE_Insert;
	kButton_Home->scancode = NSCODE_Home;
	kButton_PageUp->scancode = NSCODE_PageUp;
	kButton_NumLock->scancode = NSCODE_NumLock;
	kButton_KSlash->scancode = NSCODE_KSlash;
	kButton_KAsterisk->scancode = NSCODE_KAsterisk;
	kButton_KMinus->scancode = NSCODE_KMinus;

	kButton_Tab->scancode = NSCODE_Tab;
	kButton_Q->scancode = NSCODE_Q;
	kButton_W->scancode = NSCODE_W;
	kButton_E->scancode = NSCODE_E;
	kButton_R->scancode = NSCODE_R;
	kButton_T->scancode = NSCODE_T;
	kButton_Y->scancode = NSCODE_Y;
	kButton_U->scancode = NSCODE_U;
	kButton_I->scancode = NSCODE_I;
	kButton_O->scancode = NSCODE_O;
	kButton_P->scancode = NSCODE_P;
	kButton_BracketLeft->scancode = NSCODE_BracketLeft;
	kButton_BracketRight->scancode = NSCODE_BracketRight;
	kButton_Return->scancode = NSCODE_Return;
	kButton_Delete->scancode = NSCODE_Delete;
	kButton_End->scancode = NSCODE_End;
	kButton_PageDown->scancode = NSCODE_PageDown;
	kButton_K7->scancode = NSCODE_K7;
	kButton_K8->scancode = NSCODE_K8;
	kButton_K9->scancode = NSCODE_K9;
	kButton_KPlus->scancode = NSCODE_KPlus;

	kButton_CapsLock->scancode = NSCODE_CapsLock;
	kButton_A->scancode = NSCODE_A;
	kButton_S->scancode = NSCODE_S;
	kButton_D->scancode = NSCODE_D;
	kButton_F->scancode = NSCODE_F;
	kButton_G->scancode = NSCODE_G;
	kButton_H->scancode = NSCODE_H;
	kButton_J->scancode = NSCODE_J;
	kButton_K->scancode = NSCODE_K;
	kButton_L->scancode = NSCODE_L;
	kButton_Semicolon->scancode = NSCODE_Semicolon;
	kButton_Apostrophe->scancode = NSCODE_Apostrophe;
	kButton_NumberSign->scancode = NSCODE_NumberSign;
	kButton_K4->scancode = NSCODE_K4;
	kButton_K5->scancode = NSCODE_K5;
	kButton_K6->scancode = NSCODE_K6;

	kButton_LShift->scancode = NSCODE_LShift;
	kButton_Backslash->scancode = NSCODE_Backslash;
	kButton_Z->scancode = NSCODE_Z;
	kButton_X->scancode = NSCODE_X;
	kButton_C->scancode = NSCODE_C;
	kButton_V->scancode = NSCODE_V;
	kButton_B->scancode = NSCODE_B;
	kButton_N->scancode = NSCODE_N;
	kButton_M->scancode = NSCODE_M;
	kButton_Comma->scancode = NSCODE_Comma;
	kButton_Period->scancode = NSCODE_Period;
	kButton_Slash->scancode = NSCODE_Slash;
	kButton_RShift->scancode = NSCODE_RShift;
	kButton_Up->scancode = NSCODE_Up;
	kButton_K1->scancode = NSCODE_K1;
	kButton_K2->scancode = NSCODE_K2;
	kButton_K3->scancode = NSCODE_K3;
	kButton_Enter->scancode = NSCODE_Enter;

	kButton_LControl->scancode = NSCODE_LControl;
	kButton_Super_L->scancode = NSCODE_Super_L;
	kButton_Alt->scancode = NSCODE_Alt;
	kButton_Space->scancode = NSCODE_Space;
	kButton_AltGr->scancode = NSCODE_AltGr;
	kButton_Menu->scancode = NSCODE_Menu;
	kButton_RControl->scancode = NSCODE_RControl;
	kButton_Left->scancode = NSCODE_Left;
	kButton_Down->scancode = NSCODE_Down;
	kButton_Right->scancode = NSCODE_Right;
	kButton_K0->scancode = NSCODE_K0;
	kButton_KPeriod->scancode = NSCODE_KPeriod;

	dlgkbd = this;
	reset();

	connect(pushButton_Toggle_Keypad, SIGNAL(clicked(bool)), this, SLOT(s_toggle_keypad(bool)));

	widget_Keypad->setVisible(FALSE);

	installEventFilter(this);
}
dlgKeyboard::~dlgKeyboard() {}

bool dlgKeyboard::event(QEvent *event) {
	QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

	// disabilito la chiusura del dialog tramite il tasto ESC.
	if (keyEvent && (keyEvent->key() == Qt::Key_Escape)) {
		keyEvent->accept();
		return (true);
	}
	return (QDialog::event(event));
}
bool dlgKeyboard::eventFilter(QObject *obj, QEvent *event) {
	if (process_event(event)) {
		return (true);
	}
	return (QObject::eventFilter(obj, event));
}
void dlgKeyboard::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else if (event->type() == QEvent::ActivationChange) {
		// sotto window, a questo punto, isActiveWindow() mi restituisce true anche quando sono in uscita dall'activation
		// quindi eseguo il shortcut_toggle() con un po' di riteardo per avere il corretto stato della finestra.
		QTimer::singleShot(75, [this] {
			shortcut_toggle(this->isActiveWindow());
		});
	}
	QWidget::changeEvent(event);
}
void dlgKeyboard::showEvent(QShowEvent *event) {
	QDialog::showEvent(event);
	mainwin->statusbar->keyb->update_tooltip();
}
void dlgKeyboard::hideEvent(QHideEvent *event) {
	geom = geometry();
	QDialog::hideEvent(event);
	mainwin->statusbar->keyb->update_tooltip();
}
void dlgKeyboard::closeEvent(QCloseEvent *event) {
	event->ignore();
	QTimer::singleShot(50, [this] { setVisible(FALSE); });
}

void dlgKeyboard::retranslateUi(QDialog *dlgKeyboard) {
	Ui::dlgKeyboard::retranslateUi(dlgKeyboard);
	if (nes_keyboard.enabled) {
		switch (nes_keyboard.type) {
			default:
				setWindowTitle(tr("Virtual Keyboard"));
				break;
			case CTRL_FAMILY_BASIC_KEYBOARD:
				setWindowTitle(tr("Family BASIC Keyboard"));
				break;
		}
	}
}
void dlgKeyboard::reset(void) {
	QList<keyboardButton *> kb_list = findChildren<keyboardButton *>();

	foreach (keyboardButton *kb, kb_list) {
		kb->reset();
	}
	memset(keys, 0x00, sizeof(keys));
	one_click.activies = FALSE;
	one_click.list.clear();
	last_press.code = 0;
	last_press.time = gui_get_ms();

	paste->reset();

	retranslateUi(this);
}
void dlgKeyboard::set_keycodes(keyboardObject::_keycode keycodes[], int totals) {
	QList<keyboardButton *> kb_list= findChildren<keyboardButton *>();
	int i;

	reset();

	memset(nes_keyboard.keys, 0x00, sizeof(nes_keyboard.keys));
	memset(nes_keyboard.keycodes, 0x00, sizeof(nes_keyboard.keycodes));

	for (i = 0; i < totals; i++) {
		keyboardObject::_keycode *kc = &keycodes[i];

		nes_keyboard.keycodes[i].code = kc->code;
		nes_keyboard.keycodes[i].row = kc->row;
		nes_keyboard.keycodes[i].column = kc->column;
		nes_keyboard.keycodes[i].index = i;

		foreach (keyboardButton *kb, kb_list) {
			if (kc->code == kb->scancode) {
				kb->set(i, (kc->row * nes_keyboard.columns) + kc->column, kc->modifier, kc->left, kc->up, kc->down, kc->right);
				keys[kb->index] = kb;
				break;
			}
		}
	}

	retranslateUi(this);
}
void dlgKeyboard::set_charset(keyboardObject::_charset charset, keyboardObject::_delay delay) {
	paste->set_charset(charset, delay);
}
bool dlgKeyboard::process_event(QEvent *event) {
	if ((tas.type == NOTAS) && nes_keyboard.enabled && !dlgsettings->isActiveWindow()) {
		if (event->type() == QEvent::ShortcutOverride) {
			QKeyEvent *keyevent = (QKeyEvent *)event;

			if (!paste->enable) {
				key_event_press(keyevent, dlgKeyboard::KEVENT_NORMAL);
			} else if (keyevent->key() == Qt::Key_Escape) {
				paste->parse_break();
			}
			return (true);
		} else if (event->type() == QEvent::KeyRelease) {
			if (!paste->enable) {
				key_event_release((QKeyEvent*)event, dlgKeyboard::KEVENT_NORMAL);
			}
			return (true);
		} else if (event->type() == QEvent::Shortcut) {
			if (!paste->enable) {
				QKeySequence key = ((QShortcutEvent*)event)->key();

				if (gui.capture_input &&
					(key != mainwin->shortcut[SET_INP_SC_TOGGLE_CAPTURE_INPUT]->key()) &&
					(key != mainwin->shortcut[SET_INP_SC_TOGGLE_NES_KEYBOARD]->key())) {
					return (true);
				}
			}
		}
	}
	return (false);
}
void dlgKeyboard::shortcut_toggle(BYTE mode) {
	QObject *parent = NULL;

	if (mode) {
		parent = this;
	} else {
		parent = mainwin;
	}
	mainwin->shortcut[SET_INP_SC_TOGGLE_CAPTURE_INPUT]->setParent(parent);
	mainwin->shortcut[SET_INP_SC_TOGGLE_NES_KEYBOARD]->setParent(parent);
}
void dlgKeyboard::button_press(keyboardButton *kb, key_event_types type) {
	nes_keyboard.keys[kb->key] = 0x80;

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
void dlgKeyboard::key_event_press(QKeyEvent *keyEvent, key_event_types type) {
	int i;

	for (i = 0; i < nes_keyboard.totals; i++) {
#if defined (_WIN32)
		if (nes_keyboard.keycodes[i].code == keyEvent->nativeVirtualKey()) {
#else
		if (nes_keyboard.keycodes[i].code == keyEvent->nativeScanCode()) {
#endif
			double now = gui_get_ms();

			if ((nes_keyboard.keycodes[i].code == last_press.code) && ((now - last_press.time) < 70)) {
				break;
			}

			button_press(keys[nes_keyboard.keycodes[i].index], type);
			keys[nes_keyboard.keycodes[i].index]->pressed = TRUE;
			keys[nes_keyboard.keycodes[i].index]->update();
			last_press.code = nes_keyboard.keycodes[i].code;
			last_press.time = now;
			break;
		}
	}
}
void dlgKeyboard::button_release(keyboardButton *kb, key_event_types type) {
	switch (kb->modifier.type) {
		default:
		case keyboardButton::MODIFIERS_NONE:
		case keyboardButton::MODIFIERS_SWITCH:
			nes_keyboard.keys[kb->key] = 0x00;
			break;
		case keyboardButton::MODIFIERS_ONE_CLICK:
			if (type == KEVENT_NORMAL) {
				nes_keyboard.keys[kb->key] = 0x00;
				kb->modifier.state = 0;
				one_click_dec();
			}
			break;
	}
}
void dlgKeyboard::key_event_release(QKeyEvent *keyEvent, key_event_types type) {
	int i;

	for (i = 0; i < nes_keyboard.totals; i++) {
#if defined (_WIN32)
		if (nes_keyboard.keycodes[i].code == keyEvent->nativeVirtualKey()) {
#else
		if (nes_keyboard.keycodes[i].code == keyEvent->nativeScanCode()) {
#endif
			button_release(keys[nes_keyboard.keycodes[i].index], type);
			keys[nes_keyboard.keycodes[i].index]->pressed = FALSE;
			keys[nes_keyboard.keycodes[i].index]->update();
			break;
		}
	}
}

void dlgKeyboard::fake_keyboard(void) {
	keyboard = new keyboardObject(this);
}
void dlgKeyboard::family_basic_keyboard(void) {
	keyboard = new familyBasicKeyboard(this);
}

bool dlgKeyboard::one_click_find(keyboardButton *kb) {
	QList<keyboardButton *>::iterator it = std::find(one_click.list.begin(), one_click.list.end(), kb);

	if (it == one_click.list.end()) {
		return (false);
	}
	return (true);
}
void dlgKeyboard::one_click_append(keyboardButton *kb) {
	if (one_click_find(kb) == false) {
		one_click.list.append(kb);
	}
}
void dlgKeyboard::one_click_remove(keyboardButton *kb) {
	if (one_click_find(kb) == true) {
		one_click.list.removeOne(kb);
	}
}
void dlgKeyboard::one_click_oneshot(keyboardButton *kb) {
	QTimer::singleShot(75, [this, kb] {
		if (kb->modifier.state) {
			this->one_click_dec();
		}
		nes_keyboard.keys[kb->key] = 0x00;
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

void dlgKeyboard::s_toggle_keypad(UNUSED(bool checked)) {
	widget_Keypad->setVisible(!widget_Keypad->isVisible());
}

// keyboardButton ----------------------------------------------------------------------------------------------------------------

keyboardButton::keyboardButton(QWidget *parent) : QPushButton(parent) {
	scancode = 0;

	reset();

	setText("");
	setFocusPolicy(Qt::NoFocus);
	setAutoDefault(FALSE);
}
keyboardButton::~keyboardButton() {}

void keyboardButton::paintEvent(QPaintEvent *event) {
	if (this->isEnabled() == false) {
		QPushButton::paintEvent(event);
		return;
	}

	{
		QPainter painter(this);
		QStylePainter spainter(this);
		QStyleOptionButton option;
		qreal x, y, w, h;

		initStyleOption(&option);

		if (nes_keyboard.keys[key] && pressed) {
			option.state |= QStyle::State_Sunken;
		}

		spainter.setFont(dlgkbd->font());
		spainter.drawControl(QStyle::CE_PushButton, option);

		painter.setFont(dlgkbd->font());
		painter.setRenderHint(QPainter::Antialiasing);

		painter.save();

		// disegno il testo interno
		{
			int corner = 4;

			x = 0;
			y = 0;
			w = (qreal)rect().width();
			h = (qreal)rect().height();

			if (!down.isEmpty()) {
				painter.drawText(QRectF(x, y, w, h - corner), Qt::AlignHCenter | Qt::AlignBottom, down);
			}
			if (!right.isEmpty()) {
				painter.drawText(QRectF(x, y, w - corner, h), Qt::AlignRight | Qt::AlignVCenter, right);
			}
			if (!up.isEmpty()) {
				painter.drawText(QRectF(x, y + corner, w, h), Qt::AlignHCenter | Qt::AlignTop, up);
			}
			if (!left.isEmpty()) {
				QFont bold = dlgkbd->font();

				bold.setBold(true);

				painter.setFont(bold);

				if (down.isEmpty() && right.isEmpty() && up.isEmpty()) {
					painter.drawText(QRectF(x, y, w, h), Qt::AlignCenter, left);
				} else {
					painter.drawText(QRectF(x + corner, y, w, h), Qt::AlignLeft | Qt::AlignVCenter, left);
				}
			}
		}

		painter.restore();

		// disegno il simbolo del modifier con il colore appropriato
		if (modifier.type != MODIFIERS_NONE) {
			qreal radius = 4.0f;
			QBrush brush(Qt::black);

			x = (qreal)rect().width() - (radius * 2.0f);
			y = (qreal)rect().height() - (radius * 2.0f);

			switch (modifier.type) {
				case MODIFIERS_SWITCH:
					if (nes_keyboard.keys[key] | modifier.state) {
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

			painter.setPen(QPen(Qt::black, 2.0f));
			painter.setBrush(brush);
			painter.drawEllipse(QPointF(x, y), radius, radius);
		}
	}
}

void keyboardButton::mousePressEvent(QMouseEvent *event) {
	dlgkbd->button_press(this, dlgKeyboard::KEVENT_VIRTUAL);
	QPushButton::mousePressEvent(event);
}
void keyboardButton::mouseReleaseEvent(QMouseEvent *event) {
	dlgkbd->button_release(this, dlgKeyboard::KEVENT_VIRTUAL);
	QPushButton::mouseReleaseEvent(event);
}

void keyboardButton::set(SWORD index, SWORD key, modifier_types mtype, QString left, QString up, QString down, QString right) {
	this->index = index;
	this->key = key;
	modifier.type = mtype;
	modifier.state = 0;
	this->left = left;
	this->up = up;
	this->down = down;
	this->right = right;
	pressed = FALSE;
	setEnabled(TRUE);
}
void keyboardButton::reset(void) {
	index = -1;
	key = -1;
	modifier.type = MODIFIERS_NONE;
	modifier.state = 0;
	left = "";
	up = "";
	down = "";
	right = "";
	pressed = FALSE;
	setEnabled(FALSE);
}

// pasteObject -------------------------------------------------------------------------------------------------------------------

pasteObject::pasteObject(QObject *parent) : QObject(parent) {
	reset();
}
pasteObject::~pasteObject() {}

void pasteObject::reset(void) {
	enable = FALSE;
	delay.set = 0;
	delay.unset = 0;
	ch = NULL;
	charset.clear();
	parse_reset();
}
void pasteObject::set_charset(keyboardObject::_charset charset, keyboardObject::_delay delay) {
	int i;

	this->delay.set = delay.set;
	this->delay.unset = delay.unset;

	this->charset.clear();

	for (i = 0; i < charset.length; i++) {
		keyboardObject::_character *c = &charset.set[i];

		this->charset.append({ c->string, { c->keys[0], c->keys[1], c->keys[2], c->keys[3] } });
	}
}
void pasteObject::set_text(QString text) {
	if (text.isEmpty()) {
		return;
	}
	parse_reset();
	string = text;
	enable = TRUE;
}
void pasteObject::parse_text(void) {
	int i;

	if (delay.counter && (--delay.counter)) {
		return;
	}

	if (type == PASTE_SET) {
		if (keys_index == keys.length()) {
			ch = NULL;

			while (ch == NULL) {
				QString character;

				if (break_insert || (string_index >= string.length())) {
					if (characters_elaborate > 0) {
						gui_max_speed_stop();
					}
					enable = FALSE;
					parse_reset();
					return;
				}

				character = string.mid(string_index, 1);

				if ((character.compare(QString("\\"), Qt::CaseInsensitive) == 0) && ((string_index + 1) < string.length())) {
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
					QString second_character = string.mid(string_index + 1, 1);

					for (i = 0; i < (int)LENGTH(escape_sequences); i++) {
						if (second_character.compare(escape_sequences[i], Qt::CaseInsensitive) == 0) {
							character = string.mid(string_index, 2);
							string_index++;
							break;
						}
					}
				}

				for (i = 0; i < charset.count(); i++) {
					BYTE found = FALSE;
					int a;

					for (a = 0; a < charset[i].string.length(); a++) {
						if (charset[i].string[a].compare(character.toUpper(), Qt::CaseInsensitive) == 0) {
							ch = &charset[i];
							keys = dlgkeyb->keyboard->parse_text(ch);
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

			keys_index = 0;
		}

		if (characters_elaborate == 0) {
			gui_max_speed_start();
		}

		characters_elaborate++;

		for (i = 0; i < keys.at(keys_index).length(); i++) {
			nes_keyboard.keys[keys.at(keys_index).at(i)] = 0x80;
		}

		delay.counter = delay.set;
		type = PAST_UNSET;
	} else {
		for (i = 0; i < keys.at(keys_index).length(); i++) {
			nes_keyboard.keys[keys.at(keys_index).at(i)] = 0x00;
		}
		keys_index++;
		delay.counter = delay.unset;
		type = PASTE_SET;
	}
}
void pasteObject::parse_break(void) {
	break_insert = TRUE;
}
void pasteObject::parse_reset(void) {
	type = PASTE_SET;
	string = "";
	string_index = 0;
	characters_elaborate = 0;
	delay.counter = 0;
	break_insert = FALSE;
	keys.clear();
	keys_index = 0;
}
void pasteObject::set_keys(BYTE value) {
	if (ch) {
		int i;

		for (i = 0; i < 4; i++) {
			if (ch->keys[i] > 0) {
				nes_keyboard.keys[ch->keys[i]] = value;
			}
		}
	}
}

// keyboardObject -----------------------------------------------------------------------------------------------------------------

keyboardObject::keyboardObject(QObject *parent) : QObject(parent) {
	rows = 0;
	columns = 0;
	delay.set = 0;
	delay.unset = 0;
}
keyboardObject::~keyboardObject() {}

void keyboardObject::init(void) {
	nes_keyboard.enabled = TRUE;
	nes_keyboard.rows = rows;
	nes_keyboard.columns = columns;
	set_keycodes();
	set_charset();
}
void keyboardObject::set_keycodes(void) {}
void keyboardObject::set_charset(void) {}
QList<QList<SBYTE>> keyboardObject::parse_text(UNUSED(keyboardObject::_character *ch)) {
	QList<QList<SBYTE>> keys;

	return (keys);
}

// familyBasicKeyboard -----------------------------------------------------------------------------------------------------------

//             |              Column 0               |              Column 1             |
// ------------|-------------------------------------|-----------------------------------|
// | $4017 bit |    4   |    3   |    2   |    1     |    4   |    3   |    2   |    1   |
// ------------|-------------------------------------|-----------------------------------|
// |   Row 0   |    ]   |    [   | RETURN |    F8    |  STOP  |    ¥   | RSHIFT |  KANA  |
// |   Row 1   |    ;   |    :   |    @   |    F7    |    ^   |    -   |    /   |    _   |
// |   Row 2   |    K   |    L   |    O   |    F6    |    0   |    P   |    ,   |    .   |
// |   Row 3   |    J   |    U   |    I   |    F5    |    8   |    9   |    N   |    M   |
// |   Row 4   |    H   |    G   |    Y   |    F4    |    6   |    7   |    V   |    B   |
// |   Row 5   |    D   |    R   |    T   |    F3    |    4   |    5   |    C   |    F   |
// |   Row 6   |    A   |    S   |    W   |    F2    |    3   |    E   |    Z   |    X   |
// |   Row 7   |   CTR  |    Q   |   ESC  |    F1    |    2   |    1   |  GRPH  | LSHIFT |
// |   Row 8   |  LEFT  | RIGHT  |    UP  | CLR HOME |   INS  |   DEL  | SPACE  |  DOWN  |
// ---------------------------------------------------------------------------------------

familyBasicKeyboard::familyBasicKeyboard(QObject *parent) : keyboardObject(parent) {
	rows = 9;
	columns = 8;
	delay.set = 12;
	delay.unset = 12;
	init();
}
familyBasicKeyboard::~familyBasicKeyboard() {}

void familyBasicKeyboard::set_keycodes(void) {
	_keycode keycodes[] = {
		                                     /* Row 0 - Column 0 */
		/* F8       */ { NSCODE_F8,           0, 0, keyboardButton::MODIFIERS_NONE,      "F8",    "",  "",  "" },
		/* RETRUN   */ { NSCODE_Return,       0, 1, keyboardButton::MODIFIERS_NONE,      "⏎",     "",  "",  "" },
		/* RETRUN   */ { NSCODE_Enter,        0, 1, keyboardButton::MODIFIERS_NONE,      "⏎",     "",  "",  "" },
		/* [        */ { NSCODE_BracketRight, 0, 2, keyboardButton::MODIFIERS_NONE,      "[",     "",  "ロ", "「" },
		/* ]        */ { NSCODE_NumberSign,   0, 3, keyboardButton::MODIFIERS_NONE,      "]",     "",  "。", "」" },

		                                     /* Row 0 - Column 1 */
		/* KANA     */ { NSCODE_CapsLock,     0, 4, keyboardButton::MODIFIERS_SWITCH,    "Kana",  "",   "",  "" },
		/* RSHIFT   */ { NSCODE_RShift,       0, 5, keyboardButton::MODIFIERS_ONE_CLICK, "Shift", "",   "",  "" },
		/* ¥        */ { NSCODE_Home,         0, 6, keyboardButton::MODIFIERS_NONE,      "¥",     "",   "ル", "" },
		/* STOP     */ { NSCODE_End,          0, 7, keyboardButton::MODIFIERS_NONE,      "Stop",  "",   "",  "" },

		                                     /* Row 1 - Column 0 */
		/* F7       */ { NSCODE_F7,           1, 0, keyboardButton::MODIFIERS_NONE,      "F7",    "",   "",  "" },
		/* @        */ { NSCODE_BracketLeft,  1, 1, keyboardButton::MODIFIERS_NONE,      "@",     "",   "レ", "" },
		/* :        */ { NSCODE_Apostrophe,   1, 2, keyboardButton::MODIFIERS_NONE,      ":",     "*",  "ー", "" },
		/* ;        */ { NSCODE_Semicolon,    1, 3, keyboardButton::MODIFIERS_NONE,      ";",     "+",  "モ", "" },

                                             /* Row 1 - Column 1 */
		/* _        */ { NSCODE_QuoteLeft,    1, 4, keyboardButton::MODIFIERS_NONE,      "",      "␣",  "ン", "" },
		/* /        */ { NSCODE_Slash,        1, 5, keyboardButton::MODIFIERS_NONE,      "/",     "?",  "ヲ", "" },
		/* -        */ { NSCODE_Minus,        1, 6, keyboardButton::MODIFIERS_NONE,      "-",     "=",  "ラ", "" },
		/* ^        */ { NSCODE_Equal,        1, 7, keyboardButton::MODIFIERS_NONE,      "^",     "",   "リ", "" },

		                                     /* Row 2 - Column 0 */
		/* F6       */ { NSCODE_F6,           2, 0, keyboardButton::MODIFIERS_NONE,      "F6",    "",   "",  "" },
		/* O        */ { NSCODE_O,            2, 1, keyboardButton::MODIFIERS_NONE,      "O",     "",   "ヘ", "ペ" },
		/* L        */ { NSCODE_L,            2, 2, keyboardButton::MODIFIERS_NONE,      "L",     "",   "メ", "" },
		/* K        */ { NSCODE_K,            2, 3, keyboardButton::MODIFIERS_NONE,      "K",     "",   "ム", "" },

		                                     /* Row 2 - Column 1 */
		/* .        */ { NSCODE_Period,       2, 4, keyboardButton::MODIFIERS_NONE,      ".",     ">",  "ワ", "" },
		/* ,        */ { NSCODE_Comma,        2, 5, keyboardButton::MODIFIERS_NONE,      ",",     "<",  "ヨ", "" },
		/* P        */ { NSCODE_P,            2, 6, keyboardButton::MODIFIERS_NONE,      "P",     "",   "ホ", "ポ" },
		/* 0        */ { NSCODE_0,            2, 7, keyboardButton::MODIFIERS_NONE,      "0",     "",   "ノ", "" },

		                                     /* Row 3 - Column 0 */
		/* F5       */ { NSCODE_F5,           3, 0, keyboardButton::MODIFIERS_NONE,      "F5",    "",   "",  "" },
		/* I        */ { NSCODE_I,            3, 1, keyboardButton::MODIFIERS_NONE,      "I",     "",   "フ", "プ" },
		/* U        */ { NSCODE_U,            3, 2, keyboardButton::MODIFIERS_NONE,      "U",     "",   "ヒ", "ピ" },
		/* J        */ { NSCODE_J,            3, 3, keyboardButton::MODIFIERS_NONE,      "J",     "",   "ミ", "" },

		                                     /* Row 3 - Column 1 */
		/* M        */ { NSCODE_M,            3, 4, keyboardButton::MODIFIERS_NONE,      "M",     "",   "ユ", "" },
		/* N        */ { NSCODE_N,            3, 5, keyboardButton::MODIFIERS_NONE,      "N",     "",   "ヤ", "" },
		/* 9        */ { NSCODE_9,            3, 6, keyboardButton::MODIFIERS_NONE,      "9",     ")",  "ネ", "" },
		/* 8        */ { NSCODE_8,            3, 7, keyboardButton::MODIFIERS_NONE,      "8",     "(",  "ヌ", "" },

		                                     /* Row 4 - Column 0 */
		/* F4       */ { NSCODE_F4,           4, 0, keyboardButton::MODIFIERS_NONE,      "F4",    "",   "",  "" },
		/* Y        */ { NSCODE_Y,            4, 1, keyboardButton::MODIFIERS_NONE,      "Y",     "",   "ハ", "パ" },
		/* G        */ { NSCODE_G,            4, 2, keyboardButton::MODIFIERS_NONE,      "G",     "",   "ソ", "" },
		/* H        */ { NSCODE_H,            4, 3, keyboardButton::MODIFIERS_NONE,      "H",     "",   "マ", "" },

		                                     /* Row 4 - Column 1 */
		/* B        */ { NSCODE_B,            4, 4, keyboardButton::MODIFIERS_NONE,      "B",     "",   "ト", "" },
		/* V        */ { NSCODE_V,            4, 5, keyboardButton::MODIFIERS_NONE,      "V",     "",   "テ", "" },
		/* 7        */ { NSCODE_7,            4, 6, keyboardButton::MODIFIERS_NONE,      "7",     "'",  "ニ", "" },
		/* 6        */ { NSCODE_6,            4, 7, keyboardButton::MODIFIERS_NONE,      "6",     "&",  "ナ", "" },

		                                     /* Row 5 - Column 0 */
		/* F3       */ { NSCODE_F3,           5, 0, keyboardButton::MODIFIERS_NONE,      "F3",    "",   "",  "" },
		/* T        */ { NSCODE_T,            5, 1, keyboardButton::MODIFIERS_NONE,      "T",     "",   "コ", "" },
		/* R        */ { NSCODE_R,            5, 2, keyboardButton::MODIFIERS_NONE,      "R",     "",   "ケ", "" },
		/* D        */ { NSCODE_D,            5, 3, keyboardButton::MODIFIERS_NONE,      "D",     "",   "ス", "" },

		                                     /* Row 5 - Column 1 */
		/* F        */ { NSCODE_F,            5, 4, keyboardButton::MODIFIERS_NONE,      "F",     "",   "セ", "" },
		/* C        */ { NSCODE_C,            5, 5, keyboardButton::MODIFIERS_NONE,      "C",     "",   "ツ", "" },
		/* 5        */ { NSCODE_5,            5, 6, keyboardButton::MODIFIERS_NONE,      "5",     "%",  "オ", "ォ" },
		/* 4        */ { NSCODE_4,            5, 7, keyboardButton::MODIFIERS_NONE,      "4",     "$",  "エ", "ェ" },

		                                     /* Row 6 - Column 0 */
		/* F2       */ { NSCODE_F2,           6, 0, keyboardButton::MODIFIERS_NONE,      "F2",    "",   "",  "" },
		/* W        */ { NSCODE_W,            6, 1, keyboardButton::MODIFIERS_NONE,      "W",     "",   "キ", "" },
		/* S        */ { NSCODE_S,            6, 2, keyboardButton::MODIFIERS_NONE,      "S",     "",   "シ", "" },
		/* A        */ { NSCODE_A,            6, 3, keyboardButton::MODIFIERS_NONE,      "A",     "",   "サ", "" },

		                                     /* Row 6 - Column 1 */
		/* X        */ { NSCODE_X,            6, 4, keyboardButton::MODIFIERS_NONE,      "X",     "",   "チ", "" },
		/* Z        */ { NSCODE_Z,            6, 5, keyboardButton::MODIFIERS_NONE,      "Z",     "",   "タ", "" },
		/* E        */ { NSCODE_E,            6, 6, keyboardButton::MODIFIERS_NONE,      "E",     "",   "ク", "" },
		/* 3        */ { NSCODE_3,            6, 7, keyboardButton::MODIFIERS_NONE,      "3",     "#",  "ウ", "ゥ" },

		                                     /* Row 7 - Column 0 */
		/* F1       */ { NSCODE_F1,           7, 0, keyboardButton::MODIFIERS_NONE,      "F1",    "",   "",  "" },
		/* ESC      */ { NSCODE_Escape,       7, 1, keyboardButton::MODIFIERS_NONE,      "Esc",   "",   "",  "" },
		/* Q        */ { NSCODE_Q,            7, 2, keyboardButton::MODIFIERS_NONE,      "Q",     "",   "カ", "" },
		/* CTR      */ { NSCODE_LControl,     7, 3, keyboardButton::MODIFIERS_NONE,      "CTR",   "",   "",  "" },
		/* CTR      */ { NSCODE_RControl,     7, 3, keyboardButton::MODIFIERS_NONE,      "CTR",   "",   "",  "" },

		                                     /* Row 7 - Column 1 */
		/* LSHIFT   */ { NSCODE_LShift,       7, 4, keyboardButton::MODIFIERS_ONE_CLICK, "Shift", "",   "",  "" },
		/* GRPH     */ { NSCODE_Alt,          7, 5, keyboardButton::MODIFIERS_NONE,      "GRPH",  "",   "",  "" },
		/* 1        */ { NSCODE_1,            7, 6, keyboardButton::MODIFIERS_NONE,      "1",     "!",  "ア", "ァ" },
		/* 2        */ { NSCODE_2,            7, 7, keyboardButton::MODIFIERS_NONE,      "2",     "\"", "イ", "ィ" },

		                                     /* Row 8 - Column 0 */
		/* CLR HOME */ { NSCODE_Delete,       8, 0, keyboardButton::MODIFIERS_NONE,      "CLR",   "",   "",  "" },
		/* UP       */ { NSCODE_Up,           8, 1, keyboardButton::MODIFIERS_NONE,      "↑",     "",   "",  "" },
		/* RIGHT    */ { NSCODE_Right,        8, 2, keyboardButton::MODIFIERS_NONE,      "→",     "",   "",  "" },
		/* LEFT     */ { NSCODE_Left,         8, 3, keyboardButton::MODIFIERS_NONE,      "←",     "",   "",  "" },

		                                     /* Row 8 - Column 1 */
		/* DOWN     */ { NSCODE_Down,         8, 4, keyboardButton::MODIFIERS_NONE,      "↓",     "",   "",  "" },
		/* SPACE    */ { NSCODE_Space,        8, 5, keyboardButton::MODIFIERS_NONE,      "Space", "",   "",  "" },
		/* DEL      */ { NSCODE_Backspace,    8, 6, keyboardButton::MODIFIERS_NONE,      "Del",   "",   "",  "" },
		/* INS      */ { NSCODE_Insert,       8, 7, keyboardButton::MODIFIERS_NONE,      "Ins",   "",   "",  "" },
	};
	nes_keyboard.totals = LENGTH(keycodes);

	dlgkeyb->set_keycodes(keycodes, nes_keyboard.totals);
}
void familyBasicKeyboard::set_charset(void) {
	_character charset[] {
		                      // Row 0 - Column 0
		{ QList<QString> { "\n" }, { calc_key(0, 1), -1, -1, -1 } },
		{ QList<QString> { "[", "［" }, { calc_key(0, 2), -1, -1, -1 } },
		{ QList<QString> { "ロ" }, { calc_key(0, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "「" }, { calc_key(0, 2), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "]", "］"}, { calc_key(0, 3), -1, -1, -1 } },
		{ QList<QString> { "。" }, { calc_key(0, 3), calc_kana(), -1, -1 } },
		{ QList<QString> { "」" }, { calc_key(0, 3), calc_kana(), calc_shift(), -1 } },

		                      // Row 0 - Column 1
		{ QList<QString> { "¥", "￥" }, { calc_key(0, 6), -1, -1, -1 } },
		{ QList<QString> { "ル" }, { calc_key(0, 6), calc_kana(), -1, -1 } },

		                      // Row 1 - Column 0
		{ QList<QString> { "@", "＠" }, { calc_key(1, 1), -1, -1, -1 } },
		{ QList<QString> { "レ" }, { calc_key(1, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { ":", "：" }, { calc_key(1, 2), -1, -1, -1 } },
		{ QList<QString> { "*", "＊" }, { calc_key(1, 2), calc_shift(), -1, -1 } },
		{ QList<QString> { "ー" }, { calc_key(1, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { ";", "；" }, { calc_key(1, 3), -1, -1, -1 } },
		{ QList<QString> { "+", "＋" }, { calc_key(1, 3), calc_shift(), -1, -1 } },
		{ QList<QString> { "モ" }, { calc_key(1, 3), calc_kana(), -1, -1 } },

		                      // Row 1 - Column 1
		{ QList<QString> { "_", "␣" }, { calc_key(1, 4), calc_shift(), -1, -1 } },
		{ QList<QString> { "ン" }, { calc_key(1, 4), calc_kana(), -1, -1 } },
		{ QList<QString> {"/", "／" }, { calc_key(1, 5), -1, -1, -1 } },
		{ QList<QString> { "?", "？" }, { calc_key(1, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "ヲ" }, { calc_key(1, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "-", "－" }, { calc_key(1, 6), -1, -1, -1 } },
		{ QList<QString> { "=", "＝" }, { calc_key(1, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "ラ" }, { calc_key(1, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "^", "＾" }, { calc_key(1, 7), -1, -1, -1 } },
		{ QList<QString> { "リ" }, { calc_key(1, 7), calc_kana(), -1, -1 } },

		                      // Row 2 - Column 0
		{ QList<QString> { "O", "Ｏ" }, { calc_key(2, 1), -1, -1, -1 } },
		{ QList<QString> { "ヘ" }, { calc_key(2, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "ベ" }, { calc_key(2, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "ペ" }, { calc_key(2, 1), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "L", "Ｌ" }, { calc_key(2, 2), -1, -1, -1 } },
		{ QList<QString> { "メ" }, { calc_key(2, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "K", "Ｋ" }, { calc_key(2, 3), -1, -1, -1 } },
		{ QList<QString> { "ム" }, { calc_key(2, 3), calc_kana(), -1, -1 } },

		                      // Row 2 - Column 1
		{ QList<QString> { ".", "．" }, { calc_key(2, 4), -1, -1, -1 } },
		{ QList<QString> { ">", "〉" }, { calc_key(2, 4), calc_shift(), -1, -1 } },
		{ QList<QString> { "ワ" }, { calc_key(2, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { ",", "，" }, { calc_key(2, 5), -1, -1, -1 } },
		{ QList<QString> { "<", "〈" }, { calc_key(2, 5), calc_shift(), -1, -1 } },
		{ QList<QString> { "ヨ" }, { calc_key(2, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "ョ" }, { calc_key(2, 5), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "P", "Ｐ" }, { calc_key(2, 6), -1, -1, -1 } },
		{ QList<QString> { "ホ" }, { calc_key(2, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "ボ" }, { calc_key(2, 6), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "ポ" }, { calc_key(2, 6), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "0", "０" }, { calc_key(2, 7), -1, -1, -1 } },
		{ QList<QString> { "ノ" }, { calc_key(2, 7), calc_kana(), -1, -1 } },

		                      // Row 3 - Column 0
		{ QList<QString> { "I", "Ｉ" }, { calc_key(3, 1), -1, -1, -1 } },
		{ QList<QString> { "フ" }, { calc_key(3, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "ブ" }, { calc_key(3, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "プ" }, { calc_key(3, 1), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "U", "Ｕ" }, { calc_key(3, 2), -1, -1, -1 } },
		{ QList<QString> { "ヒ" }, { calc_key(3, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ビ" }, { calc_key(3, 2), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "ピ" }, { calc_key(3, 2), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "J", "Ｊ" }, { calc_key(3, 3), -1, -1, -1 } },
		{ QList<QString> { "ミ" }, { calc_key(3, 3), calc_kana(), -1, -1 } },

		                      // Row 3 - Column 1
		{ QList<QString> { "M", "Ｍ" }, { calc_key(3, 4), -1, -1, -1 } },
		{ QList<QString> { "ユ" }, { calc_key(3, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { "ュ" }, { calc_key(3, 4), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "N", "Ｎ" }, { calc_key(3, 5), -1, -1, -1 } },
		{ QList<QString> { "ヤ" }, { calc_key(3, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "ャ" }, { calc_key(3, 5), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "9", "９" }, { calc_key(3, 6), -1, -1, -1 } },
		{ QList<QString> { ")", "）" }, { calc_key(3, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "ネ" }, { calc_key(3, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "8", "８" }, { calc_key(3, 7), -1, -1, -1 } },
		{ QList<QString> { "(", "（" }, { calc_key(3, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "ヌ" }, { calc_key(3, 7), calc_kana(), -1, -1 } },

		                      // Row 4 - Column 0
		{ QList<QString> { "Y", "Ｙ" }, { calc_key(4, 1), -1, -1, -1 } },
		{ QList<QString> { "ハ" }, { calc_key(4, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "バ" }, { calc_key(4, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "パ" }, { calc_key(4, 1), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "G", "Ｇ" }, { calc_key(4, 2), -1, -1, -1 } },
		{ QList<QString> { "ソ" }, { calc_key(4, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゾ" }, { calc_key(4, 2), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "H", "Ｈ" }, { calc_key(4, 3), -1, -1, -1 } },
		{ QList<QString> { "マ" }, { calc_key(4, 3), calc_kana(), -1, -1 } },

		                      // Row 4 - Column 1
		{ QList<QString> { "B", "Ｂ" }, { calc_key(4, 4), -1, -1, -1 } },
		{ QList<QString> { "ト" }, { calc_key(4, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { "ド" }, { calc_key(4, 4), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "V", "Ｖ" }, { calc_key(4, 5), -1, -1, -1 } },
		{ QList<QString> { "テ" }, { calc_key(4, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "デ" }, { calc_key(4, 5), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "7", "７" }, { calc_key(4, 6), -1, -1, -1 } },
		{ QList<QString> { "'",  "＇" }, { calc_key(4, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "ニ" }, { calc_key(4, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "6", "６" }, { calc_key(4, 7), -1, -1, -1 } },
		{ QList<QString> { "&", "＆" }, { calc_key(4, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "ナ" }, { calc_key(4, 7), calc_kana(), -1, -1 } },

		                      // Row 5 - Column 0
		{ QList<QString> { "T", "Ｔ" }, { calc_key(5, 1), -1, -1, -1 } },
		{ QList<QString> { "コ" }, { calc_key(5, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゴ" }, { calc_key(5, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "R", "Ｒ" }, { calc_key(5, 2), -1, -1, -1 } },
		{ QList<QString> { "ケ" }, { calc_key(5, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゲ" }, { calc_key(5, 2), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "D", "Ｄ" }, { calc_key(5, 3), -1, -1, -1 } },
		{ QList<QString> { "ス" }, { calc_key(5, 3), calc_kana(), -1, -1 } },
		{ QList<QString> { "ズ" }, { calc_key(5, 3), calc_kana(), calc_grph(), -1 } },

		                      // Row 5 - Column 1
		{ QList<QString> { "F", "Ｆ" }, { calc_key(5, 4), -1, -1, -1 } },
		{ QList<QString> { "セ" }, { calc_key(5, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゼ" }, { calc_key(5, 4), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "C", "Ｃ" }, { calc_key(5, 5), -1, -1, -1 } },
		{ QList<QString> { "ツ" }, { calc_key(5, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "ヅ" }, { calc_key(5, 5), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "ッ" }, { calc_key(5, 5), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "5", "５" }, { calc_key(5, 6), -1, -1, -1 } },
		{ QList<QString> { "%", "％" }, { calc_key(5, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "オ" }, { calc_key(5, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "ォ" }, { calc_key(5, 6), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "4", "４" }, { calc_key(5, 7), -1, -1, -1 } },
		{ QList<QString> { "$", "＄" }, { calc_key(5, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "エ" }, { calc_key(5, 7), calc_kana(), -1, -1 } },
		{ QList<QString> { "ェ" }, { calc_key(5, 7), calc_kana(), calc_shift(), -1 } },

		                      // Row 6 - Column 0
		{ QList<QString> { "W", "Ｗ" }, { calc_key(6, 1), -1, -1, -1 } },
		{ QList<QString> { "キ" }, { calc_key(6, 1), calc_kana(), -1, -1 } },
		{ QList<QString> { "ギ" }, { calc_key(6, 1), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "S", "Ｓ" }, { calc_key(6, 2), -1, -1, -1 } },
		{ QList<QString> { "シ" }, { calc_key(6, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ジ" }, { calc_key(6, 2), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "A", "Ａ" }, { calc_key(6, 3), -1, -1, -1 } },
		{ QList<QString> { "サ" }, { calc_key(6, 3), calc_kana(), -1, -1 } },
		{ QList<QString> { "ザ" }, { calc_key(6, 3), calc_kana(), calc_grph(), -1 } },

		                      // Row 6 - Column 1
		{ QList<QString> { "X", "Ｘ" }, { calc_key(6, 4), -1, -1, -1 } },
		{ QList<QString> { "チ" }, { calc_key(6, 4), calc_kana(), -1, -1 } },
		{ QList<QString> { "ヂ" }, { calc_key(6, 4), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "Z", "Ｚ" }, { calc_key(6, 5), -1, -1, -1 } },
		{ QList<QString> { "タ" }, { calc_key(6, 5), calc_kana(), -1, -1 } },
		{ QList<QString> { "ダ" }, { calc_key(6, 5), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "E", "Ｅ" }, { calc_key(6, 6), -1, -1, -1 } },
		{ QList<QString> { "ク" }, { calc_key(6, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "グ" }, { calc_key(6, 6), calc_kana(), calc_grph(), -1 } },
		{ QList<QString> { "3", "３" }, { calc_key(6, 7), -1, -1, -1 } },
		{ QList<QString> { "#", "＃" }, { calc_key(6, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "ウ" }, { calc_key(6, 7), calc_kana(), -1, -1 } },
		{ QList<QString> { "ゥ" }, { calc_key(6, 7), calc_kana(), calc_shift(), -1 } },

		                      // Row 7 - Column 0
		{ QList<QString> { "Q", "Ｑ" }, { calc_key(7, 2), -1, -1, -1 } },
		{ QList<QString> { "カ" }, { calc_key(7, 2), calc_kana(), -1, -1 } },
		{ QList<QString> { "ガ" }, { calc_key(7, 2), calc_kana(), calc_grph(), -1 } },

		                      // Row 7 - Column 1
		{ QList<QString> { "1", "１" }, { calc_key(7, 6), -1, -1, -1 } },
		{ QList<QString> { "!", "！" }, { calc_key(7, 6), calc_shift(), -1, -1 } },
		{ QList<QString> { "ア" }, { calc_key(7, 6), calc_kana(), -1, -1 } },
		{ QList<QString> { "ァ" }, { calc_key(7, 6), calc_kana(), calc_shift(), -1 } },
		{ QList<QString> { "2", "２" }, { calc_key(7, 7), -1, -1, -1 } },
		{ QList<QString> { "\"", "＂" }, { calc_key(7, 7), calc_shift(), -1, -1 } },
		{ QList<QString> { "イ" }, { calc_key(7, 7), calc_kana(), -1, -1 } },
		{ QList<QString> { "ィ" }, { calc_key(7, 7), calc_kana(), calc_shift(), -1 } },

		                      // Row 8 - Column 0
		                      // Row 8 - Column 1
		{ QList<QString> { " ", "　" }, { calc_key(8, 5), -1, -1, -1 } },
	};

	dlgkeyb->set_charset({ &charset[0], LENGTH(charset) }, delay);
}
QList<QList<SBYTE>> familyBasicKeyboard::parse_text(keyboardObject::_character *ch) {
	QList<QList<SBYTE>> keys;
	QList<SBYTE> key;
	BYTE kana_found = FALSE;
	int i;

	for (i = 0; i < 4; i++) {
		if (ch->keys[i] > 0) {
			if (ch->keys[i] == calc_kana()) {
				kana_found = TRUE;
				continue;
			}
			key.append(ch->keys[i]);
		}
	}
	if (kana_found) {
		QList<SBYTE> kana;

		// CTR + V abilita il kana
		kana.append(calc_ctr());
		kana.append(calc_v());
		keys.append(kana);

		keys.append(key);

		// CTR + W lo disabilita
		kana.clear();
		kana.append(calc_ctr());
		kana.append(calc_w());
		keys.append(kana);
	} else {
		keys.append(key);
	}
	return (keys);
}

SBYTE familyBasicKeyboard::calc_key(BYTE row, BYTE column) {
	return ((row * columns) + column);
}
SBYTE familyBasicKeyboard::calc_kana(void) {
	return (calc_key(0, 4));
}
SBYTE familyBasicKeyboard::calc_shift(void) {
	return (calc_key(0, 5));
}
SBYTE familyBasicKeyboard::calc_ctr(void) {
	return (calc_key(7, 3));
}
SBYTE familyBasicKeyboard::calc_grph(void) {
	return (calc_key(7, 5));
}
SBYTE familyBasicKeyboard::calc_w(void) {
	return (calc_key(6, 1));
}
SBYTE familyBasicKeyboard::calc_v(void) {
	return (calc_key(4, 5));
}
