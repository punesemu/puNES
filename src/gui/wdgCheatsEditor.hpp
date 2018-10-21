/*
 *  Copyright (C) 2010-2019 Fabio Cavallo (aka FHorse)
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

#ifndef WDGCHEATSEDITOR_HPP_
#define WDGCHEATSEDITOR_HPP_

#include <QtCore/QXmlStreamReader>
#include <QtWidgets/QWidget>
#include <QtWidgets/QSpinBox>
#include "wdgCheatsEditor.hh"
#include "objCheat.hpp"

class hexSpinBox : public QSpinBox {
		Q_OBJECT

	private:
		int digits;
		bool no_prefix;

	public:
		hexSpinBox(QWidget *parent, int dgts);
		~hexSpinBox();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
		QValidator::State validate(QString &text, int &pos) const;
		int valueFromText(const QString &text) const;
		QString textFromValue(int value) const;

	private:
		QRegExpValidator *validator;
};

class wdgCheatsEditor : public QWidget, public Ui::wdgCheatEditor {
		Q_OBJECT

	private:
		bool new_cheat;
		objCheat *objch;
		hexSpinBox *hexSpinBox_Address;
		hexSpinBox *hexSpinBox_Value;
		hexSpinBox *hexSpinBox_Compare;

	public:
		wdgCheatsEditor(QWidget *parent = 0);
		~wdgCheatsEditor();

	protected:
		void changeEvent(QEvent *event);

	public:
		void hide_tools_widgets(bool state);

	private:
		chl_map extract_cheat_from_row(int row);
		void populate_cheat_table(void);
		void insert_cheat_row(int row);
		void update_cheat_row(int row, chl_map *cheat);

	private:
		void populate_edit_widgets(int row);
		void clear_edit_widgets(void);
		void set_edit_widget(void);
		void set_type_cheat_checkbox(chl_map *cheat);
		void set_edit_buttons(void);
		void change_active_compare_state(bool state);

	private slots:
		void s_cheat_item(void);
		void s_cheat_item_state(int state);
		void s_import(bool checked);
		void s_export(bool checked);
		void s_clear_all(bool checked);
		void s_hide_show_tools(bool checked);

	private slots:
		void s_grp_type_cheat(int id);
		void s_line_to_upper(const QString &text);
		void s_compare(int state);
		void s_new(bool checked);
		void s_remove(bool checked);
		void s_submit(bool checked);
		void s_cancel(bool checked);
};

#endif /* WDGCHEATSEDITOR_HPP_ */
