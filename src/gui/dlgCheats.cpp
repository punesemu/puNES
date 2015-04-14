/*
 * dlgCheats.cpp
 *
 *  Created on: 04/mar/2015
 *      Author: fhorse
 */

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QMessageBox>
#include <QtGui/QCheckBox>
#include <QtGui/QFileDialog>
#else
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFileDialog>
#endif
#include "dlgCheats.moc"
#include "mainWindow.hpp"
#include "gui.h"

#define COLOR_GG    Qt::cyan
#define COLOR_ROCKY Qt::yellow

enum cheat_table_rows {
	CR_ACTIVE,
	CR_CODE,
	CR_ADDRESS,
	CR_VALUE,
	CR_COMPARE,
	CR_DESCRIPTION,
	CR_ENABLED_COMPARE
};

dlgCheats::dlgCheats(QWidget *parent = 0, cheatObject *c = 0) : QDialog(parent) {
	mod = new cheatObject(this);
	org = c;
	new_mode = false;

	for (int i = 0; i < org->cheats.count(); i++) {
		mod->cheats.insert(i, org->cheats.at(i));
	}

	setupUi(this);

	setFont(parent->font());

	setAttribute(Qt::WA_DeleteOnClose);
	setFixedSize(width(), height());

	tableWidget_Cheats->verticalHeader()->setDefaultSectionSize(20);
	tableWidget_Cheats->setColumnWidth(CR_ACTIVE, 60);
	tableWidget_Cheats->setColumnWidth(CR_CODE, 90);
	tableWidget_Cheats->setColumnWidth(CR_ADDRESS, 70);
	tableWidget_Cheats->setColumnWidth(CR_VALUE, 60);
	tableWidget_Cheats->setColumnWidth(CR_COMPARE, 80);
	tableWidget_Cheats->setColumnWidth(CR_DESCRIPTION, 380);
	tableWidget_Cheats->setColumnCount(tableWidget_Cheats->columnCount() + 1);
	tableWidget_Cheats->setColumnHidden(CR_ENABLED_COMPARE, true);
	tableWidget_Cheats->horizontalHeaderItem(CR_DESCRIPTION)->setTextAlignment(Qt::AlignLeft);

	QButtonGroup *grp = new QButtonGroup(this);
	grp->addButton(radioButton_CPU_Ram);
	grp->setId(radioButton_CPU_Ram, 0);
	grp->addButton(radioButton_GG);
	grp->setId(radioButton_GG, 1);
	grp->addButton(radioButton_ProAR);
	grp->setId(radioButton_ProAR, 2);
	connect(grp, SIGNAL(buttonClicked(int)), this, SLOT(s_grp_button_clicked(int)));

	lineEdit_GG->setStyleSheet("QLineEdit{background: cyan;}");
	lineEdit_ProAR->setStyleSheet("QLineEdit{background: yellow;}");

	hexSpinBox_Address = new hexSpinBox(this, 4);
	hexSpinBox_Address->setMinimumWidth(60);
	gridLayout_Raw_Value->addWidget(hexSpinBox_Address, 0, 1);

	hexSpinBox_Value = new hexSpinBox(this, 2);
	hexSpinBox_Value->setMinimumWidth(60);
	gridLayout_Raw_Value->addWidget(hexSpinBox_Value, 1, 1);

	hexSpinBox_Compare = new hexSpinBox(this, 2);
	hexSpinBox_Compare->setMinimumWidth(60);
	gridLayout_Raw_Value->addWidget(hexSpinBox_Compare, 2, 1);

	QWidget::setTabOrder(lineEdit_ProAR, hexSpinBox_Address);
	QWidget::setTabOrder(hexSpinBox_Address, hexSpinBox_Value);
	QWidget::setTabOrder(hexSpinBox_Value, hexSpinBox_Compare);

	pushButton_Cancel_Cheat->setEnabled(false);
	tools_height = groupBox_Edit->height();
	hide_tools_widgets(true);

	connect(tableWidget_Cheats, SIGNAL(itemSelectionChanged()), this,
			SLOT(s_item_selection_changed()));
	connect(pushButton_Import_Cheats, SIGNAL(clicked(bool)), this, SLOT(s_import_clicked(bool)));
	connect(pushButton_Export_Cheats, SIGNAL(clicked(bool)), this, SLOT(s_export_clicked(bool)));
	connect(pushButton_Clear_All_Cheats, SIGNAL(clicked(bool)), this,
			SLOT(s_clear_all_clicked(bool)));

	connect(lineEdit_GG, SIGNAL(textEdited(const QString &)),
			SLOT(s_linedit_to_upper(const QString &)));
	connect(checkBox_Compare, SIGNAL(stateChanged(int)), this,
			SLOT(s_active_compare_state_changed(int)));
	connect(pushButton_New_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_new_clicked(bool)));
	connect(pushButton_Remove_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_remove_clicked(bool)));
	connect(pushButton_Submit_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_submit_clicked(bool)));
	connect(pushButton_Cancel_Cheat, SIGNAL(clicked(bool)), this, SLOT(s_cancel_clicked(bool)));

	connect(pushButton_Hide_Show_Tools, SIGNAL(clicked(bool)), this,
			SLOT(s_hide_show_tools_clicked(bool)));
	connect(pushButton_Apply, SIGNAL(clicked(bool)), this, SLOT(s_apply_clicked(bool)));
	connect(pushButton_Discard, SIGNAL(clicked(bool)), this, SLOT(s_discard_clicked(bool)));

	installEventFilter(this);

	populate_cheat_table();

	if (tableWidget_Cheats->rowCount() > 0) {
		tableWidget_Cheats->selectRow(0);
	}

	s_item_selection_changed();

	emu_pause(TRUE);

	/* disabilito la gestiore del focus della finestra principale */
	gui.main_win_lfp = FALSE;
}
dlgCheats::~dlgCheats() {}
bool dlgCheats::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::Close) {
		org->apply_cheats();

		/* restituisco alla finestra principale la gestione del focus */
		gui.main_win_lfp = TRUE;

		emu_pause(FALSE);
	} else if (event->type() == QEvent::LanguageChange) {
		Cheats::retranslateUi(this);
	}

	return (QObject::eventFilter(obj, event));
}
chl_map dlgCheats::extract_cheat_from_row(int row) {
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

	if (tableWidget_Cheats->item(row, CR_CODE)->backgroundColor() == COLOR_GG) {
		cheat["genie"] = tableWidget_Cheats->item(row, CR_CODE)->text();
	} else if (tableWidget_Cheats->item(row, CR_CODE)->backgroundColor() == COLOR_ROCKY) {
		cheat["rocky"] = tableWidget_Cheats->item(row, CR_CODE)->text();
	}

	return(cheat);
}
void dlgCheats::populate_cheat_table() {
	tableWidget_Cheats->setRowCount(0);

	for (int i = 0; i < mod->cheats.count(); i++) {
		insert_cheat_row(i);
	}
}
void dlgCheats::insert_cheat_row(int row) {
	chl_map cheat = mod->cheats.at(row);
	QTableWidgetItem *col;

	tableWidget_Cheats->insertRow(row);

	{
		QWidget *widget = new QWidget(this);
		QHBoxLayout* layout = new QHBoxLayout(widget);
		QCheckBox *active = new QCheckBox(widget);

		widget->setObjectName(QString("widget%1").arg(row));
		active->setObjectName("active");
		connect(active, SIGNAL(stateChanged(int)), this, SLOT(s_active_state_changed(int)));
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
	col->setTextAlignment(Qt::AlignLeft);
	tableWidget_Cheats->setItem(row, CR_DESCRIPTION, col);

	col = new QTableWidgetItem();
	col->setTextAlignment(Qt::AlignLeft);
	tableWidget_Cheats->setItem(row, CR_ENABLED_COMPARE, col);

	update_cheat_row(row, &cheat);
}
void dlgCheats::update_cheat_row(int row, chl_map *cheat) {
	QCheckBox *active = tableWidget_Cheats->cellWidget(row, CR_ACTIVE)->findChild<QCheckBox*>(
			"active");

	if ((*cheat)["enabled"].toInt() == 1) {
		active->setChecked(true);
		tableWidget_Cheats->item(row, CR_ACTIVE)->setText("1");
	} else {
		active->setChecked(false);
		tableWidget_Cheats->item(row, CR_ACTIVE)->setText("0");
	}

	if ((*cheat)["genie"] != "-") {
		tableWidget_Cheats->item(row, CR_CODE)->setText((*cheat)["genie"]);
		tableWidget_Cheats->item(row, CR_CODE)->setBackgroundColor(COLOR_GG);
	} else if ((*cheat)["rocky"] != "-") {
		tableWidget_Cheats->item(row, CR_CODE)->setText((*cheat)["rocky"]);
		tableWidget_Cheats->item(row, CR_CODE)->setBackgroundColor(COLOR_ROCKY);
	} else {
		tableWidget_Cheats->item(row, CR_CODE)->setText("-");
	}

	tableWidget_Cheats->item(row, CR_ADDRESS)->setText((*cheat)["address"]);
	tableWidget_Cheats->item(row, CR_VALUE)->setText((*cheat)["value"]);
	tableWidget_Cheats->item(row, CR_COMPARE)->setText((*cheat)["compare"]);
	tableWidget_Cheats->item(row, CR_DESCRIPTION)->setText((*cheat)["description"]);
	tableWidget_Cheats->item(row, CR_ENABLED_COMPARE)->setText((*cheat)["enabled_compare"]);
}
void dlgCheats::hide_tools_widgets(bool state) {
	int delta;

	if (groupBox_Edit->isHidden() == state) {
		return;
	}

	if (state == true) {
		delta = -tools_height;
		pushButton_Hide_Show_Tools->setText(tr("Show Tools"));
	} else {
		delta = tools_height;
		pushButton_Hide_Show_Tools->setText(tr("Hide Tools"));
	}

	groupBox_Edit->setHidden(state);
	horizontalLayoutWidget_2->move(horizontalLayoutWidget_2->x(),
			horizontalLayoutWidget_2->y() + delta);
	setFixedHeight(height() + delta);
}
void dlgCheats::populate_edit_widgets(int row) {
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
void dlgCheats::clear_edit_widgets() {
	lineEdit_Description->setText("");
	lineEdit_GG->setText("");
	lineEdit_ProAR->setText("");
	hexSpinBox_Address->setValue(0);
	hexSpinBox_Value->setValue(0);
	hexSpinBox_Compare->setValue(0);

	change_active_compare_state(false);
}
void dlgCheats::set_edit_widget() {
	if ((new_mode == true) || (tableWidget_Cheats->currentRow() >= 0)) {
		horizontalLayoutWidget_6->setEnabled(true);
		frame_Type_Cheat->setEnabled(true);
		frame_Raw_Value->setEnabled(true);
		frame_Buttons->setEnabled(true);
	} else {
		horizontalLayoutWidget_6->setEnabled(false);
		frame_Type_Cheat->setEnabled(false);
		frame_Raw_Value->setEnabled(false);
		frame_Buttons->setEnabled(true);
	}
}
void dlgCheats::set_type_cheat_checkbox(chl_map *cheat) {
	if (new_mode == true) {
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
void dlgCheats::set_edit_buttons() {
	if (new_mode == true) {
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
void dlgCheats::change_active_compare_state(bool state) {
	checkBox_Compare->setChecked(state);

	if (state == true) {
		s_active_compare_state_changed(Qt::Checked);
	} else {
		s_active_compare_state_changed(Qt::Unchecked);
	}
}
void dlgCheats::s_active_compare_state_changed(int state) {
	if (state == Qt::Checked) {
		hexSpinBox_Compare->setEnabled(true);
	} else {
		hexSpinBox_Compare->setEnabled(false);
	}
}
void dlgCheats::s_item_selection_changed() {
	set_edit_widget();
	set_edit_buttons();
	populate_edit_widgets(tableWidget_Cheats->currentRow());
}
void dlgCheats::s_active_state_changed(int state) {
	QWidget *widget;
	int i;

	for (i = 0; i < tableWidget_Cheats->rowCount(); i++) {
		widget = tableWidget_Cheats->cellWidget(i, CR_ACTIVE);

		if (widget->objectName() == ((QObject *)sender())->parent()->objectName()) {
			tableWidget_Cheats->selectRow(i);
			if (state == Qt::Checked) {
				tableWidget_Cheats->item(i, CR_ACTIVE)->setText("1");
			} else {
				tableWidget_Cheats->item(i, CR_ACTIVE)->setText("0");
			}
			break;
		}
	}

	{
		chl_map cheat = extract_cheat_from_row(i);

		if ((i = mod->find_cheat(&cheat, true)) != -1) {
			cheat["enabled"] = tableWidget_Cheats->item(i, CR_ACTIVE)->text();
			mod->cheats.replace(i, cheat);
		}
	}
}
void dlgCheats::s_import_clicked(bool checked) {
	QStringList filters;
	QString file;

	filters.append(tr("All supported formats"));
	filters.append(tr("XML files"));
	filters.append(tr("CHT files"));

	filters[0].append(" (*.xml *.XML *.cht *.CHT)");
	filters[1].append(" (*.xml *.XML)");
	filters[2].append(" (*.cht *.CHT)");

	file = QFileDialog::getOpenFileName(this, tr("Import Cheats"),
			parentMain->last_import_cheat_path, filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().toLower() == "cht") {
			mod->import_CHT(fileinfo.absoluteFilePath());
		} else {
			mod->import_XML(fileinfo.absoluteFilePath());
		}
		populate_cheat_table();
		parentMain->last_import_cheat_path = fileinfo.absolutePath();
	}
}
void dlgCheats::s_export_clicked(bool checked) {
	QStringList filters;
	QString file;

	filters.append(tr("XML files"));
	filters.append(tr("All files"));

	filters[0].append(" (*.xml *.XML)");
	filters[1].append(" (*.*)");

	file = QFileDialog::getSaveFileName(this, tr("Export cheats on file"),
			QFileInfo(info.rom_file).completeBaseName() + ".xml", filters.join(";;"));

	if (file.isNull() == false) {
		QFileInfo fileinfo(file);

		if (fileinfo.suffix().isEmpty()) {
			fileinfo.setFile(QString(file) + ".xml");
		}

		mod->save_XML(fileinfo.absoluteFilePath());
	}
}
void dlgCheats::s_clear_all_clicked(bool checked) {
	tableWidget_Cheats->setRowCount(0);
	mod->cheats.clear();
}
void dlgCheats::s_grp_button_clicked(int id) {
	if (id == 0) {
		frame_Raw_Value->setEnabled(true);
		hexSpinBox_Address->setRange(0, 0x7FFF);
		hexSpinBox_Address->setValue(0);
	} else {
		frame_Raw_Value->setEnabled(false);
		hexSpinBox_Address->setRange(0x8000, 0xFFFF);
	}

	switch(id) {
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
void dlgCheats::s_linedit_to_upper(const QString &text) {
	QLineEdit *le = qobject_cast<QLineEdit *>(sender());

	if (!le) {
		return;
	}
	le->setText(text.toUpper());
}
void dlgCheats::s_new_clicked(bool checked) {
	new_mode = true;

	groupBox_Cheats->setEnabled(false);

	clear_edit_widgets();

	set_edit_widget();
	set_edit_buttons();

	set_type_cheat_checkbox(NULL);
}
void dlgCheats::s_remove_clicked(bool checked) {
	chl_map cheat = extract_cheat_from_row(tableWidget_Cheats->currentRow());
	int i;

	if ((i = mod->find_cheat(&cheat, true)) != -1) {
		mod->cheats.removeAt(i);
	}

	tableWidget_Cheats->removeRow(tableWidget_Cheats->currentRow());
}
void dlgCheats::s_submit_clicked(bool checked) {
	int i, current, submitted = true;
	chl_map cheat;
	int type = 0;

	cheat.insert("description", lineEdit_Description->text());

	if (radioButton_CPU_Ram->isChecked()) {
		cheat.insert("genie", "-");
		cheat.insert("rocky", "-");
		cheat.insert("address", "0x" + QString("%1").arg(hexSpinBox_Address->value(), 4, 16,
				QChar('0')).toUpper());
		cheat.insert("value", "0x" + QString("%1").arg(hexSpinBox_Value->value(), 2, 16,
				QChar('0')).toUpper());
		if (checkBox_Compare->isChecked()) {
			cheat.insert("enabled_compare", "1");
			cheat.insert("compare", "0x" + QString("%1").arg(hexSpinBox_Compare->value(), 2, 16,
					QChar('0')).toUpper());
		} else {
			cheat.insert("enabled_compare", "0");
			cheat.insert("compare", "-");
		}
		type = 0;
	} else if (radioButton_GG->isChecked()) {
		cheat.insert("genie", lineEdit_GG->text());
		cheat.insert("rocky", "-");
		mod->complete_gg(&cheat);
		type = 1;
	} else if (radioButton_ProAR->isChecked()) {
		cheat.insert("genie", "-");
		cheat.insert("rocky", lineEdit_ProAR->text());
		mod->complete_rocky(&cheat);
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


	if (new_mode == true) {
		current = tableWidget_Cheats->rowCount();
	} else {
		current = tableWidget_Cheats->currentRow();
	}

	if ((i = mod->find_cheat(&cheat, false)) == -1) {
		if (new_mode == true) {
			mod->cheats.insert(current, cheat);
			insert_cheat_row(current);
			tableWidget_Cheats->selectRow(current);
		} else {
			mod->cheats.replace(current, cheat);
			update_cheat_row(current, &cheat);
		}
	} else {
		if ((new_mode == false) && (i == tableWidget_Cheats->currentRow())) {
			mod->cheats.replace(current, cheat);
			update_cheat_row(current, &cheat);
		} else {
			QMessageBox::warning(0, tr("Submit warning"), tr("The cheat is already in the list"));
			submitted = false;
		}
	}

	if (submitted == false) {
		return;
	}

	if (new_mode == true) {
		s_cancel_clicked(false);
	}
}
void dlgCheats::s_cancel_clicked(bool checked) {
	new_mode = false;

	groupBox_Cheats->setEnabled(true);

	s_item_selection_changed();
}
void dlgCheats::s_hide_show_tools_clicked(bool checked) {
	hide_tools_widgets(!groupBox_Edit->isHidden());
}
void dlgCheats::s_apply_clicked(bool checked) {
	chl_map cheat;

	org->clear_list();

	for (int i = 0; i < mod->cheats.count(); i++) {
		org->cheats.insert(i, mod->cheats.at(i));
	}

	close();
}
void dlgCheats::s_discard_clicked(bool checked) {
	close();
}

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
