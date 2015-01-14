/********************************************************************************
** Form generated from reading UI file 'dlgUncomp.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef DLGUNCOMP_H
#define DLGUNCOMP_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTableWidget>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Uncompress_selection
{
public:
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout_Standard_Button;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_Ok;
    QPushButton *pushButton_Cancel;
    QTableWidget *tableWidget_Selection;

    void setupUi(QDialog *Uncompress_selection)
    {
        if (Uncompress_selection->objectName().isEmpty())
            Uncompress_selection->setObjectName(QString::fromUtf8("Uncompress_selection"));
        Uncompress_selection->resize(384, 381);
        horizontalLayoutWidget = new QWidget(Uncompress_selection);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(10, 340, 359, 31));
        horizontalLayout_Standard_Button = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout_Standard_Button->setObjectName(QString::fromUtf8("horizontalLayout_Standard_Button"));
        horizontalLayout_Standard_Button->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_Standard_Button->addItem(horizontalSpacer);

        pushButton_Ok = new QPushButton(horizontalLayoutWidget);
        pushButton_Ok->setObjectName(QString::fromUtf8("pushButton_Ok"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(pushButton_Ok->sizePolicy().hasHeightForWidth());
        pushButton_Ok->setSizePolicy(sizePolicy);

        horizontalLayout_Standard_Button->addWidget(pushButton_Ok);

        pushButton_Cancel = new QPushButton(horizontalLayoutWidget);
        pushButton_Cancel->setObjectName(QString::fromUtf8("pushButton_Cancel"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Cancel);

        tableWidget_Selection = new QTableWidget(Uncompress_selection);
        if (tableWidget_Selection->columnCount() < 1)
            tableWidget_Selection->setColumnCount(1);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget_Selection->setHorizontalHeaderItem(0, __qtablewidgetitem);
        tableWidget_Selection->setObjectName(QString::fromUtf8("tableWidget_Selection"));
        tableWidget_Selection->setGeometry(QRect(10, 10, 361, 321));
        tableWidget_Selection->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget_Selection->setSelectionMode(QAbstractItemView::SingleSelection);
        tableWidget_Selection->horizontalHeader()->setStretchLastSection(true);

        retranslateUi(Uncompress_selection);

        QMetaObject::connectSlotsByName(Uncompress_selection);
    } // setupUi

    void retranslateUi(QDialog *Uncompress_selection)
    {
        Uncompress_selection->setWindowTitle(QApplication::translate("Uncompress_selection", "Dialog", 0, QApplication::UnicodeUTF8));
        pushButton_Ok->setText(QApplication::translate("Uncompress_selection", "OK", 0, QApplication::UnicodeUTF8));
        pushButton_Cancel->setText(QApplication::translate("Uncompress_selection", "Cancel", 0, QApplication::UnicodeUTF8));
        QTableWidgetItem *___qtablewidgetitem = tableWidget_Selection->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("Uncompress_selection", "Roms", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Uncompress_selection: public Ui_Uncompress_selection {};
} // namespace Ui

QT_END_NAMESPACE

#endif // DLGUNCOMP_H
