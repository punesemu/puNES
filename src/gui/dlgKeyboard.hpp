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
		enum label_position {
			LP_CENTER,
			LP_LEFT_TOP,
			LP_LEFT,
			LP_LEFT_BOTTOM,
			LP_BOTTOM,
			LP_RIGHT_BOTTOM,
			LP_RIGHT,
			LP_RIGHT_TOP,
			LP_TOP,
		};
		enum misc {
			MIN_W = 45,
			MIN_H = MIN_W
		};
		typedef struct _label {
			label_position position;
			QString label;
			bool bold;
		} _label;
		typedef struct _color {
			_color() : bck("#EFEFEF"), hover("#DDE0DA"), press("#BABDB6"),
				bck_border("#D91414"), hover_border("#E36666"), press_border("#D91414") {}
			QString bck;
			QString hover;
			QString press;
			QString bck_border;
			QString hover_border;
			QString press_border;
		} _color;
		typedef struct _modifier {
			modifier_types type;
			BYTE state;
		} _modifier;

	public:
		QList<_label> labels;
		DBWORD nscode;
		SWORD index;
		SBYTE row;
		SBYTE column;
		SWORD element;
		BYTE pressed;
		_modifier modifier;
		int minw;
		int minh;
		double size_factor;

	public:
		keyboardButton(QWidget *parent);
		~keyboardButton();

	protected:
		void paintEvent(QPaintEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);

	public:
		void setMinimumSize(const QSize &s);

	public:
		void set(DBWORD nscode, SWORD index, SBYTE row, SBYTE column, SWORD element, modifier_types mtype,
			_color clr, QList<_label> labels);
		void reset(void);
};

// wdgKeyboard -------------------------------------------------------------------------------------------------------------------

class wdgKeyboard : public QWidget {
	Q_OBJECT

	public:
		typedef struct _character {
			QList<QString> string;
			SBYTE elements[4];
		} _character;
		typedef struct _charset {
			_character *set;
			BYTE length;
		} _charset;
		typedef struct _button {
			QString object_name;
			BYTE row;
			BYTE column;
			keyboardButton::modifier_types modifier;
			keyboardButton::_color clr;
			QList<keyboardButton::_label> labels;
		} _button;
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
		wdgKeyboard(QWidget *parent = 0);
		~wdgKeyboard();

	protected:
		void init(void);
		virtual void set_buttons(void);
		virtual void set_charset(void);

	public:
		virtual void ext_setup(void);
		virtual SBYTE calc_element(BYTE row, BYTE column);
		virtual QList<QList<SBYTE>> parse_character(_character *ch);
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
		Qt::CaseSensitivity cs;

	private:
		BYTE type;
		QString string;
		int string_index;
		int characters_processed;
		BYTE break_process;
		QList<wdgKeyboard::_character> charset;
		wdgKeyboard::_character *character;
		wdgKeyboard::_delay delay;
		QList<QList<SBYTE>> elements;
		int element_index;

	public:
		pasteObject(QObject *parent = 0);
		~pasteObject();

	public:
		void reset(void);
		void set_charset(wdgKeyboard::_charset charset, wdgKeyboard::_delay delay);
		void set_text(QString text);
		void parse_delay();
		void parse_text(void);
		void parse_break(void);

	private:
		void parse_reset(void);
		void set_elements(BYTE value);
};

// familyBasicKeyboard -----------------------------------------------------------------------------------------------------------

#include "wdgKeyboardFB.hh"

class familyBasicKeyboard : public wdgKeyboard, public Ui::wdgKeyboardFB {
	Q_OBJECT

	public:
		familyBasicKeyboard(QWidget *parent = 0);
		~familyBasicKeyboard();

	protected:
		void set_buttons(void);
		void set_charset(void);

	public:
		void ext_setup(void);
		QList<QList<SBYTE>> parse_character(wdgKeyboard::_character *ch);

	private:
		keyboardButton::_color red_button(void);
		SBYTE calc_kana(void);
		SBYTE calc_shift(void);
		SBYTE calc_ctr(void);
		SBYTE calc_grph(void);
		SBYTE calc_w(void);
		SBYTE calc_v(void);
};

// suborKeyboard -----------------------------------------------------------------------------------------------------------------

#include "wdgKeyboardSubor.hh"

class suborKeyboard : public wdgKeyboard, public Ui::wdgKeyboardSubor {
	Q_OBJECT

	public:
		suborKeyboard(QWidget *parent = 0);
		~suborKeyboard();

	protected:
		void set_buttons(void);
		void set_charset(void);

	public:
		void ext_setup(void);

	private:
		SBYTE calc_shift(void);
};

// dlgCfgNSCode ------------------------------------------------------------------------------------------------------------------

#include "dlgCfgNSCode.hh"

class dlgCfgNSCode : public QDialog, public Ui::dlgCfgNSCode {
	Q_OBJECT

	private:
		keyboardButton *button;
		DBWORD nscode;

	public:
		dlgCfgNSCode(QWidget *parent = 0, keyboardButton *button = 0);
		~dlgCfgNSCode();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	private:
		bool keypress(QKeyEvent *event);

	private slots:
		void s_default_clicked(bool checked);
		void s_unset_clicked(bool checked);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);
};

// dlgKeyboard -------------------------------------------------------------------------------------------------------------------

#include "dlgKeyboard.hh"

class dlgKeyboard : public QDialog, public Ui::dlgKeyboard {
	Q_OBJECT

	public:
		enum keyevent_types {
			KEVENT_NORMAL,
			KEVENT_VIRTUAL,
		};
		enum dk_mode {
			DK_VIRTUAL,
			DK_SETUP
		};

	private:
		typedef struct _one_click {
			QList<keyboardButton *> list;
			BYTE activies;
		} _one_click;
		typedef struct _last_press {
			DBWORD nscode;
			double time;
		} _last_press;

	public:
		BYTE mode;
		QRect geom;
		pasteObject *paste;

	private:
		keyboardButton *kbuttons[NES_KEYBOARD_MAX_KEYS];
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
		void set_buttons(wdgKeyboard *wk, wdgKeyboard::_button buttons[], int totals);
		void set_charset(wdgKeyboard::_charset charset, wdgKeyboard::_delay delay);
		bool process_event(QEvent *event);
		void shortcut_toggle(BYTE mode);
		void button_press(keyboardButton *kb, keyevent_types type);
		void key_event_press(QKeyEvent *event, keyevent_types type);
		void button_release(keyboardButton *kb, keyevent_types type);
		void key_event_release(QKeyEvent *event, keyevent_types type);
		void switch_mode(BYTE mode);

	public:
		void fake_keyboard(void);
		void family_basic_keyboard(void);
		void subor_keyboard(void);

	private:
		void replace_keyboard(wdgKeyboard *wk);
		void set_size_factor(double size_factor);
		bool one_click_find(keyboardButton *kb);
		void one_click_append(keyboardButton *kb);
		void one_click_remove(keyboardButton *kb);
		void one_click_oneshot(keyboardButton *kb);
		void one_click_inc(void);
		void one_click_dec(void);

	private slots:
		void s_mode(int index);
		void s_size_factor(int index);
};

#endif /* DLGKEYBOARD_HPP_ */
