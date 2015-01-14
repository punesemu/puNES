/*
 * dlgInput.hpp
 *
 *  Created on: 25/nov/2014
 *      Author: fhorse
 */

#ifndef DLGINPUT_HPP_
#define DLGINPUT_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include "dlgInput.hh"
#include "conf.h"

typedef struct {
	BYTE id;
	_port port;
} _cfg_port;

class dlgInput : public QDialog, public Ui::Input_dialog {
		Q_OBJECT

	private:
		struct _data {
			_config_input settings;
			_cfg_port port[PORT_MAX];
		} data;

	public:
		dlgInput(QWidget *parent);
		~dlgInput();

	private:
		void closeEvent(QCloseEvent *e);
		void update_dialog();
		void combobox_cp_init(QComboBox *cb, _cfg_port *cfg_port);

	private slots:
		void s_combobox_cm_activated(int index);
		void s_combobox_cp_activated(int index);
		void s_setup_clicked(bool checked);
		void s_checkbox_state_changed(int state);
		void s_default_clicked(bool checked);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);
};

#endif /* DLGINPUT_HPP_ */
