/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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

#include <QtWidgets/QMessageBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QButtonGroup>
#include "wdgCheatsEditor.moc"
#include "mainWindow.hpp"
#include "conf.h"

#define COLOR_GG    Qt::cyan
#define COLOR_ROCKY Qt::yellow
#define COLOR_MEM   QColor(252, 215, 248)

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
	in_populate_cheat_table = false;

	setupUi(this);

	setFocusProxy(tableWidget_Cheats);

	cheat_tableview_resize();

	connect(tableWidget_Cheats, SIGNAL(itemSelectionChanged()), this, SLOT(s_cheat_item()));
	connect(tableWidget_Cheats->model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &)),
		this, SLOT(s_table_data_changed(const QModelIndex &, const QModelIndex &, const QVector<int> & )));
	connect(tableWidget_Cheats->model(),
		SIGNAL(layoutChanged(const QList<QPersistentModelIndex> &, QAbstractItemModel::LayoutChangeHint)), this,
		SLOT(s_table_layout_changed(const QList<QPersistentModelIndex> &, QAbstractItemModel::LayoutChangeHint)));

	connect(pushButton_Hide_Show_Tools, SIGNAL(clicked(bool)), this, SLOT(s_hide_show_tools(bool)));
	connect(pushButton_Import_Cheats, SIGNAL(clicked(bool)), this, SLOT(s_import(bool)));
	connect(pushButton_Export_Cheats, SIGNAL(clicked(bool)), this, SLOT(s_export(bool)));
	connect(pushButton_Clear_All_Cheats, SIGNAL(clicked(bool)), this, SLOT(s_clear_all(bool)));

	{
		QButtonGroup *grp = new QButtonGroup(this);

		grp->addButton(radioButton_CPU_Ram);
		grp->setId(radioButton_CPU_Ram, 0);
		grp->addButton(radioButton_GG);
		grp->setId(radioButton_GG, 1);
		grp->addButton(radioButton_ProAR);
		grp->setId(radioButton_ProAR, 2);

		connect(grp, SIGNAL(buttonClicked(int)), this, SLOT(s_grp_type_cheat(int)));
	}

	lineEdit_Ram->setStyleSheet("QLineEdit{background: #FCD7F8;}");
	lineEdit_GG->setStyleSheet("QLineEdit{background: cyan;}");
	lineEdit_ProAR->setStyleSheet("QLineEdit{background: yellow;}");

	hexSpinBox_Address = new hexSpinBox(this, 4);
	gridLayout_Raw_Value->addWidget(hexSpinBox_Address, 0, 1);

	hexSpinBox_Value = new hexSpinBox(this, 2);
	gridLayout_Raw_Value->addWidget(hexSpinBox_Value, 1, 1);

	hexSpinBox_Compare = new hexSpinBox(this, 2);
	gridLayout_Raw_Value->addWidget(hexSpinBox_Compare, 2, 1);

	QWidget::setTabOrder(lineEdit_ProAR, hexSpinBox_Address);
	QWidget::setTabOrder(hexSpinBox_Address, hexSpinBox_Value);
	QWidget::setTabOrder(hexSpinBox_Value, hexSpinBox_Compare);

	pushButton_Cancel_Cheat->setEnabled(false);

	connect(lineEdit_GG, SIGNAL(textEdited(const QString &)), SLOT(s_line_to_upper(const QString &)));
	connect(checkBox_Compare, SIGNAL(stateChanged(int)), this, SLOT(s_compare(int)));

	connect(pushButton_New_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_new(bool)));
	connect(pushButton_Remove_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_remove(bool)));
	connect(pushButton_Submit_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_submit(bool)));
	connect(pushButton_Cancel_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_cancel(bool)));

	{
		int w = QLabel("0000000000").sizeHint().width() + 10;

		lineEdit_Ram->setFixedWidth(w);
		lineEdit_GG->setFixedWidth(w);
		lineEdit_ProAR->setFixedWidth(w);
	}

	installEventFilter(this);

	populate_cheat_table();

	if (tableWidget_Cheats->rowCount() > 0) {
		tableWidget_Cheats->selectRow(0);
	}

	s_cheat_item();
}
wdgCheatsEditor::~wdgCheatsEditor() {}

void wdgCheatsEditor::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		wdgCheatsEditor::retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}
void wdgCheatsEditor::showEvent(QShowEvent *event) {
	int dim = fontMetrics().height();

	icon_Cheat_List_Editor->setPixmap(QIcon(":/icon/icons/cheats_list.svg").pixmap(dim, dim));
	icon_Editor_Tools->setPixmap(QIcon(":/icon/icons/pencil.svg").pixmap(dim, dim));

	QWidget::showEvent(event);
}

void wdgCheatsEditor::hide_tools_widgets(bool state) {
	if (widget_Edit->isHidden() == state) {
		return;
	}

	if (state == true) {
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
	} else if ((*cheat)["rocky"] != "-") {
		tableWidget_Cheats->item(row, CR_CODE)->setText((*cheat)["rocky"]);
		tableWidget_Cheats->item(row, CR_CODE)->setBackground(COLOR_ROCKY);
	} else {
		tableWidget_Cheats->item(row, CR_CODE)->setText("-");
		tableWidget_Cheats->item(row, CR_CODE)->setBackground(COLOR_MEM);
	}

	tableWidget_Cheats->item(row, CR_ADDRESS)->setText((*cheat)["address"]);
	tableWidget_Cheats->item(row, CR_VALUE)->setText((*cheat)["value"]);
	tableWidget_Cheats->item(row, CR_COMPARE)->setText((*cheat)["compare"]);
	tableWidget_Cheats->item(row, CR_ENABLED_COMPARE)->setText((*cheat)["enabled_compare"]);

	update_color_row(row, (*cheat)["enabled"].toInt() == 1);
}
void wdgCheatsEditor::update_color_row(int row, bool active) {
	QBrush brush = QBrush(QColor::fromRgb(255, 255, 255, 0));
	int i = 0;

	if (active == 1) {
		brush = QBrush(QColor::fromRgb(214, 255, 182, 255));
	}

	for (i = 0; i < tableWidget_Cheats->columnCount(); i++) {
		QTableWidgetItem *item = tableWidget_Cheats->item(row, i);

		if (!item || (i == CR_CODE)) {
			continue;
		}
		item->setBackground(brush);
	}
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

void wdgCheatsEditor::populate_edit_widgets(int row) {
	chl_map cheat;

	if (row < 0) {
		radioButton_CPU_Ram->setChecked(true);
		hexSpinBox_Address->setRange(0, 0x7FFF);

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

		hexSpinBox_Address->setValue(cheat["address"].toInt(&ok, 16));
		hexSpinBox_Value->setValue(cheat["value"].toInt(&ok, 16));
		if (cheat["compare"] == "-") {
			change_active_compare_state(false);
			hexSpinBox_Compare->setValue(0);
		} else {
			change_active_compare_state(true);
			hexSpinBox_Compare->setValue(cheat["compare"].toInt(&ok, 16));
		}
	}
}
void wdgCheatsEditor::clear_edit_widgets(void) {
	lineEdit_Description->setText("");
	lineEdit_GG->setText("");
	lineEdit_ProAR->setText("");
	hexSpinBox_Address->setValue(0);
	hexSpinBox_Value->setValue(0);
	hexSpinBox_Compare->setValue(0);

	change_active_compare_state(false);
}
void wdgCheatsEditor::set_edit_widget(void) {
	if ((new_cheat == true) || (tableWidget_Cheats->currentRow() >= 0)) {
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
	if (new_cheat == true) {
		radioButton_CPU_Ram->setEnabled(true);
		radioButton_GG->setEnabled(true);
		radioButton_ProAR->setEnabled(true);

		radioButton_CPU_Ram->click();
		lineEdit_GG->setText("");
		lineEdit_ProAR->setText("");
		return;
	}

	if ((*cheat)["genie"] != "-") {
		radioButton_CPU_Ram->setEnabled(false);
		radioButton_GG->setEnabled(true);
		radioButton_ProAR->setEnabled(false);

		radioButton_GG->click();
		lineEdit_GG->setText((*cheat)["genie"]);
		lineEdit_ProAR->setText("");
	} else if ((*cheat)["rocky"] != "-") {
		radioButton_CPU_Ram->setEnabled(false);
		radioButton_GG->setEnabled(false);
		radioButton_ProAR->setEnabled(true);

		radioButton_ProAR->click();
		lineEdit_GG->setText("");
		lineEdit_ProAR->setText((*cheat)["rocky"]);
	} else {
		radioButton_CPU_Ram->setEnabled(true);
		radioButton_GG->setEnabled(false);
		radioButton_ProAR->setEnabled(false);

		radioButton_CPU_Ram->click();
		lineEdit_GG->setText("");
		lineEdit_ProAR->setText("");
	}
}
void wdgCheatsEditor::set_edit_buttons(void) {
	if (new_cheat == true) {
		pushButton_New_Cheat->setEnabled(false);
		pushButton_Remove_Cheat->setEnabled(false);
		pushButton_Cancel_Cheat->setEnabled(true);
		pushButton_Submit_Cheat->setEnabled(true);
		return;
	}

	if (tableWidget_Cheats->currentRow() >= 0) {
		pushButton_New_Cheat->setEnabled(true);
		pushButton_Remove_Cheat->setEnabled(true);
		pushButton_Submit_Cheat->setEnabled(true);
		pushButton_Cancel_Cheat->setEnabled(false);
	} else {
		pushButton_New_Cheat->setEnabled(true);
		pushButton_Remove_Cheat->setEnabled(false);
		pushButton_Submit_Cheat->setEnabled(false);
		pushButton_Cancel_Cheat->setEnabled(false);
	}
}
void wdgCheatsEditor::change_active_compare_state(bool state) {
	checkBox_Compare->setChecked(state);

	if (state == true) {
		s_compare(Qt::Checked);
	} else {
		s_compare(Qt::Unchecked);
	}
}

void wdgCheatsEditor::s_table_data_changed(const QModelIndex &topLeft, UNUSED(const QModelIndex &bottomRight),
	UNUSED(const QVector<int> &roles)) {
	if (in_populate_cheat_table == true) {
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
	set_edit_widget();
	set_edit_buttons();
	populate_edit_widgets(tableWidget_Cheats->currentRow());
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
			objch->save_game_cheats();
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
	filters.append(tr("XML files"));
	filters.append(tr("CHT files"));

	filters[0].append(" (*.xml *.XML *.cht *.CHT)");
	filters[1].append(" (*.xml *.XML)");
	filters[2].append(" (*.cht *.CHT)");

	file = QFileDialog::getOpenFileName(this, tr("Import Cheats"), uQString(cfg->last_import_cheat_path), filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().toLower() == "cht") {
			objch->import_CHT(fileinfo.absoluteFilePath());
		} else {
			objch->import_XML(fileinfo.absoluteFilePath());
		}
		populate_cheat_table();
		objch->save_game_cheats();
		umemset(cfg->last_import_cheat_path, 0x00, usizeof(cfg->last_import_cheat_path));
		ustrncpy(cfg->last_import_cheat_path, uQStringCD(fileinfo.absolutePath()), usizeof(cfg->last_import_cheat_path) - 1);
	}
}
void wdgCheatsEditor::s_export(UNUSED(bool checked)) {
	QStringList filters;
	QString file;

	filters.append(tr("XML files"));
	filters.append(tr("All files"));

	filters[0].append(" (*.xml *.XML)");
	filters[1].append(" (*.*)");

	file = QFileDialog::getSaveFileName(this, tr("Export cheats on file"),
		QFileInfo(uQString(info.rom.file)).completeBaseName() + ".xml", filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			fileinfo.setFile(QString(file) + ".xml");
		}

		objch->save_XML(fileinfo.absoluteFilePath());
	}
}
void wdgCheatsEditor::s_clear_all(UNUSED(bool checked)) {
	tableWidget_Cheats->setRowCount(0);
	objch->cheats.clear();
	objch->save_game_cheats();
}

void wdgCheatsEditor::s_grp_type_cheat(int id) {
	if (id == 0) {
		frame_Raw_Value->setEnabled(true);
		hexSpinBox_Address->setRange(0, 0x7FFF);
		hexSpinBox_Address->setValue(0);
	} else {
		frame_Raw_Value->setEnabled(false);
		hexSpinBox_Address->setRange(0x8000, 0xFFFF);
	}

	switch (id) {
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
	}
}
void wdgCheatsEditor::s_line_to_upper(const QString &text) {
	QLineEdit *le = qobject_cast<QLineEdit *>(sender());

	if (!le) {
		return;
	}
	le->setText(text.toUpper());
}
void wdgCheatsEditor::s_compare(int state) {
	if (state == Qt::Checked) {
		hexSpinBox_Compare->setEnabled(true);
	} else {
		hexSpinBox_Compare->setEnabled(false);
	}
}
void wdgCheatsEditor::s_new(UNUSED(bool checked)) {
	new_cheat = true;

	widget_Cheats_List->setEnabled(false);

	clear_edit_widgets();

	set_edit_widget();
	set_edit_buttons();

	set_type_cheat_checkbox(NULL);
}
void wdgCheatsEditor::s_remove(UNUSED(bool checked)) {
	chl_map cheat = extract_cheat_from_row(tableWidget_Cheats->currentRow());
	int i;

	if ((i = objch->find_cheat(&cheat, true)) != -1) {
		objch->cheats.removeAt(i);
	}

	tableWidget_Cheats->removeRow(tableWidget_Cheats->currentRow());
	objch->save_game_cheats();
}
void wdgCheatsEditor::s_submit(UNUSED(bool checked)) {
	int i, current, submitted = true;
	chl_map cheat;
	int type = 0;

	if (lineEdit_Description->text().isEmpty()) {
		QMessageBox::warning(0, tr("Submit warning"), tr("A description must be entered"));
		return;
	}

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
		type = 0;
	} else if (radioButton_GG->isChecked()) {
		cheat.insert("genie", lineEdit_GG->text());
		cheat.insert("rocky", "-");
		objch->complete_gg(&cheat);
		type = 1;
	} else if (radioButton_ProAR->isChecked()) {
		cheat.insert("genie", "-");
		cheat.insert("rocky", lineEdit_ProAR->text());
		objch->complete_rocky(&cheat);
		type = 3;
	}

	if (cheat.count() == 0) {
		QMessageBox::warning(0, tr("Submit warning"), tr("The code is invalid"));
		switch (type) {
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


	if (new_cheat == true) {
		current = tableWidget_Cheats->rowCount();
	} else {
		current = tableWidget_Cheats->currentRow();
	}

	if ((i = objch->find_cheat(&cheat, false)) == -1) {
		if (new_cheat == true) {
			objch->cheats.insert(current, cheat);
			insert_cheat_row(current);
			tableWidget_Cheats->selectRow(current);
		} else {
			objch->cheats.replace(current, cheat);
			update_cheat_row(current, &cheat);
		}
	} else {
		if ((new_cheat == false) && (i == tableWidget_Cheats->currentRow())) {
			objch->cheats.replace(current, cheat);
			update_cheat_row(current, &cheat);
		} else {
			QMessageBox::warning(0, tr("Submit warning"), tr("The cheat is already in the list"));
			submitted = false;
		}
	}

	if (submitted == false) {
		return;
	}

	if (new_cheat == true) {
		s_cancel(false);
	}

	objch->save_game_cheats();
}
void wdgCheatsEditor::s_cancel(UNUSED(bool checked)) {
	new_cheat = false;

	widget_Cheats_List->setEnabled(true);

	s_cheat_item();
}

// ----------------------------------------------------------------------------------------------

hexSpinBox::hexSpinBox(QWidget *parent, int dgts = 4) : QSpinBox(parent) {
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

	validator = new QRegExpValidator(QRegExp("[0-9A-Fa-f]{1,8}"), this);

	installEventFilter(this);
}
hexSpinBox::~hexSpinBox() {}

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
	if (no_prefix == true) {
		return (QString(QString("%1").arg(value, digits, 16, QChar('0')).toUpper()));
	} else {
		return (QString("0x" + QString("%1").arg(value, digits, 16, QChar('0')).toUpper()));
	}
}
int hexSpinBox::valueFromText(const QString &text) const {
	bool ok;

	return (text.toInt(&ok, 16));
}
