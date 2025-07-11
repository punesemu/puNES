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

#ifndef DLGLOG_HPP_
#define DLGLOG_HPP_

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include "ui_dlgLog.h"
#include "wdgTitleBarWindow.hpp"
#include "common.h"

// ----------------------------------------------------------------------------------------------

class textEditThread final : public QThread {
	Q_OBJECT

	public:
		textEditThread();
		~textEditThread() override;

	protected:
		void run(void) override;

	private slots:
		void time_out(void) const;
};

// ----------------------------------------------------------------------------------------------

class dlgLog final : public QWidget, public Ui::dlgLog {
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

	signals:
		void et_close(void);

	public:
		explicit dlgLog(QWidget *parent = nullptr);
		~dlgLog() override;

	protected:
		void changeEvent(QEvent *event) override;

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
		static void std_err(const QString &string);
		static QString ctrl_special_characters(const QString &input);
};

// ----------------------------------------------------------------------------------------------

class wdgDlgLog final : public wdgTitleBarDialog {
	public:
		dlgLog *wd;

	public:
		explicit wdgDlgLog(QWidget *parent = nullptr);
		~wdgDlgLog() override;

	protected:
		void hideEvent(QHideEvent *event) override;
		void closeEvent(QCloseEvent *event) override;
};

#endif /* DLGLOG_HPP_ */
