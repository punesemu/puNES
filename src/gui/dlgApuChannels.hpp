/*
 * dlgApuChannels.hpp
 *
 *  Created on: 21/nov/2014
 *      Author: fhorse
 */

#ifndef DLGAPUCHANNELS_HPP_
#define DLGAPUCHANNELS_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include "dlgApuChannels.hh"
#include "conf.h"

class dlgApuChannels : public QDialog, public Ui::APU_channels {
		Q_OBJECT

	private:
		struct _data {
			bool update;
			bool save;
			_config_apu cfg_save;
		} data;

	public:
		dlgApuChannels(QWidget *parent);
		~dlgApuChannels();

	private:
		void closeEvent(QCloseEvent *e);
		void update_dialog();

	private slots:
		void s_checkbox_state_changed(int state);
		void s_slider_value_changed(int value);
		void s_toggle_all_clicked(bool checked);
		void s_apply_clicked(bool checked);
		void s_discard_clicked(bool checked);
};

#endif /* DLGAPUCHANNELS_HPP_ */
