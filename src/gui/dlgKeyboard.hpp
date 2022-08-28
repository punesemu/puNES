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
			MIN_W = 45,
			MIN_H = MIN_W
		};
		typedef struct _modifier {
			modifier_types type;
			BYTE state;
		} _modifier;

	public:
		SWORD index;
		SWORD key;
		DBWORD scancode;
		QString left;
		QString up;
		QString down;
		QString right;
		BYTE pressed;
		_modifier modifier;

	public:
		keyboardButton(QWidget *parent);
		~keyboardButton();

	protected:
		void paintEvent(QPaintEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);

	public:
		void set(SWORD index, SWORD key, modifier_types mtype, QString left, QString up, QString down, QString right);
		void reset(void);
};

// keyboardObject -----------------------------------------------------------------------------------------------------------------

class keyboardObject : public QObject {
	Q_OBJECT

	public:
		typedef struct _character {
			QList<QString> string;
			SBYTE keys[4];
		} _character;
		typedef struct _charset {
			_character *set;
			BYTE length;
		} _charset;
		typedef struct _keycode {
			DBWORD code;
			BYTE row;
			BYTE column;
			keyboardButton::modifier_types modifier;
			QString left;
			QString up;
			QString down;
			QString right;
		} _keycode;
		typedef struct _delay {
			BYTE counter;
			BYTE set;
			BYTE unset;
		} _delay;

	public:
		int rows;
		int columns;
		_delay delay;

	public:
		keyboardObject(QObject *parent = 0);
		~keyboardObject();

	protected:
		void init(void);
		virtual void set_keycodes(void);
		virtual void set_charset();

	public:
		virtual QList<QList<SBYTE>> parse_text(keyboardObject::_character *ch);
};

// familyBasicKeyboard -----------------------------------------------------------------------------------------------------------

class familyBasicKeyboard : public keyboardObject {
	Q_OBJECT

	public:
		familyBasicKeyboard(QObject *parent = 0);
		~familyBasicKeyboard();

	protected:
		void set_keycodes(void);
		void set_charset();

	public:
		QList<QList<SBYTE>> parse_text(keyboardObject::_character *ch);

	private:
		SBYTE calc_key(BYTE row, BYTE column);
		SBYTE calc_kana(void);
		SBYTE calc_shift(void);
		SBYTE calc_ctr(void);
		SBYTE calc_grph(void);
		SBYTE calc_w(void);
		SBYTE calc_v(void);
};

// pasteObject -------------------------------------------------------------------------------------------------------------------

class pasteObject : public QObject {
	Q_OBJECT

	private:
		enum paste_modes {
			PASTE_SET,
			PAST_UNSET
		};

	public:
		BYTE enable;

	private:
		BYTE type;
		QString string;
		int string_index;
		int characters_elaborate;
		BYTE break_insert;
		QList<keyboardObject::_character> charset;
		keyboardObject::_character *ch;
		keyboardObject::_delay delay;
		QList<QList<SBYTE>> keys;
		int keys_index;

	public:
		pasteObject(QObject *parent = 0);
		~pasteObject();

	public:
		void reset(void);
		void set_charset(keyboardObject::_charset charset, keyboardObject::_delay delay);
		void set_text(QString text);
		void parse_text(void);
		void parse_break(void);

	private:
		void parse_reset(void);
		void set_keys(BYTE value);
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

	private:
		typedef struct _one_click {
			QList<keyboardButton *> list;
			BYTE activies;
		} _one_click;
		typedef struct _last_press {
			DBWORD code;
			double time;
		} _last_press;

	public:
		keyboardObject *keyboard;
		pasteObject *paste;
		QRect geom;

	private:
		keyboardButton *keys[NES_KEYBOARD_MAX_KEYS];
		_one_click one_click;
		_last_press last_press;
		QString last_line;

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
		void reset(void);
		void set_keycodes(keyboardObject::_keycode keycodes[], int totals);
		void set_charset(keyboardObject::_charset charset, keyboardObject::_delay delay);
		bool process_event(QEvent *event);
		void shortcut_toggle(BYTE mode);
		void button_press(keyboardButton *kb, key_event_types type);
		void key_event_press(QKeyEvent *keyEvent, key_event_types type);
		void button_release(keyboardButton *kb, key_event_types type);
		void key_event_release(QKeyEvent *keyEvent, key_event_types type);

	public:
		void fake_keyboard(void);
		void family_basic_keyboard(void);

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
