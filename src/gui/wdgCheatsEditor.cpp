/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or objchify
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

#include <QtGui/QClipboard>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QButtonGroup>
#include "wdgCheatsEditor.hpp"
#include "mainWindow.hpp"
#include "objCheat.hpp"
#include "conf.h"

enum cheat_table_rows {
	CR_ACTIVE,
	CR_DESCRIPTION,
	CR_CODE,
	CR_ADDRESS,
	CR_VALUE,
	CR_COMPARE,
	CR_ENABLED_COMPARE
};

wdgCheatsEditor::wdgCheatsEditor(QWidget *parent) : QWidget(parent) {
	objch = ((objCheat *)gui_objcheat_get_ptr());
	new_cheat = false;
	modified_cheat = false;
	in_populate_cheat_table = false;
	in_lineedit_text_changed = false;
	disable_hexspinbox_value_changed = false;
	COLOR_GG = theme::get_grayed_color(Qt::cyan);
	COLOR_ROCKY = theme::get_grayed_color(Qt::yellow);
	COLOR_MEM = theme::get_grayed_color(QColor(252, 215, 248));

	setupUi(this);

	stylesheet_update();

	setFocusProxy(tableWidget_Cheats);

	cheat_tableview_resize();

	connect(tableWidget_Cheats, SIGNAL(itemSelectionChanged()), this, SLOT(s_cheat_item()));
	connect(tableWidget_Cheats->model(),
		SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),
		this,
		SLOT(s_table_data_changed(QModelIndex,QModelIndex,QVector<int>)));
	connect(tableWidget_Cheats->model(),
		SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),
		this,
		SLOT(s_table_layout_changed(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)));

	connect(pushButton_Hide_Show_Tools, SIGNAL(clicked(bool)), this, SLOT(s_hide_show_tools(bool)));
	connect(pushButton_Import_Cheats, SIGNAL(clicked(bool)), this, SLOT(s_import(bool)));
	connect(pushButton_Export_Cheats, SIGNAL(clicked(bool)), this, SLOT(s_export(bool)));
	connect(pushButton_Delete_All_Cheats, SIGNAL(clicked(bool)), this, SLOT(s_delete_all(bool)));

	{
		grp = new QButtonGroup(this);

		grp->addButton(radioButton_CPU_Ram);
		grp->setId(radioButton_CPU_Ram, 0);
		grp->addButton(radioButton_GG);
		grp->setId(radioButton_GG, 1);
		grp->addButton(radioButton_ProAR);
		grp->setId(radioButton_ProAR, 2);

		connect(grp, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(s_grp_type_cheat(QAbstractButton*)));
	}

	{
		toUpValidator *val = new toUpValidator(this);

		lineEdit_GG->setValidator(val);
		lineEdit_ProAR->setValidator(val);

		connect(lineEdit_Description, SIGNAL(textEdited(QString)), SLOT(s_gg_proar_text_edited(QString)));
		connect(lineEdit_GG, SIGNAL(textEdited(QString)), SLOT(s_gg_proar_text_edited(QString)));
		connect(lineEdit_ProAR, SIGNAL(textEdited(QString)), SLOT(s_gg_proar_text_edited(QString)));
	}

	pushButton_Copy_GG->setProperty("myValue", QVariant(0));
	pushButton_Copy_ProAR->setProperty("myValue", QVariant(1));

	connect(pushButton_Copy_GG, SIGNAL(clicked(bool)), this, SLOT(s_copy(bool)));
	connect(pushButton_Copy_ProAR, SIGNAL(clicked(bool)), this, SLOT(s_copy(bool)));

	hexSpinBox_Address = new hexSpinBox(this, 4);
	hexSpinBox_Address->setRange(0, 0xFFFF);
	hexSpinBox_Address->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	hexSpinBox_Value = new hexSpinBox(this, 2);
	hexSpinBox_Value->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	hexSpinBox_Compare = new hexSpinBox(this, 2);
	hexSpinBox_Compare->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	gridLayout_Raw_Value->addWidget(hexSpinBox_Address, 0, 1);
	gridLayout_Raw_Value->addWidget(hexSpinBox_Value, 1, 1);
	gridLayout_Raw_Value->addWidget(hexSpinBox_Compare, 2, 1);

	QWidget::setTabOrder(pushButton_Copy_ProAR, hexSpinBox_Address);
	QWidget::setTabOrder(hexSpinBox_Address, hexSpinBox_Value);
	QWidget::setTabOrder(hexSpinBox_Value, checkBox_Compare);
	QWidget::setTabOrder(checkBox_Compare, hexSpinBox_Compare);

	pushButton_Cancel_Cheat->setEnabled(false);

	connect(hexSpinBox_Address, SIGNAL(valueChanged(int)), this, SLOT(s_hexspinbox_value_changed(int)));
	connect(hexSpinBox_Value, SIGNAL(valueChanged(int)), this, SLOT(s_hexspinbox_value_changed(int)));
	connect(checkBox_Compare, SIGNAL(stateChanged(int)), this, SLOT(s_compare(int)));
	connect(hexSpinBox_Compare, SIGNAL(valueChanged(int)), this, SLOT(s_hexspinbox_value_changed(int)));

	pushButton_New_Cheat->setProperty("myValue", QVariant(0));
	pushButton_New_Cheat_GG->setProperty("myValue", QVariant(1));
	pushButton_New_Cheat_ProAR->setProperty("myValue", QVariant(2));

	connect(pushButton_New_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_new(bool)));
	connect(pushButton_New_Cheat_GG, SIGNAL(clicked(bool)), this, SLOT(s_new(bool)));
	connect(pushButton_New_Cheat_ProAR, SIGNAL(clicked(bool)), this, SLOT(s_new(bool)));

	connect(pushButton_Delete_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_delete(bool)));
	connect(pushButton_Submit_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_submit(bool)));
	connect(pushButton_Cancel_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_cancel(bool)));

	{
		int w = QLabel("0000000000").sizeHint().width() + 10;

		lineEdit_CPU_Ram->setMinimumWidth(w);
		lineEdit_GG->setMinimumWidth(w);
		lineEdit_ProAR->setMinimumWidth(w);
	}

	{
		int dim = fontMetrics().height();

		icon_Cheat_List_Editor->setPixmap(QIcon(":/icon/icons/cheats_list.svgz").pixmap(dim, dim));
		icon_Editor_Tools->setPixmap(QIcon(":/icon/icons/pencil.svgz").pixmap(dim, dim));
	}

	installEventFilter(this);

	populate_cheat_table();

	if (tableWidget_Cheats->rowCount() > 0) {
		tableWidget_Cheats->selectRow(0);
	}

	s_cheat_item();
}
wdgCheatsEditor::~wdgCheatsEditor() = default;

void wdgCheatsEditor::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else if (event->type() == QEvent::PaletteChange) {
		stylesheet_update();
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgCheatsEditor::showEvent(QShowEvent *event) {
	lineEdit_CPU_Ram->setVisible(false);
	radioButton_CPU_Ram->setFixedHeight(radioButton_ProAR->height());
	QWidget::showEvent(event);
}

void wdgCheatsEditor::stylesheet_update(void) {
	label_color_CPU_Ram->setStyleSheet(stylesheet_label(QColor("#FCD7F8")));
	label_color_GG->setStyleSheet(stylesheet_label(QColor("cyan")));
	label_color_ProAR->setStyleSheet(stylesheet_label(QColor("yellow")));
}

void wdgCheatsEditor::hide_tools_widgets(bool state) {
	if (widget_Edit->isHidden() == state) {
		return;
	}

	if (state) {
		pushButton_Hide_Show_Tools->setText(tr("Show Tools"));
	} else {
		pushButton_Hide_Show_Tools->setText(tr("Hide Tools"));
	}

	widget_Edit->setHidden(state);
}
void wdgCheatsEditor::populate_cheat_table(void) {
	int i;

	in_populate_cheat_table = true;

	for (i = 1; i < tableWidget_Cheats->rowCount(); i++) {
		tableWidget_Cheats->removeRow(i);
	}

	tableWidget_Cheats->setRowCount(0);

	for (i = 0; i < objch->cheats.count(); i++) {
		insert_cheat_row(i);
	}

	in_populate_cheat_table = false;
}

QString wdgCheatsEditor::stylesheet_label(const QColor &color) {
	QColor background = theme::get_grayed_color(color);
	QString stylesheet =
			"QLabel {"\
"	background-color: %0;"\
" }";

	return stylesheet.arg(background.name());
}

chl_map wdgCheatsEditor::extract_cheat_from_row(int row) {
	chl_map cheat;

	cheat.clear();

	cheat.insert("enabled", tableWidget_Cheats->item(row, CR_ACTIVE)->text());
	cheat.insert("genie", "-");
	cheat.insert("rocky", "-");
	cheat.insert("address", tableWidget_Cheats->item(row, CR_ADDRESS)->text());
	cheat.insert("value", tableWidget_Cheats->item(row, CR_VALUE)->text());
	cheat.insert("compare", tableWidget_Cheats->item(row, CR_COMPARE)->text());
	cheat.insert("description", tableWidget_Cheats->item(row, CR_DESCRIPTION)->text());
	cheat.insert("enabled_compare", tableWidget_Cheats->item(row, CR_ENABLED_COMPARE)->text());

	if (tableWidget_Cheats->item(row, CR_CODE)->background() == COLOR_GG) {
		cheat["genie"] = tableWidget_Cheats->item(row, CR_CODE)->text();
	} else if (tableWidget_Cheats->item(row, CR_CODE)->background() == COLOR_ROCKY) {
		cheat["rocky"] = tableWidget_Cheats->item(row, CR_CODE)->text();
	}

	return (cheat);
}
void wdgCheatsEditor::insert_cheat_row(int row) {
	chl_map cheat = objch->cheats.at(row);
	QTableWidgetItem *col;

	tableWidget_Cheats->insertRow(row);

	{
		QWidget *widget = new QWidget(this);
		QHBoxLayout* layout = new QHBoxLayout(widget);
		QCheckBox *active = new QCheckBox(widget);

		widget->setObjectName(QString("widget%1").arg(row));
		active->setObjectName("active");
		connect(active, SIGNAL(stateChanged(int)), this, SLOT(s_cheat_item_state(int)));
		layout->addWidget(active);
		layout->setAlignment(Qt::AlignCenter);
		layout->setContentsMargins(0, 0, 0, 0);
		widget->setLayout(layout);
		tableWidget_Cheats->setCellWidget(row, CR_ACTIVE, widget);

		col = new QTableWidgetItem();
		col->setTextAlignment(Qt::AlignCenter);
		tableWidget_Cheats->setItem(row, CR_ACTIVE, col);
	}

	col = new QTableWidgetItem();
	tableWidget_Cheats->setItem(row, CR_DESCRIPTION, col);

	col = new QTableWidgetItem();
	col->setTextAlignment(Qt::AlignCenter);
	tableWidget_Cheats->setItem(row, CR_CODE, col);

	col = new QTableWidgetItem();
	col->setTextAlignment(Qt::AlignCenter);
	tableWidget_Cheats->setItem(row, CR_ADDRESS, col);

	col = new QTableWidgetItem();
	col->setTextAlignment(Qt::AlignCenter);
	tableWidget_Cheats->setItem(row, CR_VALUE, col);

	col = new QTableWidgetItem();
	col->setTextAlignment(Qt::AlignCenter);
	tableWidget_Cheats->setItem(row, CR_COMPARE, col);

	col = new QTableWidgetItem();
	col->setTextAlignment(Qt::AlignCenter);
	tableWidget_Cheats->setItem(row, CR_ENABLED_COMPARE, col);

	update_cheat_row(row, &cheat);
}
void wdgCheatsEditor::update_cheat_row(int row, chl_map *cheat) {
	QCheckBox *active = tableWidget_Cheats->cellWidget(row, CR_ACTIVE)->findChild<QCheckBox*>("active");

	active->blockSignals(true);
	if ((*cheat)["enabled"].toInt() == 1) {
		active->setChecked(true);
		tableWidget_Cheats->item(row, CR_ACTIVE)->setText("1");
	} else {
		active->setChecked(false);
		tableWidget_Cheats->item(row, CR_ACTIVE)->setText("0");
	}
	active->blockSignals(false);

	tableWidget_Cheats->item(row, CR_DESCRIPTION)->setText((*cheat)["description"]);
	tableWidget_Cheats->item(row, CR_DESCRIPTION)->setToolTip((*cheat)["description"]);

	if ((*cheat)["genie"] != "-") {
		tableWidget_Cheats->item(row, CR_CODE)->setText((*cheat)["genie"]);
		tableWidget_Cheats->item(row, CR_CODE)->setBackground(COLOR_GG);
		tableWidget_Cheats->item(row, CR_CODE)->setForeground(theme::get_foreground_color(COLOR_GG));
	} else if ((*cheat)["rocky"] != "-") {
		tableWidget_Cheats->item(row, CR_CODE)->setText((*cheat)["rocky"]);
		tableWidget_Cheats->item(row, CR_CODE)->setBackground(COLOR_ROCKY);
		tableWidget_Cheats->item(row, CR_CODE)->setForeground(theme::get_foreground_color(COLOR_ROCKY));
	} else {
		tableWidget_Cheats->item(row, CR_CODE)->setText("-");
		tableWidget_Cheats->item(row, CR_CODE)->setBackground(COLOR_MEM);
		tableWidget_Cheats->item(row, CR_CODE)->setForeground(theme::get_foreground_color(COLOR_MEM));
	}

	tableWidget_Cheats->item(row, CR_ADDRESS)->setText((*cheat)["address"]);
	tableWidget_Cheats->item(row, CR_VALUE)->setText((*cheat)["value"]);
	tableWidget_Cheats->item(row, CR_COMPARE)->setText((*cheat)["compare"]);
	tableWidget_Cheats->item(row, CR_ENABLED_COMPARE)->setText((*cheat)["enabled_compare"]);

	update_color_row(row, (*cheat)["enabled"].toInt() == 1);
}
void wdgCheatsEditor::update_color_row(int row, bool active) {
	QBrush brush = QBrush(QColor::fromRgb(255, 255, 255, 0));
	int i;

	if (active == 1) {
		brush = QBrush(theme::get_grayed_color(QColor::fromRgb(214, 255, 182, 255)));
	}

	for (i = 0; i < tableWidget_Cheats->columnCount(); i++) {
		QTableWidgetItem *item = tableWidget_Cheats->item(row, i);

		if (!item || (i == CR_CODE)) {
			continue;
		}
		item->setBackground(brush);
		if (active == 1) {
			item->setForeground(theme::get_foreground_color(brush.color()));
		} else {
			item->setForeground(palette().text().color());
		}
	}
}

void wdgCheatsEditor::ctrl_last_cheat(void) {
	if (extract_cheat_from_edit_widget() != last_cheat) {
		modified_cheat = true;
		widget_Cheats_List->setEnabled(false);
		set_edit_widget();
		set_edit_buttons();
	} else {
		s_cancel(false);
	}
}
void wdgCheatsEditor::linedit_select_all(QLineEdit *le) {
	le->setFocus(Qt::ActiveWindowFocusReason);
	QTimer::singleShot(0, le, &QLineEdit::selectAll);
}
void wdgCheatsEditor::cheat_tableview_resize(void) {
	QHeaderView *hv = tableWidget_Cheats->horizontalHeader();

	tableWidget_Cheats->setColumnCount(tableWidget_Cheats->columnCount() + 1);
	tableWidget_Cheats->setColumnHidden(CR_ENABLED_COMPARE, true);

	// setto la dimensione del font
	{
		QFont f = tableWidget_Cheats->font();
		int pointsize = f.pointSize() - 1;

		if (pointsize >= 8) {
			f.setPointSize(pointsize);
			tableWidget_Cheats->setFont(f);
		}
	}

	// setto il resizeMode delle colonne
	hv->setSectionResizeMode(QHeaderView::Stretch);

	hv->setSectionResizeMode(CR_ACTIVE, QHeaderView::ResizeToContents);
	hv->setSectionResizeMode(CR_DESCRIPTION, QHeaderView::Stretch);
	hv->setSectionResizeMode(CR_CODE, QHeaderView::ResizeToContents);
	hv->setSectionResizeMode(CR_ADDRESS, QHeaderView::ResizeToContents);
	hv->setSectionResizeMode(CR_VALUE, QHeaderView::ResizeToContents);
	hv->setSectionResizeMode(CR_COMPARE, QHeaderView::ResizeToContents);
}
void wdgCheatsEditor::populate_lineedit_gg_rocky(bool control_widgets) {
	bool gg = true, rocky = true;
	_cheat ch;

	ch.address = hexSpinBox_Address->value();
	ch.replace = hexSpinBox_Value->value();
	ch.enabled_compare = checkBox_Compare->isChecked();
	ch.compare = hexSpinBox_Compare->value();

	if (in_lineedit_text_changed) {
		switch (grp->checkedId()) {
			case 1:
				lineEdit_ProAR->setText(objch->encode_rocky(&ch));
				break;
			case 2:
				lineEdit_GG->setText(objch->encode_gg(&ch));
				break;
			default:
				break;
		}
	} else {
		lineEdit_GG->setText(objch->encode_gg(&ch));
		lineEdit_ProAR->setText(objch->encode_rocky(&ch));
	}

	if (lineEdit_GG->text() == "-") {
		gg = false;
	}
	radioButton_GG->setEnabled(gg);

	if (lineEdit_ProAR->text() == "-") {
		rocky = false;
	}
	radioButton_ProAR->setEnabled(rocky);

	if (in_lineedit_text_changed) {
		switch (grp->checkedId()) {
			case 1:
				pushButton_Copy_ProAR->setEnabled(rocky);
				break;
			case 2:
				pushButton_Copy_GG->setEnabled(gg);
				break;
			default:
				break;
		}
	} else {
		pushButton_Copy_GG->setEnabled(gg);
		pushButton_Copy_ProAR->setEnabled(rocky);
	}

	if (!control_widgets) {
		return;
	}

	if (((grp->checkedId() == 1) && !gg) || ((grp->checkedId() == 2) && !rocky)) {
		radioButton_CPU_Ram->click();
		return;
	}
}
void wdgCheatsEditor::populate_edit_raw(_cheat *cheat) {
	if (cheat) {
		hexSpinBox_Address->setValue(cheat->address);
		hexSpinBox_Value->setValue(cheat->replace);
		checkBox_Compare->setChecked(cheat->enabled_compare);
		hexSpinBox_Compare->setValue(cheat->compare);

		s_compare(cheat->enabled_compare ? Qt::Checked : Qt::Unchecked);
	}
}
chl_map wdgCheatsEditor::extract_cheat_from_edit_widget(void) {
	chl_map cheat;

	cheat.clear();

	cheat.insert("description", lineEdit_Description->text());

	if (radioButton_CPU_Ram->isChecked()) {
		cheat.insert("genie", "-");
		cheat.insert("rocky", "-");
		cheat.insert("address", "0x" + QString("%1").arg(hexSpinBox_Address->value(), 4, 16, QChar('0')).toUpper());
		cheat.insert("value", "0x" + QString("%1").arg(hexSpinBox_Value->value(), 2, 16, QChar('0')).toUpper());
		if (checkBox_Compare->isChecked()) {
			cheat.insert("enabled_compare", "1");
			cheat.insert("compare", "0x" + QString("%1").arg(hexSpinBox_Compare->value(), 2, 16, QChar('0')).toUpper());
		} else {
			cheat.insert("enabled_compare", "0");
			cheat.insert("compare", "-");
		}
	} else if (radioButton_GG->isChecked()) {
		cheat.insert("genie", lineEdit_GG->text());
		cheat.insert("rocky", "-");
		objch->complete_gg(&cheat);
	} else if (radioButton_ProAR->isChecked()) {
		cheat.insert("genie", "-");
		cheat.insert("rocky", lineEdit_ProAR->text());
		objch->complete_rocky(&cheat);
	}

	return (cheat);
}
void wdgCheatsEditor::populate_edit_widgets(int row) {
	chl_map cheat;

	if (row < 0) {
		radioButton_CPU_Ram->setChecked(true);
		clear_edit_widgets();
		return;
	}

	cheat = extract_cheat_from_row(row);

	widget_Edit->setEnabled(true);
	lineEdit_Description->setText(cheat["description"]);

	set_edit_widget();
	set_type_cheat_checkbox(&cheat);

	{
		bool ok;
		_cheat ch = {
			FALSE,
			cheat["compare"] == "-" ? (BYTE)FALSE : (BYTE)TRUE,
			(WORD)cheat["address"].toInt(&ok, 16),
			(BYTE)cheat["value"].toInt(&ok, 16),
			cheat["compare"] == "-" ? (BYTE)0 : (BYTE)(cheat["compare"].toInt(&ok, 16))
		};

		populate_edit_raw(&ch);
		populate_lineedit_gg_rocky(false);
	}

	last_cheat = extract_cheat_from_edit_widget();
}
void wdgCheatsEditor::clear_edit_widgets(void) {
	_cheat ch = { FALSE, TRUE, 0x8000, 0, 0 };

	lineEdit_Description->setText("");
	populate_edit_raw(&ch);
	populate_lineedit_gg_rocky(false);
}
void wdgCheatsEditor::set_edit_widget(void) {
	if (new_cheat || (tableWidget_Cheats->currentRow() >= 0)) {
		label_Description->setEnabled(true);
		lineEdit_Description->setEnabled(true);
		frame_Type_Cheat->setEnabled(true);
		frame_Raw_Value->setEnabled(true);
		frame_Buttons->setEnabled(true);
	} else {
		label_Description->setEnabled(false);
		lineEdit_Description->setEnabled(false);
		frame_Type_Cheat->setEnabled(false);
		frame_Raw_Value->setEnabled(false);
		frame_Buttons->setEnabled(true);
	}
}
void wdgCheatsEditor::set_type_cheat_checkbox(chl_map *cheat) {
	QRadioButton *button;

	if (!cheat) {
		return;
	}

	if (new_cheat || modified_cheat) {
		radioButton_CPU_Ram->setEnabled(true);
		radioButton_GG->setEnabled(true);
		radioButton_ProAR->setEnabled(true);
		return;
	}
	if ((*cheat)["genie"] != "-") {
		button = radioButton_GG;
		lineEdit_GG->setText((*cheat)["genie"]);
	} else if ((*cheat)["rocky"] != "-") {
		button = radioButton_ProAR;
		lineEdit_ProAR->setText((*cheat)["rocky"]);
	} else {
		button = radioButton_CPU_Ram;
	}
	button->setEnabled(true);
	button->click();
}
void wdgCheatsEditor::set_edit_buttons(void) {
	if (new_cheat || modified_cheat) {
		pushButton_Delete_Cheat->setEnabled(false);

		pushButton_New_Cheat->setEnabled(false);
		pushButton_New_Cheat_GG->setEnabled(false);
		pushButton_New_Cheat_ProAR->setEnabled(false);

		pushButton_Cancel_Cheat->setEnabled(true);
		pushButton_Submit_Cheat->setEnabled(true);
		return;
	}
	if (tableWidget_Cheats->currentRow() >= 0) {
		pushButton_Delete_Cheat->setEnabled(true);
	} else {
		pushButton_Delete_Cheat->setEnabled(false);
	}

	pushButton_Submit_Cheat->setEnabled(false);
	pushButton_Cancel_Cheat->setEnabled(false);

	pushButton_New_Cheat->setEnabled(true);
	pushButton_New_Cheat_GG->setEnabled(true);
	pushButton_New_Cheat_ProAR->setEnabled(true);
}

void wdgCheatsEditor::s_table_data_changed(const QModelIndex &topLeft, UNUSED(const QModelIndex &bottomRight),
	UNUSED(const QVector<int> &roles)) {
	if (in_populate_cheat_table) {
		return;
	}

	if (topLeft.column() == CR_ACTIVE) {
		update_color_row(topLeft.row(), (tableWidget_Cheats->item(topLeft.row(), CR_ACTIVE)->text() == "1"));
	}
}
void wdgCheatsEditor::s_table_layout_changed(UNUSED(const QList<QPersistentModelIndex> &sourceParents),
	UNUSED(QAbstractItemModel::LayoutChangeHint hint)) {
	int i;

	for (i = 0; i < tableWidget_Cheats->rowCount(); i++) {
		update_color_row(i, (tableWidget_Cheats->item(i, CR_ACTIVE)->text() == "1"));
	}
}
void wdgCheatsEditor::s_cheat_item(void) {
	disable_hexspinbox_value_changed = true;
	set_edit_widget();
	set_edit_buttons();
	populate_edit_widgets(tableWidget_Cheats->currentRow());
	disable_hexspinbox_value_changed = false;
}
void wdgCheatsEditor::s_cheat_item_state(int state) {
	int a;

	for (a = 0; a < tableWidget_Cheats->rowCount(); a++) {
		QWidget *widget = tableWidget_Cheats->cellWidget(a, CR_ACTIVE);

		if (widget->objectName() == ((QObject *)sender())->parent()->objectName()) {
			tableWidget_Cheats->selectRow(a);
			if (state == Qt::Checked) {
				tableWidget_Cheats->item(a, CR_ACTIVE)->setText("1");
			} else {
				tableWidget_Cheats->item(a, CR_ACTIVE)->setText("0");
			}
			break;
		}
	}

	{
		chl_map cheat = extract_cheat_from_row(a);
		int b;

		if ((b = objch->find_cheat(&cheat, true)) != -1) {
			cheat["enabled"] = tableWidget_Cheats->item(a, CR_ACTIVE)->text();

			objch->cheats.replace(b, cheat);
			objch->save_game_cheats(this);
		}
	}

	objch->apply_cheats();
}
void wdgCheatsEditor::s_hide_show_tools(UNUSED(bool checked)) {
	hide_tools_widgets(!widget_Edit->isHidden());
}
void wdgCheatsEditor::s_import(UNUSED(bool checked)) {
	QStringList filters;
	QString file;

	filters.append(tr("All supported formats"));
	filters.append(tr("Nestopia XML files"));
	filters.append(tr("Mame 128+ XML files"));
	filters.append(tr("FCEUX CHT files"));
	filters.append(tr("libretro CHT files"));

	filters[0].append(" (*.xml *.XML *.cht *.CHT)");
	filters[1].append(" (*.xml *.XML)");
	filters[2].append(" (*.xml *.XML)");
	filters[3].append(" (*.cht *.CHT)");
	filters[3].append(" (*.cht *.CHT)");

	file = QFileDialog::getOpenFileName(this, tr("Import Cheats"),
		uQString(cfg->last_import_cheat_path), filters.join(";;"));

	if (!file.isNull()) {
		QFileInfo fileinfo(file);

		if (!fileinfo.suffix().compare("xml", Qt::CaseInsensitive)) {
			objch->import_Nestopia_xml(this, fileinfo.absoluteFilePath());
			if (objch->cheats.count() == 0) {
				objch->import_MAME_xml(this, fileinfo.absoluteFilePath());
			}
		} else if (!fileinfo.suffix().compare("cht", Qt::CaseInsensitive)) {
			objch->import_FCEUX_cht(fileinfo.absoluteFilePath());
			if (objch->cheats.count() == 0) {
				objch->import_libretro_cht(fileinfo.absoluteFilePath());
			}
		}
		populate_cheat_table();
		objch->save_game_cheats(this);
		umemset(cfg->last_import_cheat_path, 0x00, usizeof(cfg->last_import_cheat_path));
		ustrncpy(cfg->last_import_cheat_path, uQStringCD(fileinfo.absolutePath()), usizeof(cfg->last_import_cheat_path) - 1);
	}
}
void wdgCheatsEditor::s_export(UNUSED(bool checked)) {
	QStringList filters;
	QString file;

	filters.append(tr("Nestopia XML files"));
	filters.append(tr("All files"));

	filters[0].append(" (*.xml *.XML)");
	filters[1].append(" (*.*)");

	file = QFileDialog::getSaveFileName(this, tr("Export cheats on file"),
		QFileInfo(uQString(info.rom.file)).completeBaseName() + ".xml", filters.join(";;"));

	if (!file.isNull()) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			fileinfo.setFile(QString(file) + ".xml");
		}

		objch->save_Nestopia_xml(this, fileinfo.absoluteFilePath());
	}
}
void wdgCheatsEditor::s_delete(UNUSED(bool checked)) {
	chl_map cheat = extract_cheat_from_row(tableWidget_Cheats->currentRow());
	int i;

	if ((i = objch->find_cheat(&cheat, true)) != -1) {
		objch->cheats.removeAt(i);
	}
	tableWidget_Cheats->removeRow(tableWidget_Cheats->currentRow());
	objch->save_game_cheats(this);
}
void wdgCheatsEditor::s_delete_all(UNUSED(bool checked)) {
	tableWidget_Cheats->setRowCount(0);
	objch->cheats.clear();
	objch->save_game_cheats(this);
}

void wdgCheatsEditor::s_grp_type_cheat(UNUSED(QAbstractButton *button)) {
	frame_Raw_Value->setEnabled(true);

	if (!disable_hexspinbox_value_changed) {
		ctrl_last_cheat();
	}

	switch (grp->checkedId()) {
		case 0:
			lineEdit_GG->setEnabled(false);
			lineEdit_ProAR->setEnabled(false);
			break;
		case 1:
			lineEdit_GG->setEnabled(true);
			lineEdit_ProAR->setEnabled(false);
			break;
		case 2:
			lineEdit_GG->setEnabled(false);
			lineEdit_ProAR->setEnabled(true);
			break;
		default:
			return;
	}
	s_hexspinbox_value_changed(0);
}
void wdgCheatsEditor::s_gg_proar_text_edited(UNUSED(const QString &text)) {
	QLineEdit *le = qobject_cast<QLineEdit *>(sender());
	bool enabled = true;
	_cheat cheat;

	if (!le) {
		return;
	}
	switch (grp->checkedId()) {
		default:
		case 0:
			if (objch->decode_ram(extract_cheat_from_edit_widget(), &cheat) == EXIT_ERROR) {
				enabled = false;
			}
			break;
		case 1:
			if (objch->decode_gg(lineEdit_GG->text(), &cheat) == EXIT_ERROR) {
				enabled = false;
			}
			pushButton_Copy_GG->setEnabled(enabled);
			break;
		case 2:
			if (objch->decode_rocky(lineEdit_ProAR->text(), &cheat) == EXIT_ERROR) {
				enabled = false;
			}
			pushButton_Copy_ProAR->setEnabled(enabled);
			break;
	}
	frame_Raw_Value->setEnabled(enabled);
	pushButton_Submit_Cheat->setEnabled(enabled);
	in_lineedit_text_changed = true;
	populate_edit_raw(&cheat);
	in_lineedit_text_changed = false;
}
void wdgCheatsEditor::s_copy(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myValue")).toInt();
	QClipboard *clip = QApplication::clipboard();
	QLineEdit *le;

	switch (index) {
		default:
		case 0:
			le = lineEdit_GG;
			break;
		case 1:
			le = lineEdit_ProAR;
			break;
	}
	clip->setText(le->text(), QClipboard::Clipboard);
}
void wdgCheatsEditor::s_hexspinbox_value_changed(UNUSED(int i)) {
	if (!disable_hexspinbox_value_changed) {
		populate_lineedit_gg_rocky(true);
		ctrl_last_cheat();
	}
}
void wdgCheatsEditor::s_compare(int state) {
	if (state == Qt::Checked) {
		hexSpinBox_Compare->setEnabled(true);
	} else {
		hexSpinBox_Compare->setEnabled(false);
	}
	s_hexspinbox_value_changed(0);
}
void wdgCheatsEditor::s_new(UNUSED(bool checked)) {
	int index = QVariant(((QObject *)sender())->property("myValue")).toInt();
	QLineEdit *le = nullptr;
	QRadioButton *rb;

	new_cheat = true;

	widget_Cheats_List->setEnabled(false);

	clear_edit_widgets();

	set_edit_widget();
	set_edit_buttons();

	set_type_cheat_checkbox(nullptr);

	switch (index) {
		default:
		case 0:
			rb = radioButton_CPU_Ram;
			break;
		case 1:
			rb = radioButton_GG;
			le = lineEdit_GG;
			break;
		case 2:
			rb = radioButton_ProAR;
			le = lineEdit_ProAR;
			break;
	}
	rb->click();
	if (le) {
		linedit_select_all(le);
	}
}
void wdgCheatsEditor::s_submit(UNUSED(bool checked)) {
	bool submitted = true;
	int i, current;
	chl_map cheat;

	if (lineEdit_Description->text().isEmpty()) {
		QMessageBox::warning(this, tr("Submit warning"), tr("A description must be entered"));
		return;
	}

	cheat = extract_cheat_from_edit_widget();

	if (cheat.count() == 0) {
		QMessageBox::warning(this, tr("Submit warning"), tr("The code is invalid"));
		switch (grp->checkedId()) {
			default:
			case 0:
				hexSpinBox_Address->setFocus();
				break;
			case 1:
				lineEdit_GG->setFocus();
				break;
			case 2:
				lineEdit_ProAR->setFocus();
				break;
		}
		return;
	}

	if (new_cheat) {
		current = tableWidget_Cheats->rowCount();
	} else {
		current = tableWidget_Cheats->currentRow();
	}

	if ((i = objch->find_cheat(&cheat, false)) == -1) {
		if (new_cheat) {
			objch->cheats.insert(current, cheat);
			insert_cheat_row(current);
			tableWidget_Cheats->selectRow(current);
		} else {
			objch->cheats.replace(current, cheat);
			update_cheat_row(current, &cheat);
		}
	} else {
		if (!new_cheat && (i == tableWidget_Cheats->currentRow())) {
			objch->cheats.replace(current, cheat);
			update_cheat_row(current, &cheat);
		} else {
			QMessageBox::warning(this, tr("Submit warning"), tr("The cheat is already in the list"));
			submitted = false;
		}
	}

	if (!submitted) {
		return;
	}

	if (new_cheat || modified_cheat) {
		s_cancel(false);
	}

	last_cheat = cheat;

	objch->save_game_cheats(this);
}
void wdgCheatsEditor::s_cancel(UNUSED(bool checked)) {
	new_cheat = false;
	modified_cheat = false;
	widget_Cheats_List->setEnabled(true);
	s_cheat_item();
}

// ----------------------------------------------------------------------------------------------

hexSpinBox::hexSpinBox(QWidget *parent, int dgts) : QSpinBox(parent) {
	digits = dgts;
	no_prefix = false;

	switch (digits) {
		case 1:
			setRange(0, 0xF);
			break;
		case 2:
			setRange(0, 0xFF);
			break;
		case 3:
			setRange(0, 0xFFF);
			break;
		default:
		case 4:
			setRange(0, 0xFFFF);
			break;
	}

	setFocusPolicy(Qt::StrongFocus);

	validator = new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{1,8}"), this);

	installEventFilter(this);
}
hexSpinBox::~hexSpinBox() = default;

bool hexSpinBox::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::FocusIn) {
		no_prefix = true;
		setValue(this->value());
	} else if (event->type() == QEvent::FocusOut) {
		no_prefix = false;
	}

	return (QObject::eventFilter(obj, event));
}
QValidator::State hexSpinBox::validate(QString &text, int &pos) const {
	return (validator->validate(text, pos));
}
QString hexSpinBox::textFromValue(int value) const {
	if (no_prefix) {
		return (QString(QString("%1").arg(value, digits, 16, QChar('0')).toUpper()));
	} else {
		return (QString("0x" + QString("%1").arg(value, digits, 16, QChar('0')).toUpper()));
	}
}
int hexSpinBox::valueFromText(const QString &text) const {
	bool ok;

	return (text.toInt(&ok, 16));
}
