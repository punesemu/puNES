/*
 * dlgUncomp.cpp
 *
 *  Created on: 15/dic/2014
 *      Author: fhorse
 */

#include "dlgUncomp.moc"
#include <QtGui/QStandardItemModel>
#include <QtCore/QFileInfo>
#include "uncompress.h"
#if defined (__WIN32__)
#include <libgen.h>
#endif
#include "gui.h"
#include "info.h"

dlgUncomp::dlgUncomp(QWidget *parent = 0) : QDialog(parent) {
	selected = UNCOMP_NO_FILE_SELECTED;

	setupUi(this);

	setFont(parent->font());

	//tableWidget_Selection->setStyleSheet("QTreeView {selection-background-color: red;}");

	setWindowTitle(QFileInfo(info.rom_file).fileName());

	for (int i = 0; i < uncomp.files_founded; i++) {
		if (uncomp_name_file(&uncomp.file[i]) == EXIT_OK) {
			QTableWidgetItem *item = new QTableWidgetItem(basename(uncomp.buffer));

			tableWidget_Selection->insertRow(i);
			tableWidget_Selection->setItem(i, 0, item);
		}
	}

	connect(pushButton_Ok, SIGNAL(clicked(bool)), this, SLOT(s_ok_clicked(bool)));
	connect(pushButton_Cancel, SIGNAL(clicked(bool)), this, SLOT(s_cancel_clicked(bool)));

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->addWidget(tableWidget_Selection);
	vbox->addWidget(horizontalLayoutWidget);

	// disabilito la gestiore del focus della finestra principale
	gui.main_win_lfp = FALSE;

	// se l'archivio compresso e' caricato da riga di comando,
	// la gui non e' ancora stata avviata.
	if (gui.start == TRUE) {
		emu_pause(TRUE);
	}

	setAttribute(Qt::WA_DeleteOnClose);
}
dlgUncomp::~dlgUncomp() {}
void dlgUncomp::closeEvent(QCloseEvent *e) {
	// restituisco alla finestra principale la gestione del focus
	gui.main_win_lfp = TRUE;

	if (gui.start == TRUE) {
		emu_pause(FALSE);
	}

	gui.dlg_rc = selected;

	QDialog::closeEvent(e);
}
void dlgUncomp::s_ok_clicked(bool checked) {
	QModelIndexList indexList = tableWidget_Selection->selectionModel()->selectedIndexes();

	selected = indexList.first().row();
	close();
}
void dlgUncomp::s_cancel_clicked(bool checked) {
	close();
}
