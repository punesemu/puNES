/*
 *  Copyright (C) 2010-2021 Fabio Cavallo (aka FHorse)
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
#if defined (_WIN32)
#include <libgen.h>
#endif
#include "gui.h"

dlgUncomp::dlgUncomp(QWidget *parent, void *uncompress_archive, BYTE type) : QDialog(parent) {
	_uncompress_archive *archive = (_uncompress_archive *)uncompress_archive;
	uint32_t index;

	selected = UNCOMPRESS_NO_FILE_SELECTED;

	if (archive == NULL) {
		return;
	}

	setupUi(this);

	//tableWidget_Selection->setStyleSheet("QTreeView {selection-background-color: red;}");

	setWindowTitle(QFileInfo(uQString(archive->file)).fileName());

	switch (type) {
		case UNCOMPRESS_TYPE_ROM: {
			QTableWidgetItem *header = new QTableWidgetItem(tr("which ROM do you want to load?"));

			header->setTextAlignment(Qt::AlignHCenter);
			tableWidget_Selection->setHorizontalHeaderItem(0, header);
			break;
		}
		case UNCOMPRESS_TYPE_PATCH: {
			QTableWidgetItem *header = new QTableWidgetItem(tr("which PATCH do you want to apply?"));

			header->setTextAlignment(Qt::AlignHCenter);
			tableWidget_Selection->setHorizontalHeaderItem(0, header);
			break;
		}
	}

	index = 0;

	for (unsigned int i = 0; i < uncompress_archive_counter(archive, type); i++) {
		_uncompress_archive_item *aitem;
		uTCHAR *file;

		if ((aitem = uncompress_archive_find_item(archive, index, type)) == NULL) {
			continue;
		}

		if ((file = uncompress_archive_file_name(archive, index, type))) {
			QTableWidgetItem *item = new QTableWidgetItem(QFileInfo(uQString(file)).fileName());

			tableWidget_Selection->insertRow(index);
			tableWidget_Selection->setItem(index, 0, item);
			index++;
		}
	}

	tableWidget_Selection->setCurrentCell(0, 0);

	connect(tableWidget_Selection, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(s_doubleclick(int, int)));
	connect(pushButton_Ok, SIGNAL(clicked(bool)), this, SLOT(s_ok_clicked(bool)));
	connect(pushButton_None, SIGNAL(clicked(bool)), this, SLOT(s_none_clicked(bool)));

	setAttribute(Qt::WA_DeleteOnClose);

	// se l'archivio compresso e' caricato da riga di comando,
	// la gui non e' ancora stata avviata.
	if (gui.start == TRUE) {
		emu_pause(TRUE);
	}
}
dlgUncomp::~dlgUncomp() {}

void dlgUncomp::closeEvent(QCloseEvent *event) {
	if (gui.start == TRUE) {
		emu_pause(FALSE);
	}

	gui.dlg_rc = selected;

	QDialog::closeEvent(event);
}

void dlgUncomp::s_doubleclick(int row, UNUSED(int column)) {
	selected = row;
	close();
}
void dlgUncomp::s_ok_clicked(UNUSED(bool checked)) {
	QModelIndexList indexList = tableWidget_Selection->selectionModel()->selectedIndexes();

	selected = indexList.first().row();
	close();
}
void dlgUncomp::s_none_clicked(UNUSED(bool checked)) {
	close();
}
