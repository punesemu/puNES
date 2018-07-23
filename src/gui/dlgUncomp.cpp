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

	setWindowTitle(QFileInfo(uQString(info.rom_file)).fileName());

	for (int i = 0; i < uncomp.files_founded; i++) {
		if (uncomp_name_file(&uncomp.file[i]) == EXIT_OK) {
			QTableWidgetItem *item = new QTableWidgetItem(QFileInfo(uQString(uncomp.buffer)).fileName());

			tableWidget_Selection->insertRow(i);
			tableWidget_Selection->setItem(i, 0, item);
		}
	}

	connect(pushButton_Ok, SIGNAL(clicked(bool)), this, SLOT(s_ok_clicked(bool)));
	connect(pushButton_Cancel, SIGNAL(clicked(bool)), this, SLOT(s_cancel_clicked(bool)));

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->addWidget(tableWidget_Selection);
	vbox->addWidget(horizontalLayoutWidget);

	// se l'archivio compresso e' caricato da riga di comando,
	// la gui non e' ancora stata avviata.
	if (gui.start == TRUE) {
		emu_pause(TRUE);
	}

	setAttribute(Qt::WA_DeleteOnClose);
}
dlgUncomp::~dlgUncomp() {}
void dlgUncomp::closeEvent(QCloseEvent *e) {
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
