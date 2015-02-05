/*
 * pStyle.cpp
 *
 *  Created on: 05/feb/2015
 *      Author: fhorse
 */

#include "pStyle.moc"

pStyle::pStyle() : QProxyStyle() {}
pStyle::~pStyle() {}
int pStyle::styleHint(StyleHint hint, const QStyleOption* opt = 0, const QWidget* widget = 0,
        QStyleHintReturn* returnData = 0) const {
	if (hint == QStyle::SH_Menu_SloppySubMenus) {
		return (0);
	}
	return (QProxyStyle::styleHint(hint, opt, widget, returnData));
}

