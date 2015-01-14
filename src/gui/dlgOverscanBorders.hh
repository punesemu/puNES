/********************************************************************************
** Form generated from reading UI file 'dlgOverscanBorders.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef DLGOVERSCANBORDERS_H
#define DLGOVERSCANBORDERS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Set_borders
{
public:
    QGroupBox *groupBox;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout;
    QComboBox *comboBox_Mode;
    QPushButton *pushButton_Preview;
    QFrame *frame;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout_Up;
    QLabel *label_Up;
    QSpinBox *spinBox_Up;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_Left;
    QLabel *label_Left;
    QSpinBox *spinBox_Left;
    QWidget *verticalLayoutWidget_3;
    QVBoxLayout *verticalLayout_Right;
    QLabel *label_Right;
    QSpinBox *spinBox_Right;
    QWidget *verticalLayoutWidget_4;
    QVBoxLayout *verticalLayout_Down;
    QLabel *label_Down;
    QSpinBox *spinBox_Down;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *horizontalLayout_Standard_Button;
    QPushButton *pushButton_Defaults;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_Apply;
    QPushButton *pushButton_Discard;

    void setupUi(QDialog *Set_borders)
    {
        if (Set_borders->objectName().isEmpty())
            Set_borders->setObjectName(QString::fromUtf8("Set_borders"));
        Set_borders->resize(301, 331);
        groupBox = new QGroupBox(Set_borders);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 281, 271));
        groupBox->setFlat(true);
        horizontalLayoutWidget = new QWidget(groupBox);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(0, 20, 281, 31));
        horizontalLayout = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        comboBox_Mode = new QComboBox(horizontalLayoutWidget);
        comboBox_Mode->setObjectName(QString::fromUtf8("comboBox_Mode"));

        horizontalLayout->addWidget(comboBox_Mode);

        pushButton_Preview = new QPushButton(horizontalLayoutWidget);
        pushButton_Preview->setObjectName(QString::fromUtf8("pushButton_Preview"));

        horizontalLayout->addWidget(pushButton_Preview);

        frame = new QFrame(groupBox);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setGeometry(QRect(0, 60, 281, 211));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayoutWidget = new QWidget(frame);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(100, 10, 81, 52));
        verticalLayout_Up = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout_Up->setObjectName(QString::fromUtf8("verticalLayout_Up"));
        verticalLayout_Up->setContentsMargins(0, 0, 0, 0);
        label_Up = new QLabel(verticalLayoutWidget);
        label_Up->setObjectName(QString::fromUtf8("label_Up"));
        label_Up->setAlignment(Qt::AlignCenter);

        verticalLayout_Up->addWidget(label_Up);

        spinBox_Up = new QSpinBox(verticalLayoutWidget);
        spinBox_Up->setObjectName(QString::fromUtf8("spinBox_Up"));

        verticalLayout_Up->addWidget(spinBox_Up);

        verticalLayoutWidget_2 = new QWidget(frame);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(10, 70, 81, 52));
        verticalLayout_Left = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_Left->setObjectName(QString::fromUtf8("verticalLayout_Left"));
        verticalLayout_Left->setContentsMargins(0, 0, 0, 0);
        label_Left = new QLabel(verticalLayoutWidget_2);
        label_Left->setObjectName(QString::fromUtf8("label_Left"));
        label_Left->setAlignment(Qt::AlignCenter);

        verticalLayout_Left->addWidget(label_Left);

        spinBox_Left = new QSpinBox(verticalLayoutWidget_2);
        spinBox_Left->setObjectName(QString::fromUtf8("spinBox_Left"));

        verticalLayout_Left->addWidget(spinBox_Left);

        verticalLayoutWidget_3 = new QWidget(frame);
        verticalLayoutWidget_3->setObjectName(QString::fromUtf8("verticalLayoutWidget_3"));
        verticalLayoutWidget_3->setGeometry(QRect(190, 70, 81, 52));
        verticalLayout_Right = new QVBoxLayout(verticalLayoutWidget_3);
        verticalLayout_Right->setObjectName(QString::fromUtf8("verticalLayout_Right"));
        verticalLayout_Right->setContentsMargins(0, 0, 0, 0);
        label_Right = new QLabel(verticalLayoutWidget_3);
        label_Right->setObjectName(QString::fromUtf8("label_Right"));
        label_Right->setAlignment(Qt::AlignCenter);

        verticalLayout_Right->addWidget(label_Right);

        spinBox_Right = new QSpinBox(verticalLayoutWidget_3);
        spinBox_Right->setObjectName(QString::fromUtf8("spinBox_Right"));

        verticalLayout_Right->addWidget(spinBox_Right);

        verticalLayoutWidget_4 = new QWidget(frame);
        verticalLayoutWidget_4->setObjectName(QString::fromUtf8("verticalLayoutWidget_4"));
        verticalLayoutWidget_4->setGeometry(QRect(100, 130, 81, 52));
        verticalLayout_Down = new QVBoxLayout(verticalLayoutWidget_4);
        verticalLayout_Down->setObjectName(QString::fromUtf8("verticalLayout_Down"));
        verticalLayout_Down->setContentsMargins(0, 0, 0, 0);
        label_Down = new QLabel(verticalLayoutWidget_4);
        label_Down->setObjectName(QString::fromUtf8("label_Down"));
        label_Down->setAlignment(Qt::AlignCenter);

        verticalLayout_Down->addWidget(label_Down);

        spinBox_Down = new QSpinBox(verticalLayoutWidget_4);
        spinBox_Down->setObjectName(QString::fromUtf8("spinBox_Down"));

        verticalLayout_Down->addWidget(spinBox_Down);

        horizontalLayoutWidget_2 = new QWidget(Set_borders);
        horizontalLayoutWidget_2->setObjectName(QString::fromUtf8("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(10, 290, 281, 31));
        horizontalLayout_Standard_Button = new QHBoxLayout(horizontalLayoutWidget_2);
        horizontalLayout_Standard_Button->setObjectName(QString::fromUtf8("horizontalLayout_Standard_Button"));
        horizontalLayout_Standard_Button->setContentsMargins(0, 0, 0, 0);
        pushButton_Defaults = new QPushButton(horizontalLayoutWidget_2);
        pushButton_Defaults->setObjectName(QString::fromUtf8("pushButton_Defaults"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Defaults);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_Standard_Button->addItem(horizontalSpacer);

        pushButton_Apply = new QPushButton(horizontalLayoutWidget_2);
        pushButton_Apply->setObjectName(QString::fromUtf8("pushButton_Apply"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Apply);

        pushButton_Discard = new QPushButton(horizontalLayoutWidget_2);
        pushButton_Discard->setObjectName(QString::fromUtf8("pushButton_Discard"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Discard);


        retranslateUi(Set_borders);

        QMetaObject::connectSlotsByName(Set_borders);
    } // setupUi

    void retranslateUi(QDialog *Set_borders)
    {
        Set_borders->setWindowTitle(QApplication::translate("Set_borders", "Set borders", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("Set_borders", "Overscan borders", 0, QApplication::UnicodeUTF8));
        pushButton_Preview->setText(QApplication::translate("Set_borders", "Preview", 0, QApplication::UnicodeUTF8));
        label_Up->setText(QApplication::translate("Set_borders", "Up", 0, QApplication::UnicodeUTF8));
        label_Left->setText(QApplication::translate("Set_borders", "Left", 0, QApplication::UnicodeUTF8));
        label_Right->setText(QApplication::translate("Set_borders", "Right", 0, QApplication::UnicodeUTF8));
        label_Down->setText(QApplication::translate("Set_borders", "Down", 0, QApplication::UnicodeUTF8));
        pushButton_Defaults->setText(QApplication::translate("Set_borders", "Defaults", 0, QApplication::UnicodeUTF8));
        pushButton_Apply->setText(QApplication::translate("Set_borders", "Apply", 0, QApplication::UnicodeUTF8));
        pushButton_Discard->setText(QApplication::translate("Set_borders", "Discard", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Set_borders: public Ui_Set_borders {};
} // namespace Ui

QT_END_NAMESPACE

#endif // DLGOVERSCANBORDERS_H
