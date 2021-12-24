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

#ifndef WDGSTATE_HPP_
#define WDGSTATE_HPP_

#include <QtWidgets/QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

class stateBar : public QWidget {
	Q_OBJECT

	private:
		QPainter painter;

	public:
		stateBar(QWidget *parent = 0);
		~stateBar();

	protected:
		QSize sizeHint(void) const;
		bool eventFilter(QObject *obj, QEvent *event);
		void paintEvent(QPaintEvent *event);

	private:
		int slot_at(QPoint pos);
};

#include "wdgState.hh"

class wdgState : public QWidget, public Ui::wdgState {
	Q_OBJECT

	public:
		wdgState(QWidget *parent = 0);
		~wdgState();

	protected:
		void changeEvent(QEvent *event);

	private:
		void retranslateUi(wdgState *wdgState);

	public:
		void update_widget(void);

	private slots:
		void s_slot_actived(void);
		void s_save_clicked(bool checked);
		void s_load_clicked(bool checked);
};

#endif /* WDGSTATE_HPP_ */
