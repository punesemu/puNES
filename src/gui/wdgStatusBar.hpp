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

#ifndef WDGSTATUSBAR_HPP_
#define WDGSTATUSBAR_HPP_

#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStatusBar>
#include "mainWindow.hh"
#include "common.h"

class infoStatusBar : public QWidget {
	private:
		QHBoxLayout *hbox;
		QLabel *label;

	public:
		infoStatusBar(QWidget *parent = 0);
		~infoStatusBar();

	public:
		void update_label(void);
};
class recStatusBar : public QFrame {
	Q_OBJECT

	private:
		QLabel *icon;
		QLabel *desc;
		QTimer *timer;

	public:
		recStatusBar(QWidget *parent = 0);
		~recStatusBar();

	signals:
		void et_blink_icon(void);

	protected:
		void changeEvent(QEvent *event);
		void closeEvent(QCloseEvent *event);
		void mousePressEvent(QMouseEvent *event);

	private:
		void desc_text(void);
		void icon_pixmap(QIcon::Mode mode);

	private slots:
		void s_et_blink_icon(void);
		void s_blink_icon(void);
		void s_context_menu(const QPoint& pos);
};
class alignmentStatusBar : public QFrame {
	Q_OBJECT

	private:
		QLabel *label;

	public:
		alignmentStatusBar(QWidget *parent = 0);
		~alignmentStatusBar();

	public:
		void update_label(void);
};
class wdgStatusBar : public QStatusBar {
	public:
		infoStatusBar *info;
		alignmentStatusBar *alg;
		recStatusBar *rec;

	public:
		wdgStatusBar(QWidget *parent);
		~wdgStatusBar();

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
		void showEvent(QShowEvent *event);

	public:
		void update_statusbar(void);
};

#endif /* WDGSTATUSBAR_HPP_ */
