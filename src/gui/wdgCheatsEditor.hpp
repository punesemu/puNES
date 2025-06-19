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

#ifndef WDGCHEATSEDITOR_HPP_
#define WDGCHEATSEDITOR_HPP_

#include <QtWidgets/QPushButton>
#include <QtCore/QXmlStreamReader>
#include <QtWidgets/QWidget>
#include <QtWidgets/QSpinBox>
#include "objCheat.hpp"

class hexSpinBox final : public QSpinBox {
	private:
		int digits;
		bool no_prefix;
		QRegularExpressionValidator *validator;

	public:
		explicit hexSpinBox(QWidget *parent = nullptr, int dgts = 4);
		~hexSpinBox() override;

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;
		QValidator::State validate(QString &text, int &pos) const override;
		int valueFromText(const QString &text) const override;
		QString textFromValue(int value) const override;
};

#include "ui_wdgCheatsEditor.h"

class wdgCheatsEditor final : public QWidget, public Ui::wdgCheatEditor {
	Q_OBJECT

	private:
		bool new_cheat;
		bool modified_cheat;
		objCheat *objch;
		QButtonGroup *grp;
		hexSpinBox *hexSpinBox_Address;
		hexSpinBox *hexSpinBox_Value;
		hexSpinBox *hexSpinBox_Compare;
		bool in_populate_cheat_table;
		bool in_lineedit_text_changed;
		bool disable_hexspinbox_value_changed;
		chl_map last_cheat;
		QColor COLOR_GG;
		QColor COLOR_ROCKY;
		QColor COLOR_MEM;

	public:
		explicit wdgCheatsEditor(QWidget *parent = nullptr);
		~wdgCheatsEditor() override;

	protected:
		void changeEvent(QEvent *event) override;
		void showEvent(QShowEvent *event) override;

	private:
		void stylesheet_update(void) const;

	public:
		void hide_tools_widgets(bool state) const;
		void populate_cheat_table(void);

	private:
		static QString stylesheet_label(const QColor &color);

	private:
		chl_map extract_cheat_from_row(int row) const;
		void insert_cheat_row(int row);
		void update_cheat_row(int row, chl_map *cheat) const;
		void update_color_row(int row, bool active) const;

	private:
		void ctrl_last_cheat(void);
		static void linedit_select_all(QLineEdit *le);
		void cheat_tableview_resize(void) const;
		void populate_lineedit_gg_rocky(bool control_widgets) const;
		void populate_edit_raw(const _cheat *cheat);
		chl_map extract_cheat_from_edit_widget(void) const;
		void populate_edit_widgets(int row);
		void clear_edit_widgets(void);
		void set_edit_widget(void) const;
		void set_type_cheat_checkbox(chl_map *cheat) const;
		void set_edit_buttons(void) const;

	private slots:
		void s_table_data_changed(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) const;
		void s_table_layout_changed(const QList<QPersistentModelIndex> &sourceParents, QAbstractItemModel::LayoutChangeHint hint) const;
		void s_cheat_item(void);
		void s_cheat_item_state(int state);
		void s_hide_show_tools(bool checked) const;
		void s_import(bool checked);
		void s_export(bool checked);
		void s_delete(bool checked);
		void s_delete_all(bool checked);

	private slots:
		void s_grp_type_cheat(QAbstractButton *button);
		void s_gg_proar_text_edited(const QString &text);
		void s_copy(bool checked) const;
		void s_hexspinbox_value_changed(int i);
		void s_compare(int state);
		void s_new(bool checked);
		void s_submit(bool checked);
		void s_cancel(bool checked);
};

#endif /* WDGCHEATSEDITOR_HPP_ */
