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

#ifndef DLGCMDLINEHELP_HPP_
#define DLGCMDLINEHELP_HPP_

#include "ui_dlgCmdLineHelp.h"
#include "wdgTitleBarWindow.hpp"
#include "common.h"

// ----------------------------------------------------------------------------------------------

class dlgCmdLineHelp final : public QWidget, public Ui::dlgCmdLineHelp {
	public:
		explicit dlgCmdLineHelp(QWidget *parent, const QString &title, const uTCHAR *usage_string);
		explicit dlgCmdLineHelp(QWidget *parent = nullptr, const QString &name = "");
		~dlgCmdLineHelp() override;

	private:
		void init(const QString &title, const uTCHAR *usage_string, bool use_html = true);
};

// ----------------------------------------------------------------------------------------------

class wdgDlgCmdLineHelp final : public wdgTitleBarDialog {
	Q_OBJECT

	signals:
		void et_close(void);

	public:
		dlgCmdLineHelp *wd;

	public:
		explicit wdgDlgCmdLineHelp(QWidget *parent, const QString &title, const uTCHAR *usage_string);
		explicit wdgDlgCmdLineHelp(QWidget *parent = nullptr, const QString &name = "");
		~wdgDlgCmdLineHelp() override;

	protected:
		void closeEvent(QCloseEvent *event) override;

	private:
		void init(void);
};

#endif /* DLGCMDLINEHELP_HPP_ */
