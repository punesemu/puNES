/*
 *  Copyright (C) 2010-2020 Fabio Cavallo (aka FHorse)
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

#ifndef WDGSTATUSBAR_HPP_
#define WDGSTATUSBAR_HPP_

#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include "mainWindow.hh"
#include "common.h"

class infoStatusBar : public QWidget {
	Q_OBJECT

	private:
		QHBoxLayout *hbox;
		QLabel *label;

	public:
		infoStatusBar(QWidget *parent = 0);
		~infoStatusBar();

	public:
		void update_label(void);
};
class wdgStatusBar : public QStatusBar {
	Q_OBJECT

	public:
		infoStatusBar *infosb;

	public:
		wdgStatusBar(QWidget *parent);
		~wdgStatusBar();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	public:
		void update_statusbar(void);
		void update_width(int w);
};

#endif /* WDGSTATUSBAR_HPP_ */
