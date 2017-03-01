/*
 *  Copyright (C) 2010-2017 Fabio Cavallo (aka FHorse)
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

#include "pStyle.moc"
#include "conf.h"
#include "qt.h"

pStyle::pStyle() : QProxyStyle() {
	newMenuMenagement = false;
	newMenuAllowActiveAndDisabled = false;
}
pStyle::~pStyle() {}
int pStyle::styleHint(StyleHint hint, const QStyleOption* opt = 0, const QWidget* widget = 0,
		QStyleHintReturn* returnData = 0) const {
	if (hint == QStyle::SH_Menu_SloppySubMenus) {
		return (0);
	}
	if (!cfg->disable_new_menu && newMenuMenagement) {
		if (hint == QStyle::SH_Menu_FlashTriggeredItem) {
			return (0);
		}
		if (hint == QStyle::SH_Menu_AllowActiveAndDisabled) {
			if (newMenuAllowActiveAndDisabled) {
				return (1);
			}
		}
	}
	return (QProxyStyle::styleHint(hint, opt, widget, returnData));
}

