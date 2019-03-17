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

#ifndef WDGSTATUSBAR_HPP_
#define WDGSTATUSBAR_HPP_

#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSlider>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QStatusBar>
#include "mainWindow.hh"
#include "wdgRewind.hpp"
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

class slotItemDelegate : public QStyledItemDelegate {
	Q_OBJECT

	public:
		slotItemDelegate(QObject *parent);
		~slotItemDelegate();

	protected:
		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};
class slotComboBox : public QComboBox {
	Q_OBJECT

	private:
		slotItemDelegate *sid;

	public:
		slotComboBox(QWidget *parent = 0);
		~slotComboBox();

	protected:
		void paintEvent(QPaintEvent *event);
};
class wdgState : public QWidget {
	Q_OBJECT

	public:
		QHBoxLayout *hbox;
		QPushButton *save;
		slotComboBox *slot;
		QPushButton *load;
		//QFrame *vline;

	public:
		wdgState(QWidget *parent = 0);
		~wdgState();

	public:
		void retranslateUi(void);

	private slots:
		void s_save_clicked(bool checked);
		void s_slot_activated(int index);
		void s_load_clicked(bool checked);
};

class wdgStatusBar : public QStatusBar {
	Q_OBJECT

	public:
		infoStatusBar *infosb;
		wdgRewind *rewind;
		wdgState *state;
		QFrame *vlineState;
		QFrame *vlineRewind;

	public:
		wdgStatusBar(QWidget *parent);
		~wdgStatusBar();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
		void changeEvent(QEvent *event);

	public:
		void update_statusbar(void);
		void update_width(int w);
		void state_setVisible(bool visible);
		void rewind_setVisible(bool visible);
};

#endif /* WDGSTATUSBAR_HPP_ */
