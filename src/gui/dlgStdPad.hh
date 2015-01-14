/********************************************************************************
** Form generated from reading UI file 'dlgStdPad.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef DLGSTDPAD_H
#define DLGSTDPAD_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Standard_Pad
{
public:
    QGroupBox *groupBox_controller;
    QLabel *image_pad;
    QTabWidget *tabWidget;
    QWidget *tab_kbd;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_kbd_ID;
    QLabel *label_kbd_ID;
    QComboBox *comboBox_kbd_ID;
    QFrame *line_kbd_1;
    QGridLayout *gridLayout_kbd;
    QLabel *label_kbd_Up;
    QLabel *label_kbd_A;
    QLabel *label_kbd_Down;
    QPushButton *pushButton_kbd_unset_Up;
    QPushButton *pushButton_kbd_unset_Start;
    QLabel *label_kbd_Left;
    QLabel *label_kbd_Start;
    QPushButton *pushButton_kbd_Up;
    QPushButton *pushButton_kbd_Start;
    QPushButton *pushButton_kbd_B;
    QLabel *label_kbd_TurboB;
    QPushButton *pushButton_kbd_unset_B;
    QPushButton *pushButton_kbd_TurboB;
    QPushButton *pushButton_kbd_unset_Right;
    QPushButton *pushButton_kbd_Right;
    QPushButton *pushButton_kbd_TurboA;
    QLabel *label_kbd_Select;
    QLabel *label_kbd_TurboA;
    QPushButton *pushButton_kbd_A;
    QPushButton *pushButton_kbd_unset_Down;
    QLabel *label_kbd_B;
    QPushButton *pushButton_kbd_unset_A;
    QPushButton *pushButton_kbd_unset_Left;
    QPushButton *pushButton_kbd_Left;
    QLabel *label_kbd_Right;
    QPushButton *pushButton_kbd_unset_TurboA;
    QPushButton *pushButton_kbd_Select;
    QPushButton *pushButton_kbd_unset_Select;
    QPushButton *pushButton_kbd_unset_TurboB;
    QPushButton *pushButton_kbd_Down;
    QPlainTextEdit *plainTextEdit_kbd_info;
    QFrame *line_kbd_2;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *pushButton_kbd_Sequence;
    QPushButton *pushButton_kbd_Unset_all;
    QPushButton *pushButton_kbd_Defaults;
    QWidget *tab_joy;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_joy_ID;
    QLabel *label_joy_ID;
    QComboBox *comboBox_joy_ID;
    QFrame *line_joy_1;
    QGridLayout *gridLayout_joy;
    QLabel *label_joy_Up;
    QLabel *label_joy_A;
    QLabel *label_joy_Down;
    QPushButton *pushButton_joy_unset_Up;
    QPushButton *pushButton_joy_unset_Start;
    QLabel *label_joy_Left;
    QLabel *label_joy_Start;
    QPushButton *pushButton_joy_Up;
    QPushButton *pushButton_joy_Start;
    QPushButton *pushButton_joy_B;
    QLabel *label_joy_TurboB;
    QPushButton *pushButton_joy_unset_B;
    QPushButton *pushButton_joy_TurboB;
    QPushButton *pushButton_joy_unset_Right;
    QPushButton *pushButton_joy_Right;
    QPushButton *pushButton_joy_TurboA;
    QLabel *label_joy_Select;
    QLabel *label_joy_TurboA;
    QPushButton *pushButton_joy_A;
    QPushButton *pushButton_joy_unset_Down;
    QLabel *label_joy_B;
    QPushButton *pushButton_joy_unset_A;
    QPushButton *pushButton_joy_unset_Left;
    QPushButton *pushButton_joy_Left;
    QLabel *label_joy_Right;
    QPushButton *pushButton_joy_unset_TurboA;
    QPushButton *pushButton_joy_Select;
    QPushButton *pushButton_joy_unset_Select;
    QPushButton *pushButton_joy_unset_TurboB;
    QPushButton *pushButton_joy_Down;
    QPlainTextEdit *plainTextEdit_joy_info;
    QFrame *line_joy_2;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *pushButton_joy_Sequence;
    QPushButton *pushButton_joy_Unset_all;
    QPushButton *pushButton_joy_Defaults;
    QGroupBox *groupBox_Turbo_delay;
    QWidget *gridLayoutWidget_2;
    QGridLayout *gridLayout_Turbo_delay;
    QLabel *label_value_slider_TurboA;
    QSlider *horizontalSlider_TurboA;
    QLabel *label_slider_TurboA;
    QLabel *label_slider_TurboB;
    QSlider *horizontalSlider_TurboB;
    QLabel *label_value_slider_TurboB;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *horizontalLayout_Standard_Button;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_Apply;
    QPushButton *pushButton_Discard;

    void setupUi(QDialog *Standard_Pad)
    {
        if (Standard_Pad->objectName().isEmpty())
            Standard_Pad->setObjectName(QString::fromUtf8("Standard_Pad"));
        Standard_Pad->setWindowModality(Qt::WindowModal);
        Standard_Pad->resize(441, 661);
        groupBox_controller = new QGroupBox(Standard_Pad);
        groupBox_controller->setObjectName(QString::fromUtf8("groupBox_controller"));
        groupBox_controller->setGeometry(QRect(10, 10, 421, 531));
        groupBox_controller->setTitle(QString::fromUtf8("GroupBox"));
        groupBox_controller->setFlat(true);
        image_pad = new QLabel(groupBox_controller);
        image_pad->setObjectName(QString::fromUtf8("image_pad"));
        image_pad->setGeometry(QRect(-2, 20, 426, 178));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(image_pad->sizePolicy().hasHeightForWidth());
        image_pad->setSizePolicy(sizePolicy);
        image_pad->setPixmap(QPixmap(QString::fromUtf8(":/pics/pics/Nes_controller.png")));
        image_pad->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        tabWidget = new QTabWidget(groupBox_controller);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(0, 200, 420, 326));
        tab_kbd = new QWidget();
        tab_kbd->setObjectName(QString::fromUtf8("tab_kbd"));
        verticalLayoutWidget = new QWidget(tab_kbd);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 10, 411, 282));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(6, 0, 1, 0);
        horizontalLayout_kbd_ID = new QHBoxLayout();
        horizontalLayout_kbd_ID->setObjectName(QString::fromUtf8("horizontalLayout_kbd_ID"));
        label_kbd_ID = new QLabel(verticalLayoutWidget);
        label_kbd_ID->setObjectName(QString::fromUtf8("label_kbd_ID"));
        label_kbd_ID->setEnabled(false);
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_kbd_ID->sizePolicy().hasHeightForWidth());
        label_kbd_ID->setSizePolicy(sizePolicy1);
        label_kbd_ID->setMinimumSize(QSize(50, 0));
        label_kbd_ID->setMaximumSize(QSize(50, 16777215));
        label_kbd_ID->setAlignment(Qt::AlignCenter);

        horizontalLayout_kbd_ID->addWidget(label_kbd_ID);

        comboBox_kbd_ID = new QComboBox(verticalLayoutWidget);
        comboBox_kbd_ID->setObjectName(QString::fromUtf8("comboBox_kbd_ID"));
        comboBox_kbd_ID->setEnabled(false);

        horizontalLayout_kbd_ID->addWidget(comboBox_kbd_ID);


        verticalLayout->addLayout(horizontalLayout_kbd_ID);

        line_kbd_1 = new QFrame(verticalLayoutWidget);
        line_kbd_1->setObjectName(QString::fromUtf8("line_kbd_1"));
        line_kbd_1->setFrameShape(QFrame::HLine);
        line_kbd_1->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_kbd_1);

        gridLayout_kbd = new QGridLayout();
        gridLayout_kbd->setObjectName(QString::fromUtf8("gridLayout_kbd"));
        label_kbd_Up = new QLabel(verticalLayoutWidget);
        label_kbd_Up->setObjectName(QString::fromUtf8("label_kbd_Up"));
        sizePolicy1.setHeightForWidth(label_kbd_Up->sizePolicy().hasHeightForWidth());
        label_kbd_Up->setSizePolicy(sizePolicy1);
        label_kbd_Up->setMinimumSize(QSize(50, 0));
        label_kbd_Up->setMaximumSize(QSize(50, 16777215));
        label_kbd_Up->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_Up, 0, 0, 1, 1);

        label_kbd_A = new QLabel(verticalLayoutWidget);
        label_kbd_A->setObjectName(QString::fromUtf8("label_kbd_A"));
        sizePolicy1.setHeightForWidth(label_kbd_A->sizePolicy().hasHeightForWidth());
        label_kbd_A->setSizePolicy(sizePolicy1);
        label_kbd_A->setMinimumSize(QSize(50, 0));
        label_kbd_A->setMaximumSize(QSize(50, 16777215));
        label_kbd_A->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_A, 1, 3, 1, 1);

        label_kbd_Down = new QLabel(verticalLayoutWidget);
        label_kbd_Down->setObjectName(QString::fromUtf8("label_kbd_Down"));
        sizePolicy1.setHeightForWidth(label_kbd_Down->sizePolicy().hasHeightForWidth());
        label_kbd_Down->setSizePolicy(sizePolicy1);
        label_kbd_Down->setMinimumSize(QSize(50, 0));
        label_kbd_Down->setMaximumSize(QSize(50, 16777215));
        label_kbd_Down->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_Down, 1, 0, 1, 1);

        pushButton_kbd_unset_Up = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_Up->setObjectName(QString::fromUtf8("pushButton_kbd_unset_Up"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_Up->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_Up->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_Up->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_Up, 0, 2, 1, 1);

        pushButton_kbd_unset_Start = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_Start->setObjectName(QString::fromUtf8("pushButton_kbd_unset_Start"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_Start->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_Start->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_Start->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_Start, 0, 5, 1, 1);

        label_kbd_Left = new QLabel(verticalLayoutWidget);
        label_kbd_Left->setObjectName(QString::fromUtf8("label_kbd_Left"));
        sizePolicy1.setHeightForWidth(label_kbd_Left->sizePolicy().hasHeightForWidth());
        label_kbd_Left->setSizePolicy(sizePolicy1);
        label_kbd_Left->setMinimumSize(QSize(50, 0));
        label_kbd_Left->setMaximumSize(QSize(50, 16777215));
        label_kbd_Left->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_Left, 2, 0, 1, 1);

        label_kbd_Start = new QLabel(verticalLayoutWidget);
        label_kbd_Start->setObjectName(QString::fromUtf8("label_kbd_Start"));
        sizePolicy1.setHeightForWidth(label_kbd_Start->sizePolicy().hasHeightForWidth());
        label_kbd_Start->setSizePolicy(sizePolicy1);
        label_kbd_Start->setMinimumSize(QSize(50, 0));
        label_kbd_Start->setMaximumSize(QSize(50, 16777215));
        label_kbd_Start->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_Start, 0, 3, 1, 1);

        pushButton_kbd_Up = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_Up->setObjectName(QString::fromUtf8("pushButton_kbd_Up"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_Up->sizePolicy().hasHeightForWidth());
        pushButton_kbd_Up->setSizePolicy(sizePolicy2);
        pushButton_kbd_Up->setMinimumSize(QSize(0, 0));
        pushButton_kbd_Up->setMaximumSize(QSize(160, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_Up, 0, 1, 1, 1);

        pushButton_kbd_Start = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_Start->setObjectName(QString::fromUtf8("pushButton_kbd_Start"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_Start->sizePolicy().hasHeightForWidth());
        pushButton_kbd_Start->setSizePolicy(sizePolicy2);
        pushButton_kbd_Start->setMinimumSize(QSize(0, 0));
        pushButton_kbd_Start->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_Start, 0, 4, 1, 1);

        pushButton_kbd_B = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_B->setObjectName(QString::fromUtf8("pushButton_kbd_B"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_B->sizePolicy().hasHeightForWidth());
        pushButton_kbd_B->setSizePolicy(sizePolicy2);
        pushButton_kbd_B->setMinimumSize(QSize(0, 0));
        pushButton_kbd_B->setMaximumSize(QSize(160, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_B, 2, 4, 1, 1);

        label_kbd_TurboB = new QLabel(verticalLayoutWidget);
        label_kbd_TurboB->setObjectName(QString::fromUtf8("label_kbd_TurboB"));
        sizePolicy1.setHeightForWidth(label_kbd_TurboB->sizePolicy().hasHeightForWidth());
        label_kbd_TurboB->setSizePolicy(sizePolicy1);
        label_kbd_TurboB->setMinimumSize(QSize(50, 0));
        label_kbd_TurboB->setMaximumSize(QSize(50, 16777215));
        label_kbd_TurboB->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_TurboB, 4, 3, 1, 1);

        pushButton_kbd_unset_B = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_B->setObjectName(QString::fromUtf8("pushButton_kbd_unset_B"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_B->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_B->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_B->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_B, 2, 5, 1, 1);

        pushButton_kbd_TurboB = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_TurboB->setObjectName(QString::fromUtf8("pushButton_kbd_TurboB"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_TurboB->sizePolicy().hasHeightForWidth());
        pushButton_kbd_TurboB->setSizePolicy(sizePolicy2);
        pushButton_kbd_TurboB->setMinimumSize(QSize(0, 0));
        pushButton_kbd_TurboB->setMaximumSize(QSize(160, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_TurboB, 4, 4, 1, 1);

        pushButton_kbd_unset_Right = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_Right->setObjectName(QString::fromUtf8("pushButton_kbd_unset_Right"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_Right->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_Right->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_Right->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_Right, 3, 2, 1, 1);

        pushButton_kbd_Right = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_Right->setObjectName(QString::fromUtf8("pushButton_kbd_Right"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_Right->sizePolicy().hasHeightForWidth());
        pushButton_kbd_Right->setSizePolicy(sizePolicy2);
        pushButton_kbd_Right->setMinimumSize(QSize(0, 0));
        pushButton_kbd_Right->setMaximumSize(QSize(160, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_Right, 3, 1, 1, 1);

        pushButton_kbd_TurboA = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_TurboA->setObjectName(QString::fromUtf8("pushButton_kbd_TurboA"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_TurboA->sizePolicy().hasHeightForWidth());
        pushButton_kbd_TurboA->setSizePolicy(sizePolicy2);
        pushButton_kbd_TurboA->setMinimumSize(QSize(0, 0));
        pushButton_kbd_TurboA->setMaximumSize(QSize(160, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_TurboA, 3, 4, 1, 1);

        label_kbd_Select = new QLabel(verticalLayoutWidget);
        label_kbd_Select->setObjectName(QString::fromUtf8("label_kbd_Select"));
        sizePolicy1.setHeightForWidth(label_kbd_Select->sizePolicy().hasHeightForWidth());
        label_kbd_Select->setSizePolicy(sizePolicy1);
        label_kbd_Select->setMinimumSize(QSize(50, 0));
        label_kbd_Select->setMaximumSize(QSize(50, 16777215));
        label_kbd_Select->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_Select, 4, 0, 1, 1);

        label_kbd_TurboA = new QLabel(verticalLayoutWidget);
        label_kbd_TurboA->setObjectName(QString::fromUtf8("label_kbd_TurboA"));
        sizePolicy1.setHeightForWidth(label_kbd_TurboA->sizePolicy().hasHeightForWidth());
        label_kbd_TurboA->setSizePolicy(sizePolicy1);
        label_kbd_TurboA->setMinimumSize(QSize(50, 0));
        label_kbd_TurboA->setMaximumSize(QSize(50, 16777215));
        label_kbd_TurboA->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_TurboA, 3, 3, 1, 1);

        pushButton_kbd_A = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_A->setObjectName(QString::fromUtf8("pushButton_kbd_A"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_A->sizePolicy().hasHeightForWidth());
        pushButton_kbd_A->setSizePolicy(sizePolicy2);
        pushButton_kbd_A->setMinimumSize(QSize(0, 0));
        pushButton_kbd_A->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_A, 1, 4, 1, 1);

        pushButton_kbd_unset_Down = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_Down->setObjectName(QString::fromUtf8("pushButton_kbd_unset_Down"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_Down->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_Down->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_Down->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_Down, 1, 2, 1, 1);

        label_kbd_B = new QLabel(verticalLayoutWidget);
        label_kbd_B->setObjectName(QString::fromUtf8("label_kbd_B"));
        sizePolicy1.setHeightForWidth(label_kbd_B->sizePolicy().hasHeightForWidth());
        label_kbd_B->setSizePolicy(sizePolicy1);
        label_kbd_B->setMinimumSize(QSize(50, 0));
        label_kbd_B->setMaximumSize(QSize(50, 16777215));
        label_kbd_B->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_B, 2, 3, 1, 1);

        pushButton_kbd_unset_A = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_A->setObjectName(QString::fromUtf8("pushButton_kbd_unset_A"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_A->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_A->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_A->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_A, 1, 5, 1, 1);

        pushButton_kbd_unset_Left = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_Left->setObjectName(QString::fromUtf8("pushButton_kbd_unset_Left"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_Left->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_Left->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_Left->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_Left, 2, 2, 1, 1);

        pushButton_kbd_Left = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_Left->setObjectName(QString::fromUtf8("pushButton_kbd_Left"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_Left->sizePolicy().hasHeightForWidth());
        pushButton_kbd_Left->setSizePolicy(sizePolicy2);
        pushButton_kbd_Left->setMinimumSize(QSize(0, 0));
        pushButton_kbd_Left->setMaximumSize(QSize(160, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_Left, 2, 1, 1, 1);

        label_kbd_Right = new QLabel(verticalLayoutWidget);
        label_kbd_Right->setObjectName(QString::fromUtf8("label_kbd_Right"));
        sizePolicy1.setHeightForWidth(label_kbd_Right->sizePolicy().hasHeightForWidth());
        label_kbd_Right->setSizePolicy(sizePolicy1);
        label_kbd_Right->setMinimumSize(QSize(50, 0));
        label_kbd_Right->setMaximumSize(QSize(50, 16777215));
        label_kbd_Right->setAlignment(Qt::AlignCenter);

        gridLayout_kbd->addWidget(label_kbd_Right, 3, 0, 1, 1);

        pushButton_kbd_unset_TurboA = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_TurboA->setObjectName(QString::fromUtf8("pushButton_kbd_unset_TurboA"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_TurboA->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_TurboA->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_TurboA->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_TurboA, 3, 5, 1, 1);

        pushButton_kbd_Select = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_Select->setObjectName(QString::fromUtf8("pushButton_kbd_Select"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_Select->sizePolicy().hasHeightForWidth());
        pushButton_kbd_Select->setSizePolicy(sizePolicy2);
        pushButton_kbd_Select->setMinimumSize(QSize(0, 0));
        pushButton_kbd_Select->setMaximumSize(QSize(160, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_Select, 4, 1, 1, 1);

        pushButton_kbd_unset_Select = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_Select->setObjectName(QString::fromUtf8("pushButton_kbd_unset_Select"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_Select->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_Select->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_Select->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_Select, 4, 2, 1, 1);

        pushButton_kbd_unset_TurboB = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_unset_TurboB->setObjectName(QString::fromUtf8("pushButton_kbd_unset_TurboB"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_unset_TurboB->sizePolicy().hasHeightForWidth());
        pushButton_kbd_unset_TurboB->setSizePolicy(sizePolicy2);
        pushButton_kbd_unset_TurboB->setMaximumSize(QSize(40, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_unset_TurboB, 4, 5, 1, 1);

        pushButton_kbd_Down = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_Down->setObjectName(QString::fromUtf8("pushButton_kbd_Down"));
        sizePolicy2.setHeightForWidth(pushButton_kbd_Down->sizePolicy().hasHeightForWidth());
        pushButton_kbd_Down->setSizePolicy(sizePolicy2);
        pushButton_kbd_Down->setMinimumSize(QSize(0, 0));
        pushButton_kbd_Down->setMaximumSize(QSize(160, 16777215));

        gridLayout_kbd->addWidget(pushButton_kbd_Down, 1, 1, 1, 1);


        verticalLayout->addLayout(gridLayout_kbd);

        plainTextEdit_kbd_info = new QPlainTextEdit(verticalLayoutWidget);
        plainTextEdit_kbd_info->setObjectName(QString::fromUtf8("plainTextEdit_kbd_info"));
        plainTextEdit_kbd_info->setMaximumSize(QSize(16777215, 25));
        plainTextEdit_kbd_info->setFocusPolicy(Qt::NoFocus);
        plainTextEdit_kbd_info->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        plainTextEdit_kbd_info->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        plainTextEdit_kbd_info->setUndoRedoEnabled(false);
        plainTextEdit_kbd_info->setReadOnly(true);
        plainTextEdit_kbd_info->setPlainText(QString::fromUtf8(""));
        plainTextEdit_kbd_info->setTextInteractionFlags(Qt::NoTextInteraction);
        plainTextEdit_kbd_info->setCenterOnScroll(true);

        verticalLayout->addWidget(plainTextEdit_kbd_info);

        line_kbd_2 = new QFrame(verticalLayoutWidget);
        line_kbd_2->setObjectName(QString::fromUtf8("line_kbd_2"));
        line_kbd_2->setFrameShape(QFrame::HLine);
        line_kbd_2->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_kbd_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        pushButton_kbd_Sequence = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_Sequence->setObjectName(QString::fromUtf8("pushButton_kbd_Sequence"));

        horizontalLayout_2->addWidget(pushButton_kbd_Sequence);

        pushButton_kbd_Unset_all = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_Unset_all->setObjectName(QString::fromUtf8("pushButton_kbd_Unset_all"));

        horizontalLayout_2->addWidget(pushButton_kbd_Unset_all);

        pushButton_kbd_Defaults = new QPushButton(verticalLayoutWidget);
        pushButton_kbd_Defaults->setObjectName(QString::fromUtf8("pushButton_kbd_Defaults"));

        horizontalLayout_2->addWidget(pushButton_kbd_Defaults);


        verticalLayout->addLayout(horizontalLayout_2);

        tabWidget->addTab(tab_kbd, QString());
        tab_joy = new QWidget();
        tab_joy->setObjectName(QString::fromUtf8("tab_joy"));
        verticalLayoutWidget_2 = new QWidget(tab_joy);
        verticalLayoutWidget_2->setObjectName(QString::fromUtf8("verticalLayoutWidget_2"));
        verticalLayoutWidget_2->setGeometry(QRect(0, 10, 411, 282));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(6, 0, 1, 0);
        horizontalLayout_joy_ID = new QHBoxLayout();
        horizontalLayout_joy_ID->setObjectName(QString::fromUtf8("horizontalLayout_joy_ID"));
        label_joy_ID = new QLabel(verticalLayoutWidget_2);
        label_joy_ID->setObjectName(QString::fromUtf8("label_joy_ID"));
        sizePolicy1.setHeightForWidth(label_joy_ID->sizePolicy().hasHeightForWidth());
        label_joy_ID->setSizePolicy(sizePolicy1);
        label_joy_ID->setMinimumSize(QSize(50, 0));
        label_joy_ID->setMaximumSize(QSize(50, 16777215));
        label_joy_ID->setAlignment(Qt::AlignCenter);

        horizontalLayout_joy_ID->addWidget(label_joy_ID);

        comboBox_joy_ID = new QComboBox(verticalLayoutWidget_2);
        comboBox_joy_ID->setObjectName(QString::fromUtf8("comboBox_joy_ID"));

        horizontalLayout_joy_ID->addWidget(comboBox_joy_ID);


        verticalLayout_2->addLayout(horizontalLayout_joy_ID);

        line_joy_1 = new QFrame(verticalLayoutWidget_2);
        line_joy_1->setObjectName(QString::fromUtf8("line_joy_1"));
        line_joy_1->setFrameShape(QFrame::HLine);
        line_joy_1->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line_joy_1);

        gridLayout_joy = new QGridLayout();
        gridLayout_joy->setObjectName(QString::fromUtf8("gridLayout_joy"));
        label_joy_Up = new QLabel(verticalLayoutWidget_2);
        label_joy_Up->setObjectName(QString::fromUtf8("label_joy_Up"));
        sizePolicy1.setHeightForWidth(label_joy_Up->sizePolicy().hasHeightForWidth());
        label_joy_Up->setSizePolicy(sizePolicy1);
        label_joy_Up->setMinimumSize(QSize(50, 0));
        label_joy_Up->setMaximumSize(QSize(50, 16777215));
        label_joy_Up->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_Up, 0, 0, 1, 1);

        label_joy_A = new QLabel(verticalLayoutWidget_2);
        label_joy_A->setObjectName(QString::fromUtf8("label_joy_A"));
        sizePolicy1.setHeightForWidth(label_joy_A->sizePolicy().hasHeightForWidth());
        label_joy_A->setSizePolicy(sizePolicy1);
        label_joy_A->setMinimumSize(QSize(50, 0));
        label_joy_A->setMaximumSize(QSize(50, 16777215));
        label_joy_A->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_A, 1, 3, 1, 1);

        label_joy_Down = new QLabel(verticalLayoutWidget_2);
        label_joy_Down->setObjectName(QString::fromUtf8("label_joy_Down"));
        sizePolicy1.setHeightForWidth(label_joy_Down->sizePolicy().hasHeightForWidth());
        label_joy_Down->setSizePolicy(sizePolicy1);
        label_joy_Down->setMinimumSize(QSize(50, 0));
        label_joy_Down->setMaximumSize(QSize(50, 16777215));
        label_joy_Down->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_Down, 1, 0, 1, 1);

        pushButton_joy_unset_Up = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_Up->setObjectName(QString::fromUtf8("pushButton_joy_unset_Up"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_Up->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_Up->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_Up->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_Up, 0, 2, 1, 1);

        pushButton_joy_unset_Start = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_Start->setObjectName(QString::fromUtf8("pushButton_joy_unset_Start"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_Start->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_Start->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_Start->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_Start, 0, 5, 1, 1);

        label_joy_Left = new QLabel(verticalLayoutWidget_2);
        label_joy_Left->setObjectName(QString::fromUtf8("label_joy_Left"));
        sizePolicy1.setHeightForWidth(label_joy_Left->sizePolicy().hasHeightForWidth());
        label_joy_Left->setSizePolicy(sizePolicy1);
        label_joy_Left->setMinimumSize(QSize(50, 0));
        label_joy_Left->setMaximumSize(QSize(50, 16777215));
        label_joy_Left->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_Left, 2, 0, 1, 1);

        label_joy_Start = new QLabel(verticalLayoutWidget_2);
        label_joy_Start->setObjectName(QString::fromUtf8("label_joy_Start"));
        sizePolicy1.setHeightForWidth(label_joy_Start->sizePolicy().hasHeightForWidth());
        label_joy_Start->setSizePolicy(sizePolicy1);
        label_joy_Start->setMinimumSize(QSize(50, 0));
        label_joy_Start->setMaximumSize(QSize(50, 16777215));
        label_joy_Start->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_Start, 0, 3, 1, 1);

        pushButton_joy_Up = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_Up->setObjectName(QString::fromUtf8("pushButton_joy_Up"));
        sizePolicy2.setHeightForWidth(pushButton_joy_Up->sizePolicy().hasHeightForWidth());
        pushButton_joy_Up->setSizePolicy(sizePolicy2);
        pushButton_joy_Up->setMinimumSize(QSize(0, 0));
        pushButton_joy_Up->setMaximumSize(QSize(160, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_Up, 0, 1, 1, 1);

        pushButton_joy_Start = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_Start->setObjectName(QString::fromUtf8("pushButton_joy_Start"));
        sizePolicy2.setHeightForWidth(pushButton_joy_Start->sizePolicy().hasHeightForWidth());
        pushButton_joy_Start->setSizePolicy(sizePolicy2);
        pushButton_joy_Start->setMinimumSize(QSize(0, 0));
        pushButton_joy_Start->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_Start, 0, 4, 1, 1);

        pushButton_joy_B = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_B->setObjectName(QString::fromUtf8("pushButton_joy_B"));
        sizePolicy2.setHeightForWidth(pushButton_joy_B->sizePolicy().hasHeightForWidth());
        pushButton_joy_B->setSizePolicy(sizePolicy2);
        pushButton_joy_B->setMinimumSize(QSize(0, 0));
        pushButton_joy_B->setMaximumSize(QSize(160, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_B, 2, 4, 1, 1);

        label_joy_TurboB = new QLabel(verticalLayoutWidget_2);
        label_joy_TurboB->setObjectName(QString::fromUtf8("label_joy_TurboB"));
        sizePolicy1.setHeightForWidth(label_joy_TurboB->sizePolicy().hasHeightForWidth());
        label_joy_TurboB->setSizePolicy(sizePolicy1);
        label_joy_TurboB->setMinimumSize(QSize(50, 0));
        label_joy_TurboB->setMaximumSize(QSize(50, 16777215));
        label_joy_TurboB->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_TurboB, 4, 3, 1, 1);

        pushButton_joy_unset_B = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_B->setObjectName(QString::fromUtf8("pushButton_joy_unset_B"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_B->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_B->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_B->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_B, 2, 5, 1, 1);

        pushButton_joy_TurboB = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_TurboB->setObjectName(QString::fromUtf8("pushButton_joy_TurboB"));
        sizePolicy2.setHeightForWidth(pushButton_joy_TurboB->sizePolicy().hasHeightForWidth());
        pushButton_joy_TurboB->setSizePolicy(sizePolicy2);
        pushButton_joy_TurboB->setMinimumSize(QSize(0, 0));
        pushButton_joy_TurboB->setMaximumSize(QSize(160, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_TurboB, 4, 4, 1, 1);

        pushButton_joy_unset_Right = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_Right->setObjectName(QString::fromUtf8("pushButton_joy_unset_Right"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_Right->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_Right->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_Right->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_Right, 3, 2, 1, 1);

        pushButton_joy_Right = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_Right->setObjectName(QString::fromUtf8("pushButton_joy_Right"));
        sizePolicy2.setHeightForWidth(pushButton_joy_Right->sizePolicy().hasHeightForWidth());
        pushButton_joy_Right->setSizePolicy(sizePolicy2);
        pushButton_joy_Right->setMinimumSize(QSize(0, 0));
        pushButton_joy_Right->setMaximumSize(QSize(160, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_Right, 3, 1, 1, 1);

        pushButton_joy_TurboA = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_TurboA->setObjectName(QString::fromUtf8("pushButton_joy_TurboA"));
        sizePolicy2.setHeightForWidth(pushButton_joy_TurboA->sizePolicy().hasHeightForWidth());
        pushButton_joy_TurboA->setSizePolicy(sizePolicy2);
        pushButton_joy_TurboA->setMinimumSize(QSize(0, 0));
        pushButton_joy_TurboA->setMaximumSize(QSize(160, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_TurboA, 3, 4, 1, 1);

        label_joy_Select = new QLabel(verticalLayoutWidget_2);
        label_joy_Select->setObjectName(QString::fromUtf8("label_joy_Select"));
        sizePolicy1.setHeightForWidth(label_joy_Select->sizePolicy().hasHeightForWidth());
        label_joy_Select->setSizePolicy(sizePolicy1);
        label_joy_Select->setMinimumSize(QSize(50, 0));
        label_joy_Select->setMaximumSize(QSize(50, 16777215));
        label_joy_Select->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_Select, 4, 0, 1, 1);

        label_joy_TurboA = new QLabel(verticalLayoutWidget_2);
        label_joy_TurboA->setObjectName(QString::fromUtf8("label_joy_TurboA"));
        sizePolicy1.setHeightForWidth(label_joy_TurboA->sizePolicy().hasHeightForWidth());
        label_joy_TurboA->setSizePolicy(sizePolicy1);
        label_joy_TurboA->setMinimumSize(QSize(50, 0));
        label_joy_TurboA->setMaximumSize(QSize(50, 16777215));
        label_joy_TurboA->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_TurboA, 3, 3, 1, 1);

        pushButton_joy_A = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_A->setObjectName(QString::fromUtf8("pushButton_joy_A"));
        sizePolicy2.setHeightForWidth(pushButton_joy_A->sizePolicy().hasHeightForWidth());
        pushButton_joy_A->setSizePolicy(sizePolicy2);
        pushButton_joy_A->setMinimumSize(QSize(0, 0));
        pushButton_joy_A->setMaximumSize(QSize(16777215, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_A, 1, 4, 1, 1);

        pushButton_joy_unset_Down = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_Down->setObjectName(QString::fromUtf8("pushButton_joy_unset_Down"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_Down->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_Down->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_Down->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_Down, 1, 2, 1, 1);

        label_joy_B = new QLabel(verticalLayoutWidget_2);
        label_joy_B->setObjectName(QString::fromUtf8("label_joy_B"));
        sizePolicy1.setHeightForWidth(label_joy_B->sizePolicy().hasHeightForWidth());
        label_joy_B->setSizePolicy(sizePolicy1);
        label_joy_B->setMinimumSize(QSize(50, 0));
        label_joy_B->setMaximumSize(QSize(50, 16777215));
        label_joy_B->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_B, 2, 3, 1, 1);

        pushButton_joy_unset_A = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_A->setObjectName(QString::fromUtf8("pushButton_joy_unset_A"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_A->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_A->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_A->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_A, 1, 5, 1, 1);

        pushButton_joy_unset_Left = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_Left->setObjectName(QString::fromUtf8("pushButton_joy_unset_Left"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_Left->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_Left->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_Left->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_Left, 2, 2, 1, 1);

        pushButton_joy_Left = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_Left->setObjectName(QString::fromUtf8("pushButton_joy_Left"));
        sizePolicy2.setHeightForWidth(pushButton_joy_Left->sizePolicy().hasHeightForWidth());
        pushButton_joy_Left->setSizePolicy(sizePolicy2);
        pushButton_joy_Left->setMinimumSize(QSize(0, 0));
        pushButton_joy_Left->setMaximumSize(QSize(160, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_Left, 2, 1, 1, 1);

        label_joy_Right = new QLabel(verticalLayoutWidget_2);
        label_joy_Right->setObjectName(QString::fromUtf8("label_joy_Right"));
        sizePolicy1.setHeightForWidth(label_joy_Right->sizePolicy().hasHeightForWidth());
        label_joy_Right->setSizePolicy(sizePolicy1);
        label_joy_Right->setMinimumSize(QSize(50, 0));
        label_joy_Right->setMaximumSize(QSize(50, 16777215));
        label_joy_Right->setAlignment(Qt::AlignCenter);

        gridLayout_joy->addWidget(label_joy_Right, 3, 0, 1, 1);

        pushButton_joy_unset_TurboA = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_TurboA->setObjectName(QString::fromUtf8("pushButton_joy_unset_TurboA"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_TurboA->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_TurboA->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_TurboA->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_TurboA, 3, 5, 1, 1);

        pushButton_joy_Select = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_Select->setObjectName(QString::fromUtf8("pushButton_joy_Select"));
        sizePolicy2.setHeightForWidth(pushButton_joy_Select->sizePolicy().hasHeightForWidth());
        pushButton_joy_Select->setSizePolicy(sizePolicy2);
        pushButton_joy_Select->setMinimumSize(QSize(0, 0));
        pushButton_joy_Select->setMaximumSize(QSize(160, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_Select, 4, 1, 1, 1);

        pushButton_joy_unset_Select = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_Select->setObjectName(QString::fromUtf8("pushButton_joy_unset_Select"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_Select->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_Select->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_Select->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_Select, 4, 2, 1, 1);

        pushButton_joy_unset_TurboB = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_unset_TurboB->setObjectName(QString::fromUtf8("pushButton_joy_unset_TurboB"));
        sizePolicy2.setHeightForWidth(pushButton_joy_unset_TurboB->sizePolicy().hasHeightForWidth());
        pushButton_joy_unset_TurboB->setSizePolicy(sizePolicy2);
        pushButton_joy_unset_TurboB->setMaximumSize(QSize(40, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_unset_TurboB, 4, 5, 1, 1);

        pushButton_joy_Down = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_Down->setObjectName(QString::fromUtf8("pushButton_joy_Down"));
        sizePolicy2.setHeightForWidth(pushButton_joy_Down->sizePolicy().hasHeightForWidth());
        pushButton_joy_Down->setSizePolicy(sizePolicy2);
        pushButton_joy_Down->setMinimumSize(QSize(0, 0));
        pushButton_joy_Down->setMaximumSize(QSize(160, 16777215));

        gridLayout_joy->addWidget(pushButton_joy_Down, 1, 1, 1, 1);


        verticalLayout_2->addLayout(gridLayout_joy);

        plainTextEdit_joy_info = new QPlainTextEdit(verticalLayoutWidget_2);
        plainTextEdit_joy_info->setObjectName(QString::fromUtf8("plainTextEdit_joy_info"));
        plainTextEdit_joy_info->setMaximumSize(QSize(16777215, 25));
        plainTextEdit_joy_info->setFocusPolicy(Qt::NoFocus);
        plainTextEdit_joy_info->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        plainTextEdit_joy_info->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        plainTextEdit_joy_info->setUndoRedoEnabled(false);
        plainTextEdit_joy_info->setReadOnly(true);
        plainTextEdit_joy_info->setPlainText(QString::fromUtf8(""));
        plainTextEdit_joy_info->setTextInteractionFlags(Qt::NoTextInteraction);
        plainTextEdit_joy_info->setCenterOnScroll(true);

        verticalLayout_2->addWidget(plainTextEdit_joy_info);

        line_joy_2 = new QFrame(verticalLayoutWidget_2);
        line_joy_2->setObjectName(QString::fromUtf8("line_joy_2"));
        line_joy_2->setFrameShape(QFrame::HLine);
        line_joy_2->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line_joy_2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        pushButton_joy_Sequence = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_Sequence->setObjectName(QString::fromUtf8("pushButton_joy_Sequence"));

        horizontalLayout_4->addWidget(pushButton_joy_Sequence);

        pushButton_joy_Unset_all = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_Unset_all->setObjectName(QString::fromUtf8("pushButton_joy_Unset_all"));

        horizontalLayout_4->addWidget(pushButton_joy_Unset_all);

        pushButton_joy_Defaults = new QPushButton(verticalLayoutWidget_2);
        pushButton_joy_Defaults->setObjectName(QString::fromUtf8("pushButton_joy_Defaults"));

        horizontalLayout_4->addWidget(pushButton_joy_Defaults);


        verticalLayout_2->addLayout(horizontalLayout_4);

        tabWidget->addTab(tab_joy, QString());
        groupBox_Turbo_delay = new QGroupBox(Standard_Pad);
        groupBox_Turbo_delay->setObjectName(QString::fromUtf8("groupBox_Turbo_delay"));
        groupBox_Turbo_delay->setGeometry(QRect(10, 540, 421, 81));
        groupBox_Turbo_delay->setFlat(true);
        gridLayoutWidget_2 = new QWidget(groupBox_Turbo_delay);
        gridLayoutWidget_2->setObjectName(QString::fromUtf8("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(0, 20, 421, 54));
        gridLayout_Turbo_delay = new QGridLayout(gridLayoutWidget_2);
        gridLayout_Turbo_delay->setObjectName(QString::fromUtf8("gridLayout_Turbo_delay"));
        gridLayout_Turbo_delay->setHorizontalSpacing(10);
        gridLayout_Turbo_delay->setContentsMargins(0, 0, 0, 0);
        label_value_slider_TurboA = new QLabel(gridLayoutWidget_2);
        label_value_slider_TurboA->setObjectName(QString::fromUtf8("label_value_slider_TurboA"));
        label_value_slider_TurboA->setText(QString::fromUtf8("00"));

        gridLayout_Turbo_delay->addWidget(label_value_slider_TurboA, 0, 2, 1, 1);

        horizontalSlider_TurboA = new QSlider(gridLayoutWidget_2);
        horizontalSlider_TurboA->setObjectName(QString::fromUtf8("horizontalSlider_TurboA"));
        horizontalSlider_TurboA->setOrientation(Qt::Horizontal);

        gridLayout_Turbo_delay->addWidget(horizontalSlider_TurboA, 0, 1, 1, 1);

        label_slider_TurboA = new QLabel(gridLayoutWidget_2);
        label_slider_TurboA->setObjectName(QString::fromUtf8("label_slider_TurboA"));

        gridLayout_Turbo_delay->addWidget(label_slider_TurboA, 0, 0, 1, 1);

        label_slider_TurboB = new QLabel(gridLayoutWidget_2);
        label_slider_TurboB->setObjectName(QString::fromUtf8("label_slider_TurboB"));

        gridLayout_Turbo_delay->addWidget(label_slider_TurboB, 1, 0, 1, 1);

        horizontalSlider_TurboB = new QSlider(gridLayoutWidget_2);
        horizontalSlider_TurboB->setObjectName(QString::fromUtf8("horizontalSlider_TurboB"));
        horizontalSlider_TurboB->setOrientation(Qt::Horizontal);

        gridLayout_Turbo_delay->addWidget(horizontalSlider_TurboB, 1, 1, 1, 1);

        label_value_slider_TurboB = new QLabel(gridLayoutWidget_2);
        label_value_slider_TurboB->setObjectName(QString::fromUtf8("label_value_slider_TurboB"));
        label_value_slider_TurboB->setText(QString::fromUtf8("00"));

        gridLayout_Turbo_delay->addWidget(label_value_slider_TurboB, 1, 2, 1, 1);

        horizontalLayoutWidget_3 = new QWidget(Standard_Pad);
        horizontalLayoutWidget_3->setObjectName(QString::fromUtf8("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(10, 620, 421, 31));
        horizontalLayout_Standard_Button = new QHBoxLayout(horizontalLayoutWidget_3);
        horizontalLayout_Standard_Button->setObjectName(QString::fromUtf8("horizontalLayout_Standard_Button"));
        horizontalLayout_Standard_Button->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_Standard_Button->addItem(horizontalSpacer);

        pushButton_Apply = new QPushButton(horizontalLayoutWidget_3);
        pushButton_Apply->setObjectName(QString::fromUtf8("pushButton_Apply"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Apply);

        pushButton_Discard = new QPushButton(horizontalLayoutWidget_3);
        pushButton_Discard->setObjectName(QString::fromUtf8("pushButton_Discard"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Discard);

        QWidget::setTabOrder(tabWidget, comboBox_kbd_ID);
        QWidget::setTabOrder(comboBox_kbd_ID, pushButton_kbd_Up);
        QWidget::setTabOrder(pushButton_kbd_Up, pushButton_kbd_unset_Up);
        QWidget::setTabOrder(pushButton_kbd_unset_Up, pushButton_kbd_Down);
        QWidget::setTabOrder(pushButton_kbd_Down, pushButton_kbd_unset_Down);
        QWidget::setTabOrder(pushButton_kbd_unset_Down, pushButton_kbd_Left);
        QWidget::setTabOrder(pushButton_kbd_Left, pushButton_kbd_unset_Left);
        QWidget::setTabOrder(pushButton_kbd_unset_Left, pushButton_kbd_Right);
        QWidget::setTabOrder(pushButton_kbd_Right, pushButton_kbd_unset_Right);
        QWidget::setTabOrder(pushButton_kbd_unset_Right, pushButton_kbd_Select);
        QWidget::setTabOrder(pushButton_kbd_Select, pushButton_kbd_unset_Select);
        QWidget::setTabOrder(pushButton_kbd_unset_Select, pushButton_kbd_Start);
        QWidget::setTabOrder(pushButton_kbd_Start, pushButton_kbd_unset_Start);
        QWidget::setTabOrder(pushButton_kbd_unset_Start, pushButton_kbd_A);
        QWidget::setTabOrder(pushButton_kbd_A, pushButton_kbd_unset_A);
        QWidget::setTabOrder(pushButton_kbd_unset_A, pushButton_kbd_B);
        QWidget::setTabOrder(pushButton_kbd_B, pushButton_kbd_unset_B);
        QWidget::setTabOrder(pushButton_kbd_unset_B, pushButton_kbd_TurboA);
        QWidget::setTabOrder(pushButton_kbd_TurboA, pushButton_kbd_unset_TurboA);
        QWidget::setTabOrder(pushButton_kbd_unset_TurboA, pushButton_kbd_TurboB);
        QWidget::setTabOrder(pushButton_kbd_TurboB, pushButton_kbd_unset_TurboB);
        QWidget::setTabOrder(pushButton_kbd_unset_TurboB, pushButton_kbd_Sequence);
        QWidget::setTabOrder(pushButton_kbd_Sequence, pushButton_kbd_Unset_all);
        QWidget::setTabOrder(pushButton_kbd_Unset_all, pushButton_kbd_Defaults);
        QWidget::setTabOrder(pushButton_kbd_Defaults, horizontalSlider_TurboA);
        QWidget::setTabOrder(horizontalSlider_TurboA, horizontalSlider_TurboB);
        QWidget::setTabOrder(horizontalSlider_TurboB, pushButton_Apply);
        QWidget::setTabOrder(pushButton_Apply, pushButton_Discard);
        QWidget::setTabOrder(pushButton_Discard, comboBox_joy_ID);
        QWidget::setTabOrder(comboBox_joy_ID, pushButton_joy_Up);
        QWidget::setTabOrder(pushButton_joy_Up, pushButton_joy_unset_Up);
        QWidget::setTabOrder(pushButton_joy_unset_Up, pushButton_joy_Down);
        QWidget::setTabOrder(pushButton_joy_Down, pushButton_joy_unset_Down);
        QWidget::setTabOrder(pushButton_joy_unset_Down, pushButton_joy_Left);
        QWidget::setTabOrder(pushButton_joy_Left, pushButton_joy_unset_Left);
        QWidget::setTabOrder(pushButton_joy_unset_Left, pushButton_joy_Right);
        QWidget::setTabOrder(pushButton_joy_Right, pushButton_joy_unset_Right);
        QWidget::setTabOrder(pushButton_joy_unset_Right, pushButton_joy_Select);
        QWidget::setTabOrder(pushButton_joy_Select, pushButton_joy_unset_Select);
        QWidget::setTabOrder(pushButton_joy_unset_Select, pushButton_joy_Start);
        QWidget::setTabOrder(pushButton_joy_Start, pushButton_joy_unset_Start);
        QWidget::setTabOrder(pushButton_joy_unset_Start, pushButton_joy_A);
        QWidget::setTabOrder(pushButton_joy_A, pushButton_joy_unset_A);
        QWidget::setTabOrder(pushButton_joy_unset_A, pushButton_joy_B);
        QWidget::setTabOrder(pushButton_joy_B, pushButton_joy_unset_B);
        QWidget::setTabOrder(pushButton_joy_unset_B, pushButton_joy_TurboA);
        QWidget::setTabOrder(pushButton_joy_TurboA, pushButton_joy_unset_TurboA);
        QWidget::setTabOrder(pushButton_joy_unset_TurboA, pushButton_joy_TurboB);
        QWidget::setTabOrder(pushButton_joy_TurboB, pushButton_joy_unset_TurboB);
        QWidget::setTabOrder(pushButton_joy_unset_TurboB, pushButton_joy_Sequence);
        QWidget::setTabOrder(pushButton_joy_Sequence, pushButton_joy_Unset_all);
        QWidget::setTabOrder(pushButton_joy_Unset_all, pushButton_joy_Defaults);

        retranslateUi(Standard_Pad);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Standard_Pad);
    } // setupUi

    void retranslateUi(QDialog *Standard_Pad)
    {
        Standard_Pad->setWindowTitle(QApplication::translate("Standard_Pad", "Standard Pad", 0, QApplication::UnicodeUTF8));
        image_pad->setText(QString());
        label_kbd_ID->setText(QApplication::translate("Standard_Pad", "ID", 0, QApplication::UnicodeUTF8));
        label_kbd_Up->setText(QApplication::translate("Standard_Pad", "Up", 0, QApplication::UnicodeUTF8));
        label_kbd_A->setText(QApplication::translate("Standard_Pad", "A", 0, QApplication::UnicodeUTF8));
        label_kbd_Down->setText(QApplication::translate("Standard_Pad", "Down", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_Up->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_Start->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        label_kbd_Left->setText(QApplication::translate("Standard_Pad", "Left", 0, QApplication::UnicodeUTF8));
        label_kbd_Start->setText(QApplication::translate("Standard_Pad", "Start", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_Up->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_Start->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_B->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        label_kbd_TurboB->setText(QApplication::translate("Standard_Pad", "Turbo B", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_B->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_TurboB->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_Right->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_Right->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_TurboA->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        label_kbd_Select->setText(QApplication::translate("Standard_Pad", "Select", 0, QApplication::UnicodeUTF8));
        label_kbd_TurboA->setText(QApplication::translate("Standard_Pad", "Turbo A", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_A->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_Down->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        label_kbd_B->setText(QApplication::translate("Standard_Pad", "B", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_A->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_Left->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_Left->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        label_kbd_Right->setText(QApplication::translate("Standard_Pad", "Right", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_TurboA->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_Select->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_Select->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_unset_TurboB->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_Down->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_Sequence->setText(QApplication::translate("Standard_Pad", "In sequence", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_Unset_all->setText(QApplication::translate("Standard_Pad", "Unset all", 0, QApplication::UnicodeUTF8));
        pushButton_kbd_Defaults->setText(QApplication::translate("Standard_Pad", "Defaults", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_kbd), QApplication::translate("Standard_Pad", "Keyboard", 0, QApplication::UnicodeUTF8));
        label_joy_ID->setText(QApplication::translate("Standard_Pad", "ID", 0, QApplication::UnicodeUTF8));
        label_joy_Up->setText(QApplication::translate("Standard_Pad", "Up", 0, QApplication::UnicodeUTF8));
        label_joy_A->setText(QApplication::translate("Standard_Pad", "A", 0, QApplication::UnicodeUTF8));
        label_joy_Down->setText(QApplication::translate("Standard_Pad", "Down", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_Up->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_Start->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        label_joy_Left->setText(QApplication::translate("Standard_Pad", "Left", 0, QApplication::UnicodeUTF8));
        label_joy_Start->setText(QApplication::translate("Standard_Pad", "Start", 0, QApplication::UnicodeUTF8));
        pushButton_joy_Up->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_joy_Start->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_joy_B->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        label_joy_TurboB->setText(QApplication::translate("Standard_Pad", "Turbo B", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_B->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_joy_TurboB->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_Right->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_joy_Right->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_joy_TurboA->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        label_joy_Select->setText(QApplication::translate("Standard_Pad", "Select", 0, QApplication::UnicodeUTF8));
        label_joy_TurboA->setText(QApplication::translate("Standard_Pad", "Turbo A", 0, QApplication::UnicodeUTF8));
        pushButton_joy_A->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_Down->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        label_joy_B->setText(QApplication::translate("Standard_Pad", "B", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_A->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_Left->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_joy_Left->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        label_joy_Right->setText(QApplication::translate("Standard_Pad", "Right", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_TurboA->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_joy_Select->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_Select->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_joy_unset_TurboB->setText(QApplication::translate("Standard_Pad", "unset", 0, QApplication::UnicodeUTF8));
        pushButton_joy_Down->setText(QApplication::translate("Standard_Pad", "PushButton", 0, QApplication::UnicodeUTF8));
        pushButton_joy_Sequence->setText(QApplication::translate("Standard_Pad", "In sequence", 0, QApplication::UnicodeUTF8));
        pushButton_joy_Unset_all->setText(QApplication::translate("Standard_Pad", "Unset all", 0, QApplication::UnicodeUTF8));
        pushButton_joy_Defaults->setText(QApplication::translate("Standard_Pad", "Defaults", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab_joy), QApplication::translate("Standard_Pad", "Joystick", 0, QApplication::UnicodeUTF8));
        groupBox_Turbo_delay->setTitle(QApplication::translate("Standard_Pad", "Turbo Delay", 0, QApplication::UnicodeUTF8));
        label_slider_TurboA->setText(QApplication::translate("Standard_Pad", "Turbo A", 0, QApplication::UnicodeUTF8));
        label_slider_TurboB->setText(QApplication::translate("Standard_Pad", "Turbo B", 0, QApplication::UnicodeUTF8));
        pushButton_Apply->setText(QApplication::translate("Standard_Pad", "Apply", 0, QApplication::UnicodeUTF8));
        pushButton_Discard->setText(QApplication::translate("Standard_Pad", "Discard", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Standard_Pad: public Ui_Standard_Pad {};
} // namespace Ui

QT_END_NAMESPACE

#endif // DLGSTDPAD_H
