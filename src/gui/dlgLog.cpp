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

#include <QtCore/QTimer>
#include <QtCore/QTextStream>
#include "dlgLog.hpp"
#include "info.h"
#include "gui.h"
#include "conf.h"

void log_info(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->info(txt, ap);
	va_end(ap);
}
void log_info_open(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->lopen(dlgLog::types::LINFO, txt, ap);
	va_end(ap);
}
void log_info_box(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->info_box(txt, ap);
	va_end(ap);
}
void log_info_box_open(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->lopen_box(dlgLog::types::LINFO, txt, ap);
	va_end(ap);
}

void log_warning(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->warning(txt, ap);
	va_end(ap);
}
void log_warning_open(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->lopen(dlgLog::types::LWARNING, txt, ap);
	va_end(ap);
}
void log_warning_box(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->warning_box(txt, ap);
	va_end(ap);
}
void log_warning_box_open(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->lopen_box(dlgLog::types::LWARNING, txt, ap);
	va_end(ap);
}

void log_error(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->error(txt, ap);
	va_end(ap);
}
void log_error_open(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->lopen(dlgLog::types::LERROR, txt, ap);
	va_end(ap);
}
void log_error_box(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->error_box(txt, ap);
	va_end(ap);
}
void log_error_box_open(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->lopen_box(dlgLog::types::LERROR, txt, ap);
	va_end(ap);
}

void log_close(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->lclose(txt, ap);
	va_end(ap);
}
void log_close_box(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->lclose_box(txt, ap);
	va_end(ap);
}

void log_append(const uTCHAR *txt, ...) {
	va_list ap;

	va_start(ap, txt);
	dlglog->wd->lappend(txt, ap);
	va_end(ap);
}
void log_newline(void) {
	dlglog->wd->lnewline();
}

// ----------------------------------------------------------------------------------------------

wdgDlgLog::wdgDlgLog(QWidget *parent) : wdgTitleBarDialog(parent) {
	wd = new dlgLog(this);
	setWindowTitle(wd->windowTitle());
	setWindowIcon(QIcon(":/icon/icons/cheat_editor.svgz"));
	set_border_color(Qt::green);
	set_buttons(barButton::Maximize | barButton::Close);
	add_widget(wd);
	init_geom_variable(cfg->lg_log);

	connect(wd->pushButton_Close, SIGNAL(clicked(bool)), this, SLOT(close(void)));
}
wdgDlgLog::~wdgDlgLog() = default;

void wdgDlgLog::hideEvent(QHideEvent *event) {
	geom = geometry();
	wdgTitleBarDialog::hideEvent(event);
}
void wdgDlgLog::closeEvent(QCloseEvent *event) {
	geom = geometry();
	wdgTitleBarDialog::closeEvent(event);
}

// ----------------------------------------------------------------------------------------------

textEditThread::textEditThread() = default;
textEditThread::~textEditThread() = default;

void textEditThread::run() {
	QTimer::singleShot(0, this, SLOT(time_out()));
	exec();
}

void textEditThread::time_out(void) const {
	dlglog->wd->mutex.lock();
	for (QString &msg : dlglog->wd->messages) {
		dlglog->wd->plainTextEdit_Log->appendHtml(msg);
	}
	dlglog->wd->messages.clear();
	dlglog->wd->mutex.unlock();
	QTimer::singleShot(10, this, SLOT(time_out()));
}

// ----------------------------------------------------------------------------------------------

dlgLog::dlgLog(QWidget *parent) : QWidget(parent) {
	max = 13;

	setupUi(this);

	if (font().pointSize() > 9) {
		QFont font;

		font.setPointSize(9);
		setFont(font);
	}

	{
		QPalette p = plainTextEdit_Log->palette();

		p.setColor(QPalette::Base, Qt::black);
		p.setColor(QPalette::Text, Qt::white);
		plainTextEdit_Log->setPalette(p);
	}
}
dlgLog::~dlgLog() {
	textedit_thread.quit();
	textedit_thread.wait();
};

void dlgLog::changeEvent(QEvent *event) {
	if (event->type() == QEvent::LanguageChange) {
		retranslateUi(this);
	} else {
		QWidget::changeEvent(event);
	}
}

void dlgLog::start_thread(void) {
	textedit_thread.start();
}

void dlgLog::info(const uTCHAR *utxt, va_list ap) {
	print(types::LINFO, utxt, ap);
}
void dlgLog::info_box(const uTCHAR *utxt, va_list ap) {
	print_box(types::LINFO,utxt, ap);
}

void dlgLog::warning(const uTCHAR *utxt, va_list ap) {
	print(types::LWARNING, utxt, ap);
}
void dlgLog::warning_box(const uTCHAR *utxt, va_list ap) {
	print_box(types::LWARNING,utxt, ap);
}

void dlgLog::error(const uTCHAR *utxt, va_list ap) {
	print(types::LERROR, utxt, ap);
}
void dlgLog::error_box(const uTCHAR *utxt, va_list ap) {
	print_box(types::LERROR,utxt, ap);
}

void dlgLog::lopen(const types type, const uTCHAR *utxt, va_list ap) {
	_lopen(type, ">", ">", utxt, ap);
}
void dlgLog::lclose(const uTCHAR *utxt, va_list ap) {
	_lclose(TRUE, utxt, ap);
}

void dlgLog::lopen_box(const types type, const uTCHAR *utxt, va_list ap) {
	_lopen(type, " ", " ", utxt, ap);
}
void dlgLog::lclose_box(const uTCHAR *utxt, va_list ap) {
	_lclose(FALSE, utxt, ap);
}

void dlgLog::lappend(const uTCHAR *utxt, va_list ap) {
	static uTCHAR tmp[1024];

	uvsnprintf(tmp, usizeof(tmp), utxt, ap);
	va_end(ap);

	buffer.tmp += uQString(tmp);
}
void dlgLog::lnewline(void) {
	mutex.lock();
	plainTextEdit_Log->appendHtml("\n");
	std_err("");
	mutex.unlock();
}

void dlgLog::_lopen(const types type, const QString &shtml, const QString &snohtml, const uTCHAR *utxt, va_list ap) {
	mutex.lock();
	buffer.type = type;
	buffer.symbol.html = shtml;
	buffer.symbol.nohtml = snohtml;
	buffer.tmp = "";
	buffer.txt = "";
	if (utxt) {
		lappend(utxt, ap);
	}
	switch (type) {
		default:
		case types::LINFO:
			buffer.color = "\"#99FF00\"";
			break;
		case types::LWARNING:
			buffer.color = "\"#FFFF99\"";
			break;
		case types::LERROR:
			buffer.color = "\"#FF6633\"";
			break;
	}
}
void dlgLog::_lclose(const BYTE color, const uTCHAR *utxt, va_list ap) {
	if (utxt) {
		lappend(utxt, ap);
	}
	extract();
	const QString txt = ctrl_special_characters(buffer.txt);
	const QStringList list = txt.split(':');
	const QString symbol = color
		? QString("<font color=%0>%1").arg(buffer.color, buffer.symbol.html)
		: QString("%1").arg(buffer.symbol.html);
	const QString html = list.count() == 2
		? QString("<pre>%0 %1:%2</font></pre>").arg(symbol, list.at(0), list.at(1))
		: QString("<pre>%0 %1</pre>").arg(symbol, txt);
	messages.append(html);
	std_err(QString("%0 %1").arg(buffer.symbol.nohtml, buffer.txt));
	mutex.unlock();
}
void dlgLog::print(const types type, const uTCHAR *utxt, va_list ap) {
	lopen(type, utxt, ap);
	lclose(nullptr, ap);
}
void dlgLog::print_box(const types type, const uTCHAR *utxt, va_list ap) {
	lopen_box(type, utxt, ap);
	lclose_box(nullptr, ap);
}
void dlgLog::extract(void) {
	if (buffer.tmp.count(';') > 1) {
		int count = 0, index = buffer.tmp.indexOf(';');

		while (index >= 0) {
			if (count > 0) {
				buffer.tmp[index] = ',';
			}
			index = buffer.tmp.indexOf(';', index + 1);
			count++;
		}
	}
	const QStringList list = buffer.tmp.split(';');
	buffer.txt = list.count() >= 2
		? QString("%0 : %1").arg(list.at(0), -max).arg(list.at(1))
		: list.count() == 1
			? QString("%0").arg(list.at(0), -max)
			: buffer.txt = "[NONE]";
}
void dlgLog::std_err(const QString &string) {
	QTextStream err(stderr);

	err << string << "\n";
}
QString dlgLog::ctrl_special_characters(const QString &input) {
	return (QString(input).replace("<", "&lt;").replace(">", "&gt;"));
}
