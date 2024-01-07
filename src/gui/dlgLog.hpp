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

#ifndef DLGLOG_HPP_
#define DLGLOG_HPP_

#include <QtWidgets/QDialog>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include "ui_dlgLog.h"
#include "common.h"

class textEditThread : public QThread {
	Q_OBJECT

	public:
		textEditThread();
		~textEditThread() override;

	protected:
		void run(void) override;

	private slots:
		void time_out(void);
};

class dlgLog : public QDialog, public Ui::dlgLog {
	Q_OBJECT

	public:
		enum types {
			LINFO,
			LWARNING,
			LERROR,
			LNONE
		};

	private:
		struct _buffer {
			types type;
			QString color;
			QString tmp;
			QString txt;
			struct _symbol {
				QString html;
				QString nohtml;
			} symbol;
		} buffer;
		textEditThread textedit_thread;
		int max;

	public:
		QStringList messages;
		QMutex mutex;
		QRect geom;

	public:
		explicit dlgLog(QWidget *parent = nullptr);
		~dlgLog() override;

	protected:
		void changeEvent(QEvent *event) override;
		void hideEvent(QHideEvent *event) override;

	public:
		void start_thread(void);

		void info(const uTCHAR *utxt, va_list ap);
		void info_box(const uTCHAR *utxt, va_list ap);

		void warning(const uTCHAR *utxt, va_list ap);
		void warning_box(const uTCHAR *utxt, va_list ap);

		void error(const uTCHAR *utxt, va_list ap);
		void error_box(const uTCHAR *utxt, va_list ap);

		void lopen(types type, const uTCHAR *utxt, va_list ap);
		void lclose(const uTCHAR *utxt, va_list ap);

		void lopen_box(types type, const uTCHAR *utxt, va_list ap);
		void lclose_box(const uTCHAR *utxt, va_list ap);

		void lappend(const uTCHAR *utxt, va_list ap);
		void lnewline(void);

	private:
		void _lopen(types type, const QString &shtml, const QString &snohtml, const uTCHAR *utxt, va_list ap);
		void _lclose(BYTE color, const uTCHAR *utxt, va_list ap);
		void print(types type, const uTCHAR *utxt, va_list ap);
		void print_box(types type, const uTCHAR *utxt, va_list ap);
		void extract(void);
		void std_err(const QString &string);
		QString ctrl_special_characters(const QString &input);

	private slots:
		void s_close_clicked(bool checked);
};

#endif /* DLGLOG_HPP_ */
