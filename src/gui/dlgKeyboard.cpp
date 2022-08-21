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

		if (nes_keyboard.enabled) {
			switch (nes_keyboard.type) {
				default:
					disable = TRUE;
					break;
				case CTRL_FAMILY_BASIC_KEYBOARD:
					dlgkeyb->familyBasicKeyboard();
					break;
			}
		}
		if (disable) {
			if (!dlgkeyb->isHidden()) {
				dlgkeyb->close();
			}
			dlgkeyb->reset();
		}
		mainwin->action_Virtual_Keyboard->setEnabled(!disable);
		mainwin->statusbar->keyb->setEnabled(!disable);
		mainwin->update_window();
	}
}

// dlgKeyboard -------------------------------------------------------------------------------------------------------------------

dlgKeyboard::dlgKeyboard(QWidget *parent) : QDialog(parent) {
	QFont f8;

	f8.setPointSize(8);
	f8.setWeight(QFont::Light);
	setFont(f8);

	setupUi(this);

	geom.setX(cfg->lg_nes_keyboard.x);
	geom.setY(cfg->lg_nes_keyboard.y);
	geom.setWidth(cfg->lg_nes_keyboard.w);
	geom.setHeight(cfg->lg_nes_keyboard.h);

	{
		QList<keyboardButton *>kb_list = findChildren<keyboardButton *>();

		foreach (keyboardButton *kb, kb_list) {
			if (kb->minimumWidth() == 46) {
				kb->setMinimumWidth(keyboardButton::MIN_W);
			}
			if (kb->minimumHeight() == 46) {
				kb->setMinimumHeight(keyboardButton::MIN_H);
			}
		}
	}

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
bool dlgKeyboard::process_event(QEvent *event) {
	if ((tas.type == NOTAS) && nes_keyboard.enabled && !dlgsettings->isActiveWindow()) {
		if (event->type() == QEvent::ShortcutOverride) {
			key_event_press((QKeyEvent*)event, dlgKeyboard::KEVENT_NORMAL);
			return (true);
		} else if (event->type() == QEvent::KeyRelease) {
			key_event_release((QKeyEvent*)event, dlgKeyboard::KEVENT_NORMAL);
			return (true);
		} else if (event->type() == QEvent::Shortcut) {
			QKeySequence key = ((QShortcutEvent*)event)->key();

			if (gui.capture_input &&
				(key != mainwin->shortcut[SET_INP_SC_TOGGLE_CAPTURE_INPUT]->key()) &&
				(key != mainwin->shortcut[SET_INP_SC_TOGGLE_NES_KEYBOARD]->key())) {
				return (true);
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

void dlgKeyboard::reset(void) {
	QList<keyboardButton *>kb_list = findChildren<keyboardButton *>();

	foreach (keyboardButton *kb, kb_list) {
		kb->reset();
	}
	memset(keys, 0x00, sizeof(keys));
	one_click.activies = FALSE;
	one_click.list.clear();
	last_press.code = 0;
	last_press.time = gui_get_ms();

	retranslateUi(this);
}
void dlgKeyboard::familyBasicKeyboard(void) {
	// Matrix :
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

	_dlgKeyboard_keycode family_basic_keyboard[] = {
		// Row 0 - Column 0
		{ NSCODE_F8,           0, 0, keyboardButton::MODIFIERS_NONE,      "F8",    ""  }, // F8
		{ NSCODE_Return,       0, 1, keyboardButton::MODIFIERS_NONE,      "⏎",     ""  }, // RETURN
		{ NSCODE_Enter,        0, 1, keyboardButton::MODIFIERS_NONE,      "⏎",     ""  }, // RETURN
		{ NSCODE_BracketRight, 0, 2, keyboardButton::MODIFIERS_NONE,      "[",     ""  }, // [
		{ NSCODE_NumberSign,   0, 3, keyboardButton::MODIFIERS_NONE,      "]",     ""  }, // ]
		// Row 0 - Column 1
		{ NSCODE_CapsLock,     0, 4, keyboardButton::MODIFIERS_SWITCH,    "Kana",  ""  }, // KANA
		{ NSCODE_RShift,       0, 5, keyboardButton::MODIFIERS_ONE_CLICK, "Shift", ""  }, // RSHIFT
		{ NSCODE_Home,         0, 6, keyboardButton::MODIFIERS_NONE,      "¥",     ""  }, // ¥
		{ NSCODE_End,          0, 7, keyboardButton::MODIFIERS_NONE,      "Stop",  ""  }, // STOP
		// Row 1 - Column 0
		{ NSCODE_F7,           1, 0, keyboardButton::MODIFIERS_NONE,      "F7",    ""  }, // F7
		{ NSCODE_BracketLeft,  1, 1, keyboardButton::MODIFIERS_NONE,      "@",     ""  }, // @
		{ NSCODE_Apostrophe,   1, 2, keyboardButton::MODIFIERS_NONE,      ":",     "*" }, // :
		{ NSCODE_Semicolon,    1, 3, keyboardButton::MODIFIERS_NONE,      ";",     "+" }, // ;
		// Row 1 - Column 1
		{ NSCODE_QuoteLeft,    1, 4, keyboardButton::MODIFIERS_NONE,      "_",     ""  }, // _
		{ NSCODE_Slash,        1, 5, keyboardButton::MODIFIERS_NONE,      "/",     "?" }, // /
		{ NSCODE_Minus,        1, 6, keyboardButton::MODIFIERS_NONE,      "-",     "=" }, // -
		{ NSCODE_Equal,        1, 7, keyboardButton::MODIFIERS_NONE,      "^",     ""  }, // ^
		// Row 2 - Column 0
		{ NSCODE_F6,           2, 0, keyboardButton::MODIFIERS_NONE,      "F6",     "" }, // F6
		{ NSCODE_O,            2, 1, keyboardButton::MODIFIERS_NONE,      "O",      "" }, // O
		{ NSCODE_L,            2, 2, keyboardButton::MODIFIERS_NONE,      "L",      "" }, // L
		{ NSCODE_K,            2, 3, keyboardButton::MODIFIERS_NONE,      "K",      "" }, // K
		// Row 2 - Column 1
		{ NSCODE_Period,       2, 4, keyboardButton::MODIFIERS_NONE,      ".",     ">" }, // .
		{ NSCODE_Comma,        2, 5, keyboardButton::MODIFIERS_NONE,      ",",     "<" }, // ,
		{ NSCODE_P,            2, 6, keyboardButton::MODIFIERS_NONE,      "P",      "" }, // P
		{ NSCODE_0,            2, 7, keyboardButton::MODIFIERS_NONE,      "0",      "" }, // 0
		// Row 3 - Column 0
		{ NSCODE_F5,           3, 0, keyboardButton::MODIFIERS_NONE,      "F5",     "" }, // F5
		{ NSCODE_I,            3, 1, keyboardButton::MODIFIERS_NONE,      "I",      "" }, // I
		{ NSCODE_U,            3, 2, keyboardButton::MODIFIERS_NONE,      "U",      "" }, // U
		{ NSCODE_J,            3, 3, keyboardButton::MODIFIERS_NONE,      "J",      "" }, // J
		// Row 3 - Column 1
		{ NSCODE_M,            3, 4, keyboardButton::MODIFIERS_NONE,      "M",      "" }, // M
		{ NSCODE_N,            3, 5, keyboardButton::MODIFIERS_NONE,      "N",      "" }, // N
		{ NSCODE_9,            3, 6, keyboardButton::MODIFIERS_NONE,      "9",     ")" }, // 9
		{ NSCODE_8,            3, 7, keyboardButton::MODIFIERS_NONE,      "8",     "(" }, // 8
		// Row 4 - Column 0
		{ NSCODE_F4,           4, 0, keyboardButton::MODIFIERS_NONE,      "F4",     "" }, // F4
		{ NSCODE_Y,            4, 1, keyboardButton::MODIFIERS_NONE,      "Y",      "" }, // Y
		{ NSCODE_G,            4, 2, keyboardButton::MODIFIERS_NONE,      "G",      "" }, // G
		{ NSCODE_H,            4, 3, keyboardButton::MODIFIERS_NONE,      "H",      "" }, // H
		// Row 4 - Column 1
		{ NSCODE_B,            4, 4, keyboardButton::MODIFIERS_NONE,      "B",      "" }, // B
		{ NSCODE_V,            4, 5, keyboardButton::MODIFIERS_NONE,      "V",      "" }, // V
		{ NSCODE_7,            4, 6, keyboardButton::MODIFIERS_NONE,      "7",     "'" }, // 7
		{ NSCODE_6,            4, 7, keyboardButton::MODIFIERS_NONE,      "6",     "&" }, // 6
		// Row 5 - Column 0
		{ NSCODE_F3,           5, 0, keyboardButton::MODIFIERS_NONE,      "F3",     "" }, // F3
		{ NSCODE_T,            5, 1, keyboardButton::MODIFIERS_NONE,      "T",      "" }, // T
		{ NSCODE_R,            5, 2, keyboardButton::MODIFIERS_NONE,      "R",      "" }, // R
		{ NSCODE_D,            5, 3, keyboardButton::MODIFIERS_NONE,      "D",      "" }, // D
		// Row 5 - Column 1
		{ NSCODE_F,            5, 4, keyboardButton::MODIFIERS_NONE,      "F",      "" }, // F
		{ NSCODE_C,            5, 5, keyboardButton::MODIFIERS_NONE,      "C",      "" }, // C
		{ NSCODE_5,            5, 6, keyboardButton::MODIFIERS_NONE,      "5",     "%" }, // 5
		{ NSCODE_4,            5, 7, keyboardButton::MODIFIERS_NONE,      "4",     "$" }, // 4
		// Row 6 - Column 0
		{ NSCODE_F2,           6, 0, keyboardButton::MODIFIERS_NONE,      "F2",     "" }, // F2
		{ NSCODE_W,            6, 1, keyboardButton::MODIFIERS_NONE,      "W",      "" }, // W
		{ NSCODE_S,            6, 2, keyboardButton::MODIFIERS_NONE,      "S",      "" }, // S
		{ NSCODE_A,            6, 3, keyboardButton::MODIFIERS_NONE,      "A",      "" }, // A
		// Row 6 - Column 1
		{ NSCODE_X,            6, 4, keyboardButton::MODIFIERS_NONE,      "X",      "" }, // X
		{ NSCODE_Z,            6, 5, keyboardButton::MODIFIERS_NONE,      "Z",      "" }, // Z
		{ NSCODE_E,            6, 6, keyboardButton::MODIFIERS_NONE,      "E",      "" }, // E
		{ NSCODE_3,            6, 7, keyboardButton::MODIFIERS_NONE,      "3",     "#" }, // 3
		// Row 7 - Column 0
		{ NSCODE_F1,           7, 0, keyboardButton::MODIFIERS_NONE,      "F1",     "" }, // F1
		{ NSCODE_Escape,       7, 1, keyboardButton::MODIFIERS_NONE,      "Esc",    "" }, // ESC
		{ NSCODE_Q,            7, 2, keyboardButton::MODIFIERS_NONE,      "Q",      "" }, // Q
		{ NSCODE_LControl,     7, 3, keyboardButton::MODIFIERS_NONE,      "CTR",    "" }, // CTR
		{ NSCODE_RControl,     7, 3, keyboardButton::MODIFIERS_NONE,      "CTR",    "" }, // CTR
		// Row 7 - Column 1
		{ NSCODE_LShift,       7, 4, keyboardButton::MODIFIERS_ONE_CLICK, "Shift",  "" }, // LSHIFT
		{ NSCODE_Alt,          7, 5, keyboardButton::MODIFIERS_NONE,      "GRPH",   "" }, // GRPH
		{ NSCODE_1,            7, 6, keyboardButton::MODIFIERS_NONE,      "1",     "!" }, // 1
		{ NSCODE_2,            7, 7, keyboardButton::MODIFIERS_NONE,      "2",    "\"" }, // 2
		// Row 8 - Column 0
		{ NSCODE_Delete,       8, 0, keyboardButton::MODIFIERS_NONE,      "CLR",    "" }, // CLR HOME
		{ NSCODE_Up,           8, 1, keyboardButton::MODIFIERS_NONE,      "↑",      "" }, // UP
		{ NSCODE_Right,        8, 2, keyboardButton::MODIFIERS_NONE,      "→",      "" }, // RIGHT
		{ NSCODE_Left,         8, 3, keyboardButton::MODIFIERS_NONE,      "←",      "" }, // LEFT
		// Row 8 - Column 1
		{ NSCODE_Down,         8, 4, keyboardButton::MODIFIERS_NONE,      "↓",      "" }, // DOWN
		{ NSCODE_Space,        8, 5, keyboardButton::MODIFIERS_NONE,      "Space",  "" }, // SPACE
		{ NSCODE_Backspace,    8, 6, keyboardButton::MODIFIERS_NONE,      "Del",    "" }, // DEL
		{ NSCODE_Insert,       8, 7, keyboardButton::MODIFIERS_NONE,      "Ins",    "" }, // INS
	};
	nes_keyboard.enabled = TRUE;
	nes_keyboard.rows = 9;
	nes_keyboard.columns = 8;
	nes_keyboard.totals = sizeof(family_basic_keyboard) / sizeof(_dlgKeyboard_keycode);

	init(family_basic_keyboard, nes_keyboard.totals);
}

void dlgKeyboard::init(_dlgKeyboard_keycode kcodes[], int totals) {
	QList<keyboardButton *>kb_list= findChildren<keyboardButton *>();
	int i;

	reset();

	memset(nes_keyboard.keys, 0x00, sizeof(nes_keyboard.keys));
	memset(nes_keyboard.keycodes, 0x00, sizeof(nes_keyboard.keycodes));

	for (i = 0; i < totals; i++) {
		_dlgKeyboard_keycode *kc = &kcodes[i];

		nes_keyboard.keycodes[i].code = kc->code;
		nes_keyboard.keycodes[i].row = kc->row;
		nes_keyboard.keycodes[i].column = kc->column;
		nes_keyboard.keycodes[i].index = i;

		foreach (keyboardButton *kb, kb_list) {
			if (kc->code == kb->scancode) {
				kb->set(i, (kc->row * nes_keyboard.columns) + kc->column, kc->modifier, kc->line1, kc->line2);
				keys[kb->index] = kb;
				break;
			}
		}
	}

	retranslateUi(this);
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

		painter.save();

		// disegno il testo interno
		{
			x = 0;
			y = 0;
			w = (qreal)rect().width();
			h = (qreal)rect().height();

			painter.setFont(dlgkbd->font());

			if (!line1.isEmpty()) {
				if (!line2.isEmpty()) {
					h = (qreal)rect().height() / 2.0f;
					y = h;
				}
				painter.drawText(QRectF(x, y, w, h), Qt::AlignCenter, line1);
			}
			if (!line2.isEmpty()) {
				y = 0;
				h = (qreal)rect().height() / 2.0f;
				painter.drawText(QRectF(x, y, w, h), Qt::AlignCenter, line2);
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

			painter.setRenderHint(QPainter::HighQualityAntialiasing);
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

void keyboardButton::set(SWORD index, SWORD key, modifier_types mtype, QString line1, QString line2) {
	this->index = index;
	this->key = key;
	modifier.type = mtype;
	modifier.state = 0;
	this->line1 = line1;
	this->line2 = line2;
	pressed = FALSE;
	setEnabled(TRUE);
}
void keyboardButton::reset(void) {
	index = -1;
	key = -1;
	modifier.type = MODIFIERS_NONE;
	modifier.state = 0;
	line1 = "";
	line2 = "";
	pressed = FALSE;
	setEnabled(FALSE);
}
