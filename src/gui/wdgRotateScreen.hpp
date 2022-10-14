/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
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

#ifndef WDGROTATESCREEN_HPP_
#define WDGROTATESCREEN_HPP_

#include <QtWidgets/QWidget>
#include "ui_wdgRotateScreen.h"

class wdgRotateScreen : public QWidget, public Ui::wdgRotateScreen {
	Q_OBJECT

	public:
		wdgRotateScreen(QWidget *parent = 0);
		~wdgRotateScreen();

	protected:
		void changeEvent(QEvent *event);
		void paintEvent(QPaintEvent *event);

	public:
		void update_widget(void);

	private slots:
		void s_rotate_to_left(bool checked);
		void s_rotate_to_right(bool checked);
		void s_flip(bool checked);
		void s_update_widget(void);
};

#endif /* WDGROTATESCREEN_HPP_ */
