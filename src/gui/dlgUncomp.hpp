/*
 * dlgUncomp.hpp
 *
 *  Created on: 15/dic/2014
 *      Author: fhorse
 */

#ifndef DLGUNCOMP_HPP_
#define DLGUNCOMP_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include "dlgUncomp.hh"

class dlgUncomp : public QDialog, public Ui::Uncompress_selection {
		Q_OBJECT

	private:
		int selected;

	public:
		dlgUncomp(QWidget *parent);
		~dlgUncomp();

	private:
		void closeEvent(QCloseEvent *e);

	private slots:
		void s_ok_clicked(bool checked);
		void s_cancel_clicked(bool checked);
};

#endif /* DLGUNCOMP_HPP_ */
