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

dlgUncomp::dlgUncomp(QWidget *parent = 0, void *uncompress_archive = NULL,
	BYTE type = UNCOMPRESS_TYPE_ALL) : QDialog(parent) {
	_uncompress_archive *archive = (_uncompress_archive *)uncompress_archive;
	uint32_t index;

	selected = UNCOMPRESS_NO_FILE_SELECTED;

	if (archive == NULL) {
		return;
	}

	setupUi(this);

	setFont(parent->font());

	//tableWidget_Selection->setStyleSheet("QTreeView {selection-background-color: red;}");

	switch (type) {
		case UNCOMPRESS_TYPE_ROM: {
			QTableWidgetItem *header = new QTableWidgetItem(tr("Roms"));

			header->setTextAlignment(Qt::AlignHCenter);
			tableWidget_Selection->setHorizontalHeaderItem(0, header);

			setWindowTitle(tr("which ROM do you want to load?"));
			break;
		}
		case UNCOMPRESS_TYPE_PATCH: {
			QTableWidgetItem *header = new QTableWidgetItem(tr("Patches"));

			header->setTextAlignment(Qt::AlignHCenter);
			tableWidget_Selection->setHorizontalHeaderItem(0, header);

			setWindowTitle(tr("which PATCH do you want to apply?"));
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

	connect(pushButton_Ok, SIGNAL(clicked(bool)), this, SLOT(s_ok_clicked(bool)));
	connect(pushButton_None, SIGNAL(clicked(bool)), this, SLOT(s_none_clicked(bool)));

	QVBoxLayout *vbox = new QVBoxLayout(this);
	vbox->addWidget(tableWidget_Selection);
	vbox->addWidget(horizontalLayoutWidget);

	tableWidget_Selection->setCurrentCell(0, 0);

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
void dlgUncomp::s_none_clicked(bool checked) {
	close();
}
