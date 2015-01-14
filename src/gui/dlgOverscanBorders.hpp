/*
 * dlgOverscanBorders.hpp
 *
 *  Created on: 20/nov/2014
 *      Author: fhorse
 */

#ifndef DLGOVERSCANBORDERS_HPP_
#define DLGOVERSCANBORDERS_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include "dlgOverscanBorders.hh"
#include "overscan.h"

class dlgOverscanBorders : public QDialog, public Ui::Set_borders {
		Q_OBJECT

	private:
		struct _data {
			BYTE save_overscan;

			int mode;

			_overscan_borders save_borders;
			_overscan_borders preview;
			_overscan_borders overscan_borders[2];
			_overscan_borders *borders;
		} data;

	public:
		dlgOverscanBorders(QWidget *parent);
		~dlgOverscanBorders();

	private:
		void closeEvent(QCloseEvent *e);
		void update_dialog();

	private slots:
		void s_combobox_activated(int index);
		void s_preview_clicked(bool checked);
		void s_default_clicked(bool checked);
		void s_spinbox_value_changed(int i);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);
};

#endif /* DLGOVERSCANBORDERS_HPP_ */
