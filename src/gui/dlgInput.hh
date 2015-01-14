/********************************************************************************
** Form generated from reading UI file 'dlgInput.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef DLGINPUT_H
#define DLGINPUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Input_dialog
{
public:
    QGroupBox *groupBox_Controllers;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QComboBox *comboBox_cp1;
    QLabel *label_cp2;
    QPushButton *pushButton_cp1;
    QLabel *label_cp3;
    QLabel *label_cp4;
    QComboBox *comboBox_cp3;
    QPushButton *pushButton_cp3;
    QComboBox *comboBox_cp4;
    QLabel *label_cp1;
    QPushButton *pushButton_cp2;
    QComboBox *comboBox_cp2;
    QPushButton *pushButton_cp4;
    QComboBox *comboBox_cm;
    QLabel *label_cm;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_Standard_Button;
    QPushButton *pushButton_Default;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_Apply;
    QPushButton *pushButton_Discard;
    QGroupBox *groupBox_Misc;
    QCheckBox *checkBox_Permit_updown;

    void setupUi(QDialog *Input_dialog)
    {
        if (Input_dialog->objectName().isEmpty())
            Input_dialog->setObjectName(QString::fromUtf8("Input_dialog"));
        Input_dialog->resize(391, 301);
        groupBox_Controllers = new QGroupBox(Input_dialog);
        groupBox_Controllers->setObjectName(QString::fromUtf8("groupBox_Controllers"));
        groupBox_Controllers->setGeometry(QRect(10, 10, 371, 181));
        groupBox_Controllers->setFlat(true);
        gridLayoutWidget = new QWidget(groupBox_Controllers);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(0, 20, 371, 161));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        comboBox_cp1 = new QComboBox(gridLayoutWidget);
        comboBox_cp1->setObjectName(QString::fromUtf8("comboBox_cp1"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(comboBox_cp1->sizePolicy().hasHeightForWidth());
        comboBox_cp1->setSizePolicy(sizePolicy);

        gridLayout->addWidget(comboBox_cp1, 1, 1, 1, 1);

        label_cp2 = new QLabel(gridLayoutWidget);
        label_cp2->setObjectName(QString::fromUtf8("label_cp2"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_cp2->sizePolicy().hasHeightForWidth());
        label_cp2->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_cp2, 2, 0, 1, 1);

        pushButton_cp1 = new QPushButton(gridLayoutWidget);
        pushButton_cp1->setObjectName(QString::fromUtf8("pushButton_cp1"));

        gridLayout->addWidget(pushButton_cp1, 1, 2, 1, 1);

        label_cp3 = new QLabel(gridLayoutWidget);
        label_cp3->setObjectName(QString::fromUtf8("label_cp3"));
        sizePolicy1.setHeightForWidth(label_cp3->sizePolicy().hasHeightForWidth());
        label_cp3->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_cp3, 3, 0, 1, 1);

        label_cp4 = new QLabel(gridLayoutWidget);
        label_cp4->setObjectName(QString::fromUtf8("label_cp4"));
        sizePolicy1.setHeightForWidth(label_cp4->sizePolicy().hasHeightForWidth());
        label_cp4->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_cp4, 4, 0, 1, 1);

        comboBox_cp3 = new QComboBox(gridLayoutWidget);
        comboBox_cp3->setObjectName(QString::fromUtf8("comboBox_cp3"));
        sizePolicy.setHeightForWidth(comboBox_cp3->sizePolicy().hasHeightForWidth());
        comboBox_cp3->setSizePolicy(sizePolicy);

        gridLayout->addWidget(comboBox_cp3, 3, 1, 1, 1);

        pushButton_cp3 = new QPushButton(gridLayoutWidget);
        pushButton_cp3->setObjectName(QString::fromUtf8("pushButton_cp3"));

        gridLayout->addWidget(pushButton_cp3, 3, 2, 1, 1);

        comboBox_cp4 = new QComboBox(gridLayoutWidget);
        comboBox_cp4->setObjectName(QString::fromUtf8("comboBox_cp4"));
        sizePolicy.setHeightForWidth(comboBox_cp4->sizePolicy().hasHeightForWidth());
        comboBox_cp4->setSizePolicy(sizePolicy);

        gridLayout->addWidget(comboBox_cp4, 4, 1, 1, 1);

        label_cp1 = new QLabel(gridLayoutWidget);
        label_cp1->setObjectName(QString::fromUtf8("label_cp1"));
        sizePolicy1.setHeightForWidth(label_cp1->sizePolicy().hasHeightForWidth());
        label_cp1->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_cp1, 1, 0, 1, 1);

        pushButton_cp2 = new QPushButton(gridLayoutWidget);
        pushButton_cp2->setObjectName(QString::fromUtf8("pushButton_cp2"));

        gridLayout->addWidget(pushButton_cp2, 2, 2, 1, 1);

        comboBox_cp2 = new QComboBox(gridLayoutWidget);
        comboBox_cp2->setObjectName(QString::fromUtf8("comboBox_cp2"));
        sizePolicy.setHeightForWidth(comboBox_cp2->sizePolicy().hasHeightForWidth());
        comboBox_cp2->setSizePolicy(sizePolicy);

        gridLayout->addWidget(comboBox_cp2, 2, 1, 1, 1);

        pushButton_cp4 = new QPushButton(gridLayoutWidget);
        pushButton_cp4->setObjectName(QString::fromUtf8("pushButton_cp4"));

        gridLayout->addWidget(pushButton_cp4, 4, 2, 1, 1);

        comboBox_cm = new QComboBox(gridLayoutWidget);
        comboBox_cm->setObjectName(QString::fromUtf8("comboBox_cm"));
        sizePolicy.setHeightForWidth(comboBox_cm->sizePolicy().hasHeightForWidth());
        comboBox_cm->setSizePolicy(sizePolicy);

        gridLayout->addWidget(comboBox_cm, 0, 1, 1, 1);

        label_cm = new QLabel(gridLayoutWidget);
        label_cm->setObjectName(QString::fromUtf8("label_cm"));
        sizePolicy1.setHeightForWidth(label_cm->sizePolicy().hasHeightForWidth());
        label_cm->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(label_cm, 0, 0, 1, 1);

        horizontalLayoutWidget_2 = new QWidget(Input_dialog);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(10, 260, 371, 31));
        horizontalLayout_Standard_Button = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_Standard_Button->setObjectName(QString::fromUtf8("horizontalLayout_Standard_Button"));
        horizontalLayout_Standard_Button->setContentsMargins(0, 0, 0, 0);
        pushButton_Default = new QPushButton(horizontalLayoutWidget_2);
        pushButton_Default->setObjectName(QString::fromUtf8("pushButton_Default"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Default);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_Standard_Button->addItem(horizontalSpacer);

        pushButton_Apply = new QPushButton(horizontalLayoutWidget_2);
        pushButton_Apply->setObjectName(QString::fromUtf8("pushButton_Apply"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Apply);

        pushButton_Discard = new QPushButton(horizontalLayoutWidget_2);
        pushButton_Discard->setObjectName(QString::fromUtf8("pushButton_Discard"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Discard);

        groupBox_Misc = new QGroupBox(Input_dialog);
        groupBox_Misc->setObjectName(QString::fromUtf8("groupBox_Misc"));
        groupBox_Misc->setGeometry(QRect(10, 190, 371, 61));
        groupBox_Misc->setFlat(true);
        checkBox_Permit_updown = new QCheckBox(groupBox_Misc);
        checkBox_Permit_updown->setObjectName(QString::fromUtf8("checkBox_Permit_updown"));
        checkBox_Permit_updown->setGeometry(QRect(0, 20, 371, 31));

        retranslateUi(Input_dialog);

        QMetaObject::connectSlotsByName(Input_dialog);
    } // setupUi

    void retranslateUi(QDialog *Input_dialog)
    {
        Input_dialog->setWindowTitle(QApplication::translate("Input_dialog", "Input Configuration", 0, QApplication::UnicodeUTF8));
        groupBox_Controllers->setTitle(QApplication::translate("Input_dialog", "Controllers", 0, QApplication::UnicodeUTF8));
        label_cp2->setText(QApplication::translate("Input_dialog", "Controller Port 2", 0, QApplication::UnicodeUTF8));
        pushButton_cp1->setText(QApplication::translate("Input_dialog", "Setup", 0, QApplication::UnicodeUTF8));
        label_cp3->setText(QApplication::translate("Input_dialog", "Controller Port 3", 0, QApplication::UnicodeUTF8));
        label_cp4->setText(QApplication::translate("Input_dialog", "Controller Port 4", 0, QApplication::UnicodeUTF8));
        pushButton_cp3->setText(QApplication::translate("Input_dialog", "Setup", 0, QApplication::UnicodeUTF8));
        label_cp1->setText(QApplication::translate("Input_dialog", "Controller Port 1", 0, QApplication::UnicodeUTF8));
        pushButton_cp2->setText(QApplication::translate("Input_dialog", "Setup", 0, QApplication::UnicodeUTF8));
        pushButton_cp4->setText(QApplication::translate("Input_dialog", "Setup", 0, QApplication::UnicodeUTF8));
        label_cm->setText(QApplication::translate("Input_dialog", "Controller Mode", 0, QApplication::UnicodeUTF8));
        pushButton_Default->setText(QApplication::translate("Input_dialog", "Default", 0, QApplication::UnicodeUTF8));
        pushButton_Apply->setText(QApplication::translate("Input_dialog", "Apply", 0, QApplication::UnicodeUTF8));
        pushButton_Discard->setText(QApplication::translate("Input_dialog", "Discard", 0, QApplication::UnicodeUTF8));
        groupBox_Misc->setTitle(QApplication::translate("Input_dialog", "Misc", 0, QApplication::UnicodeUTF8));
        checkBox_Permit_updown->setText(QApplication::translate("Input_dialog", "Permit \"Up + Down\" and \"Left + Right\" at the same time", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Input_dialog: public Ui_Input_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // DLGINPUT_H
