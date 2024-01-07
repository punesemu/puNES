/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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
#include "ui_mainWindow.h"
#include "common.h"

class infoStatusBar : public QWidget {
	private:
		QHBoxLayout *hbox;
		QLabel *label;

	public:
		explicit infoStatusBar(QWidget *parent = nullptr);
		~infoStatusBar() override;

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
		explicit recStatusBar(QWidget *parent = nullptr);
		~recStatusBar() override;

	signals:
		void et_blink_icon(void);

	protected:
		void changeEvent(QEvent *event) override;
		void closeEvent(QCloseEvent *event) override;
		void mousePressEvent(QMouseEvent *event) override;

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
		explicit alignmentStatusBar(QWidget *parent = nullptr);
		~alignmentStatusBar() override;

	public:
		void update_label(void);
};
class nesKeyboardIcon: public QLabel {
	Q_OBJECT

	public:
		explicit nesKeyboardIcon(QWidget *parent = nullptr);
		~nesKeyboardIcon() override;

	Q_SIGNALS:
		void clicked(int button);

	protected:
		void mousePressEvent(QMouseEvent *event) override;
};
class nesKeyboardStatusBar : public QFrame {
	Q_OBJECT

	public:
		nesKeyboardIcon *icon;

	public:
		explicit nesKeyboardStatusBar(QWidget *parent = nullptr);
		~nesKeyboardStatusBar() override;

	protected:
		void changeEvent(QEvent *event) override;

	public:
		void update_tooltip(void);
		void icon_pixmap(QIcon::Mode mode) const;

	private slots:
		void s_clicked(int button);
};
class wdgStatusBar : public QStatusBar {
	public:
		infoStatusBar *info;
		nesKeyboardStatusBar *keyb;
		alignmentStatusBar *alg;
		recStatusBar *rec;

	public:
		explicit wdgStatusBar(QWidget *parent = nullptr);
		~wdgStatusBar() override;

	protected:
		bool eventFilter(QObject *obj, QEvent *event) override;
		void showEvent(QShowEvent *event) override;

	public:
		void update_statusbar(void) const;
};

#endif /* WDGSTATUSBAR_HPP_ */
