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

#ifndef DLGKEYBOARD_HPP_
#define DLGKEYBOARD_HPP_

#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGraphicsColorizeEffect>
#include "common.h"
#include "input.h"

// keyboardButton ----------------------------------------------------------------------------------------------------------------

class keyboardButton : public QPushButton {
	Q_OBJECT

	public:
		enum modifier_types {
			MODIFIERS_NONE,
			MODIFIERS_SWITCH,
			MODIFIERS_ONE_CLICK
		};
		enum misc {
			MIN_W = 40,
			MIN_H = MIN_W
		};
		struct _keyboardButton_modifier {
			modifier_types type;
			BYTE state;
		} modifier;
		SWORD index;
		SWORD key;
		DBWORD scancode;
		QString line1;
		QString line2;
		BYTE pressed;

	public:
		keyboardButton(QWidget *parent);
		~keyboardButton();

	protected:
		void paintEvent(QPaintEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);

	public:
		void set(SWORD index, SWORD key, modifier_types mtype, QString line1, QString line2);
		void reset(void);
};

// dlgKeyboard -------------------------------------------------------------------------------------------------------------------

#include "dlgKeyboard.hh"

class dlgKeyboard : public QDialog, public Ui::dlgKeyboard {
	Q_OBJECT

	public:
		enum key_event_types {
			KEVENT_NORMAL,
			KEVENT_VIRTUAL,
		};
		QRect geom;

	private:
		typedef struct _keycode {
			DBWORD code;
			BYTE row;
			BYTE column;
			keyboardButton::modifier_types modifier;
			QString line1;
			QString line2;
		} _dlgKeyboard_keycode;
		struct _one_click {
			QList<keyboardButton *>list;
			BYTE activies;
		} one_click;
		struct _last_press {
			DBWORD code;
			double time;
		} last_press;
		keyboardButton *keys[NES_KEYBOARD_MAX_KEYS];

	public:
		dlgKeyboard(QWidget *parent = 0);
		~dlgKeyboard();

	protected:
		bool event(QEvent *event);
		bool eventFilter(QObject *obj, QEvent *event);
		void changeEvent(QEvent *event);
		void showEvent(QShowEvent *event);
		void hideEvent(QHideEvent *event);
		void closeEvent(QCloseEvent *event);

	public:
		void retranslateUi(QDialog *dlgKeyboard);
		bool process_event(QEvent *event);
		void shortcut_toggle(BYTE mode);
		void button_press(keyboardButton *kb, key_event_types type);
		void key_event_press(QKeyEvent *keyEvent, key_event_types type);
		void button_release(keyboardButton *kb, key_event_types type);
		void key_event_release(QKeyEvent *keyEvent, key_event_types type);

	public:
		void reset(void);
		void familyBasicKeyboard(void);

	private:
		void init(_dlgKeyboard_keycode kcodes[], int totals);

	private:
		bool one_click_find(keyboardButton *kb);
		void one_click_append(keyboardButton *kb);
		void one_click_remove(keyboardButton *kb);
		void one_click_oneshot(keyboardButton *kb);
		void one_click_inc(void);
		void one_click_dec(void);

	private slots:
		void s_toggle_keypad(bool checked);
};

#endif /* DLGKEYBOARD_HPP_ */
