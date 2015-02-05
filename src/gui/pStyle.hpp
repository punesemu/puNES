/*
 * pStyle.hpp
 *
 *  Created on: 05/feb/2015
 *      Author: fhorse
 */

#ifndef PSTYLE_HPP_
#define PSTYLE_HPP_

#include <QtCore/QtGlobal>
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtGui/QProxyStyle>
#else
#include <QtWidgets/QProxyStyle>
#endif

class pStyle: public QProxyStyle {
		Q_OBJECT

	public:
		pStyle();
		~pStyle();

		int styleHint(StyleHint hint, const QStyleOption* opt, const QWidget* widget,
				QStyleHintReturn* returnData) const;
};

#endif /* PSTYLE_HPP_ */
