/*
 * dlgCheats.hpp
 *
 *  Created on: 04/mar/2015
 *      Author: fhorse
 */

#ifndef DLGCHEATS_HPP_
#define DLGCHEATS_HPP_

#include <QtCore/QtGlobal>
#include <QtXml/QXmlStreamReader>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDialog>
#include <QtGui/QSpinBox>
#else
#include <QtWidgets/QDialog>
#include <QtWidgets/QSpinBox>
#endif
#include "dlgCheats.hh"
#include "cheatObject.hpp"

class hexSpinBox: public QSpinBox {
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

class dlgCheats : public QDialog, public Ui::Cheats {
		Q_OBJECT

	private:
		int tools_height;
		bool new_mode;
		cheatObject *mod, *org;
		hexSpinBox *hexSpinBox_Address;
		hexSpinBox *hexSpinBox_Value;
		hexSpinBox *hexSpinBox_Compare;

	public:
		dlgCheats(QWidget *parent, cheatObject *c);
		~dlgCheats();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	private:
		chl_map extract_cheat_from_row(int row);
		void populate_cheat_table();
		void insert_cheat_row(int row);
		void update_cheat_row(int row, chl_map *cheat);
		void hide_tools_widgets(bool state);
		void populate_edit_widgets(int row);
		void clear_edit_widgets();
		void set_edit_widget();
		void set_type_cheat_checkbox(chl_map *cheat);
		void set_edit_buttons();
		void change_active_compare_state(bool state);

	private slots:
		void s_active_compare_state_changed(int state);
		void s_item_selection_changed();
		void s_active_state_changed(int state);
		void s_import_clicked(bool checked);
		void s_export_clicked(bool checked);
		void s_clear_all_clicked(bool checked);
		void s_grp_button_clicked(int id);
		void s_linedit_to_upper(const QString &text);
		void s_new_clicked(bool checked);
		void s_remove_clicked(bool checked);
		void s_submit_clicked(bool checked);
		void s_cancel_clicked(bool checked);
		void s_hide_show_tools_clicked(bool checked);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);
};

#endif /* DLGCHEATS_HPP_ */
