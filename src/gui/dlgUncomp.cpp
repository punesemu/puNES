/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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

#include "dlgUncomp.hpp"
#include <QtGui/QStandardItemModel>
#include <QtCore/QFileInfo>
#include "gui.h"

// ----------------------------------------------------------------------------------------------

wdgDlgUncomp::wdgDlgUncomp(QWidget *parent, void *uncompress_archive, BYTE type) : wdgTitleBarDialog(parent) {
	setAttribute(Qt::WA_DeleteOnClose);
	selected = UNCOMPRESS_NO_FILE_SELECTED;
	wd = new dlgUncomp(this, uncompress_archive, type);
	setWindowTitle(wd->windowTitle());
	setWindowIcon(QIcon(":/icon/icons/compressed_file.svgz"));
	set_border_color("gold");
	set_buttons(barButton::Maximize | barButton::Close);
	add_widget(wd);

	connect(wd->tableWidget_Selection, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(s_doubleclick(int,int)));
	connect(wd->pushButton_Ok, SIGNAL(clicked(bool)), this, SLOT(s_ok_clicked(bool)));
	connect(wd->pushButton_None, SIGNAL(clicked(bool)), this, SLOT(s_reject(void)));
}
wdgDlgUncomp::~wdgDlgUncomp() = default;

void wdgDlgUncomp::closeEvent(QCloseEvent *event) {
	if (gui.start) {
		emu_pause(FALSE);
	}
	gui.dlg_rc = selected;
	wdgTitleBarDialog::closeEvent(event);
}

void wdgDlgUncomp::s_doubleclick(int row, UNUSED(int column)) {
	selected = row;
	s_accept();
}
void wdgDlgUncomp::s_ok_clicked(UNUSED(bool checked)) {
	QModelIndexList indexList = wd->tableWidget_Selection->selectionModel()->selectedIndexes();

	selected = indexList.first().row();
	s_accept();
}

// ----------------------------------------------------------------------------------------------

dlgUncomp::dlgUncomp(QWidget *parent, void *uncompress_archive, BYTE type) : QWidget(parent) {
	_uncompress_archive *archive = (_uncompress_archive *)uncompress_archive;
	uint32_t index;

	if (archive == nullptr) {
		return;
	}

	setupUi(this);

	setWindowTitle(QFileInfo(uQString(archive->file)).fileName());

	switch (type) {
		default:
		case UNCOMPRESS_TYPE_ROM: {
			QTableWidgetItem *header = new QTableWidgetItem(tr("which ROM do you want to load?"));

			header->setTextAlignment(Qt::AlignHCenter);
			tableWidget_Selection->setHorizontalHeaderItem(0, header);
			break;
		}
		case UNCOMPRESS_TYPE_FLOPPY_DISK: {
			QTableWidgetItem *header = new QTableWidgetItem(tr("which Floppy Disk image do you want to load?"));

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
		uTCHAR *file;

		if (uncompress_archive_find_item(archive, index, type) == nullptr) {
			continue;
		}

		if ((file = uncompress_archive_file_name(archive, index, type))) {
			QTableWidgetItem *item = new QTableWidgetItem(QFileInfo(uQString(file)).fileName());

			tableWidget_Selection->insertRow((int)index);
			tableWidget_Selection->setItem((int)index, 0, item);
			index++;
		}
	}

	tableWidget_Selection->setCurrentCell(0, 0);

	// se l'archivio compresso e' caricato da riga di comando,
	// la gui non e' ancora stata avviata.
	if (gui.start) {
		emu_pause(TRUE);
	}
}
dlgUncomp::~dlgUncomp() = default;
