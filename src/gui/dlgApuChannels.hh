/********************************************************************************
** Form generated from reading UI file 'dlgApuChannels.ui'
**
** Created by: Qt User Interface Compiler version 4.8.6
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef DLGAPUCHANNELS_H
#define DLGAPUCHANNELS_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_APU_channels
{
public:
    QGroupBox *groupBox;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QSlider *horizontalSlider_DMC;
    QCheckBox *checkBox_Extra;
    QCheckBox *checkBox_Noise;
    QLabel *label_Square1;
    QCheckBox *checkBox_Triangle;
    QLabel *label_DMC;
    QCheckBox *checkBox_Master;
    QFrame *line_Master;
    QFrame *line_Extra;
    QLabel *label_Noise;
    QFrame *line_Noise;
    QSlider *horizontalSlider_Master;
    QLabel *label_Square2;
    QLabel *label_Extra;
    QLabel *label_Master;
    QLabel *label_Triangle;
    QFrame *line_Square1;
    QSlider *horizontalSlider_Square1;
    QCheckBox *checkBox_Square1;
    QSlider *horizontalSlider_Noise;
    QFrame *line_Square2;
    QSlider *horizontalSlider_Extra;
    QCheckBox *checkBox_DMC;
    QSlider *horizontalSlider_Triangle;
    QFrame *line_Triangle;
    QFrame *line_DMC;
    QCheckBox *checkBox_Square2;
    QSlider *horizontalSlider_Square2;
    QLabel *label;
    QLabel *label_2;
    QFrame *line;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_Active_all;
    QPushButton *pushButton_Disable_all;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *horizontalLayout_Standard_Button;
    QPushButton *pushButton_Defaults;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton_Apply;
    QPushButton *pushButton_Discard;

    void setupUi(QDialog *APU_channels)
    {
        if (APU_channels->objectName().isEmpty())
            APU_channels->setObjectName(QString::fromUtf8("APU_channels"));
        APU_channels->resize(311, 331);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(APU_channels->sizePolicy().hasHeightForWidth());
        APU_channels->setSizePolicy(sizePolicy);
        groupBox = new QGroupBox(APU_channels);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(10, 10, 291, 261));
        groupBox->setFlat(true);
        verticalLayoutWidget = new QWidget(groupBox);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(0, 20, 291, 242));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setVerticalSpacing(2);
        horizontalSlider_DMC = new QSlider(verticalLayoutWidget);
        horizontalSlider_DMC->setObjectName(QString::fromUtf8("horizontalSlider_DMC"));
        horizontalSlider_DMC->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_DMC, 6, 3, 1, 1);

        checkBox_Extra = new QCheckBox(verticalLayoutWidget);
        checkBox_Extra->setObjectName(QString::fromUtf8("checkBox_Extra"));
        checkBox_Extra->setLayoutDirection(Qt::RightToLeft);
        checkBox_Extra->setText(QString::fromUtf8(""));

        gridLayout->addWidget(checkBox_Extra, 7, 1, 1, 1);

        checkBox_Noise = new QCheckBox(verticalLayoutWidget);
        checkBox_Noise->setObjectName(QString::fromUtf8("checkBox_Noise"));
        checkBox_Noise->setLayoutDirection(Qt::RightToLeft);
        checkBox_Noise->setText(QString::fromUtf8(""));

        gridLayout->addWidget(checkBox_Noise, 5, 1, 1, 1);

        label_Square1 = new QLabel(verticalLayoutWidget);
        label_Square1->setObjectName(QString::fromUtf8("label_Square1"));
        label_Square1->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_Square1, 2, 0, 1, 1);

        checkBox_Triangle = new QCheckBox(verticalLayoutWidget);
        checkBox_Triangle->setObjectName(QString::fromUtf8("checkBox_Triangle"));
        checkBox_Triangle->setLayoutDirection(Qt::RightToLeft);
        checkBox_Triangle->setText(QString::fromUtf8(""));

        gridLayout->addWidget(checkBox_Triangle, 4, 1, 1, 1);

        label_DMC = new QLabel(verticalLayoutWidget);
        label_DMC->setObjectName(QString::fromUtf8("label_DMC"));
        label_DMC->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_DMC, 6, 0, 1, 1);

        checkBox_Master = new QCheckBox(verticalLayoutWidget);
        checkBox_Master->setObjectName(QString::fromUtf8("checkBox_Master"));
        checkBox_Master->setLayoutDirection(Qt::RightToLeft);
        checkBox_Master->setText(QString::fromUtf8(""));

        gridLayout->addWidget(checkBox_Master, 1, 1, 1, 1);

        line_Master = new QFrame(verticalLayoutWidget);
        line_Master->setObjectName(QString::fromUtf8("line_Master"));
        line_Master->setFrameShadow(QFrame::Sunken);
        line_Master->setFrameShape(QFrame::VLine);

        gridLayout->addWidget(line_Master, 1, 2, 1, 1);

        line_Extra = new QFrame(verticalLayoutWidget);
        line_Extra->setObjectName(QString::fromUtf8("line_Extra"));
        line_Extra->setFrameShape(QFrame::VLine);
        line_Extra->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_Extra, 7, 2, 1, 1);

        label_Noise = new QLabel(verticalLayoutWidget);
        label_Noise->setObjectName(QString::fromUtf8("label_Noise"));
        label_Noise->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_Noise, 5, 0, 1, 1);

        line_Noise = new QFrame(verticalLayoutWidget);
        line_Noise->setObjectName(QString::fromUtf8("line_Noise"));
        line_Noise->setFrameShape(QFrame::VLine);
        line_Noise->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_Noise, 5, 2, 1, 1);

        horizontalSlider_Master = new QSlider(verticalLayoutWidget);
        horizontalSlider_Master->setObjectName(QString::fromUtf8("horizontalSlider_Master"));
        horizontalSlider_Master->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Master, 1, 3, 1, 1);

        label_Square2 = new QLabel(verticalLayoutWidget);
        label_Square2->setObjectName(QString::fromUtf8("label_Square2"));
        label_Square2->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_Square2, 3, 0, 1, 1);

        label_Extra = new QLabel(verticalLayoutWidget);
        label_Extra->setObjectName(QString::fromUtf8("label_Extra"));
        label_Extra->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_Extra, 7, 0, 1, 1);

        label_Master = new QLabel(verticalLayoutWidget);
        label_Master->setObjectName(QString::fromUtf8("label_Master"));
        label_Master->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_Master, 1, 0, 1, 1);

        label_Triangle = new QLabel(verticalLayoutWidget);
        label_Triangle->setObjectName(QString::fromUtf8("label_Triangle"));
        label_Triangle->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_Triangle, 4, 0, 1, 1);

        line_Square1 = new QFrame(verticalLayoutWidget);
        line_Square1->setObjectName(QString::fromUtf8("line_Square1"));
        line_Square1->setFrameShape(QFrame::VLine);
        line_Square1->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_Square1, 2, 2, 1, 1);

        horizontalSlider_Square1 = new QSlider(verticalLayoutWidget);
        horizontalSlider_Square1->setObjectName(QString::fromUtf8("horizontalSlider_Square1"));
        horizontalSlider_Square1->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Square1, 2, 3, 1, 1);

        checkBox_Square1 = new QCheckBox(verticalLayoutWidget);
        checkBox_Square1->setObjectName(QString::fromUtf8("checkBox_Square1"));
        checkBox_Square1->setLayoutDirection(Qt::RightToLeft);
        checkBox_Square1->setText(QString::fromUtf8(""));

        gridLayout->addWidget(checkBox_Square1, 2, 1, 1, 1);

        horizontalSlider_Noise = new QSlider(verticalLayoutWidget);
        horizontalSlider_Noise->setObjectName(QString::fromUtf8("horizontalSlider_Noise"));
        horizontalSlider_Noise->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Noise, 5, 3, 1, 1);

        line_Square2 = new QFrame(verticalLayoutWidget);
        line_Square2->setObjectName(QString::fromUtf8("line_Square2"));
        line_Square2->setFrameShape(QFrame::VLine);
        line_Square2->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_Square2, 3, 2, 1, 1);

        horizontalSlider_Extra = new QSlider(verticalLayoutWidget);
        horizontalSlider_Extra->setObjectName(QString::fromUtf8("horizontalSlider_Extra"));
        horizontalSlider_Extra->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Extra, 7, 3, 1, 1);

        checkBox_DMC = new QCheckBox(verticalLayoutWidget);
        checkBox_DMC->setObjectName(QString::fromUtf8("checkBox_DMC"));
        checkBox_DMC->setLayoutDirection(Qt::RightToLeft);
        checkBox_DMC->setText(QString::fromUtf8(""));

        gridLayout->addWidget(checkBox_DMC, 6, 1, 1, 1);

        horizontalSlider_Triangle = new QSlider(verticalLayoutWidget);
        horizontalSlider_Triangle->setObjectName(QString::fromUtf8("horizontalSlider_Triangle"));
        horizontalSlider_Triangle->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Triangle, 4, 3, 1, 1);

        line_Triangle = new QFrame(verticalLayoutWidget);
        line_Triangle->setObjectName(QString::fromUtf8("line_Triangle"));
        line_Triangle->setFrameShape(QFrame::VLine);
        line_Triangle->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_Triangle, 4, 2, 1, 1);

        line_DMC = new QFrame(verticalLayoutWidget);
        line_DMC->setObjectName(QString::fromUtf8("line_DMC"));
        line_DMC->setFrameShape(QFrame::VLine);
        line_DMC->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_DMC, 6, 2, 1, 1);

        checkBox_Square2 = new QCheckBox(verticalLayoutWidget);
        checkBox_Square2->setObjectName(QString::fromUtf8("checkBox_Square2"));
        checkBox_Square2->setLayoutDirection(Qt::RightToLeft);
        checkBox_Square2->setText(QString::fromUtf8(""));

        gridLayout->addWidget(checkBox_Square2, 3, 1, 1, 1);

        horizontalSlider_Square2 = new QSlider(verticalLayoutWidget);
        horizontalSlider_Square2->setObjectName(QString::fromUtf8("horizontalSlider_Square2"));
        horizontalSlider_Square2->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(horizontalSlider_Square2, 3, 3, 1, 1);

        label = new QLabel(verticalLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label->sizePolicy().hasHeightForWidth());
        label->setSizePolicy(sizePolicy1);
        label->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label, 0, 1, 1, 1);

        label_2 = new QLabel(verticalLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        sizePolicy1.setHeightForWidth(label_2->sizePolicy().hasHeightForWidth());
        label_2->setSizePolicy(sizePolicy1);
        label_2->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_2, 0, 3, 1, 1);


        verticalLayout->addLayout(gridLayout);

        line = new QFrame(verticalLayoutWidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton_Active_all = new QPushButton(verticalLayoutWidget);
        pushButton_Active_all->setObjectName(QString::fromUtf8("pushButton_Active_all"));

        horizontalLayout->addWidget(pushButton_Active_all);

        pushButton_Disable_all = new QPushButton(verticalLayoutWidget);
        pushButton_Disable_all->setObjectName(QString::fromUtf8("pushButton_Disable_all"));

        horizontalLayout->addWidget(pushButton_Disable_all);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayoutWidget = new QWidget(APU_channels);
        horizontalLayoutWidget->setObjectName(QString::fromUtf8("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(10, 290, 291, 31));
        horizontalLayout_Standard_Button = new QHBoxLayout(horizontalLayoutWidget);
        horizontalLayout_Standard_Button->setObjectName(QString::fromUtf8("horizontalLayout_Standard_Button"));
        horizontalLayout_Standard_Button->setContentsMargins(0, 0, 0, 0);
        pushButton_Defaults = new QPushButton(horizontalLayoutWidget);
        pushButton_Defaults->setObjectName(QString::fromUtf8("pushButton_Defaults"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Defaults);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_Standard_Button->addItem(horizontalSpacer);

        pushButton_Apply = new QPushButton(horizontalLayoutWidget);
        pushButton_Apply->setObjectName(QString::fromUtf8("pushButton_Apply"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Apply);

        pushButton_Discard = new QPushButton(horizontalLayoutWidget);
        pushButton_Discard->setObjectName(QString::fromUtf8("pushButton_Discard"));

        horizontalLayout_Standard_Button->addWidget(pushButton_Discard);

        QWidget::setTabOrder(checkBox_Master, horizontalSlider_Master);
        QWidget::setTabOrder(horizontalSlider_Master, checkBox_Square1);
        QWidget::setTabOrder(checkBox_Square1, horizontalSlider_Square1);
        QWidget::setTabOrder(horizontalSlider_Square1, checkBox_Square2);
        QWidget::setTabOrder(checkBox_Square2, horizontalSlider_Square2);
        QWidget::setTabOrder(horizontalSlider_Square2, checkBox_Triangle);
        QWidget::setTabOrder(checkBox_Triangle, horizontalSlider_Triangle);
        QWidget::setTabOrder(horizontalSlider_Triangle, checkBox_Noise);
        QWidget::setTabOrder(checkBox_Noise, horizontalSlider_Noise);
        QWidget::setTabOrder(horizontalSlider_Noise, checkBox_DMC);
        QWidget::setTabOrder(checkBox_DMC, horizontalSlider_DMC);
        QWidget::setTabOrder(horizontalSlider_DMC, checkBox_Extra);
        QWidget::setTabOrder(checkBox_Extra, horizontalSlider_Extra);
        QWidget::setTabOrder(horizontalSlider_Extra, pushButton_Active_all);
        QWidget::setTabOrder(pushButton_Active_all, pushButton_Disable_all);

        retranslateUi(APU_channels);

        QMetaObject::connectSlotsByName(APU_channels);
    } // setupUi

    void retranslateUi(QDialog *APU_channels)
    {
        APU_channels->setWindowTitle(QApplication::translate("APU_channels", "APU Channels", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("APU_channels", "Channels", 0, QApplication::UnicodeUTF8));
        label_Square1->setText(QApplication::translate("APU_channels", "Square 1", 0, QApplication::UnicodeUTF8));
        label_DMC->setText(QApplication::translate("APU_channels", "DMC", 0, QApplication::UnicodeUTF8));
        label_Noise->setText(QApplication::translate("APU_channels", "Noise", 0, QApplication::UnicodeUTF8));
        label_Square2->setText(QApplication::translate("APU_channels", "Square 2", 0, QApplication::UnicodeUTF8));
        label_Extra->setText(QApplication::translate("APU_channels", "Extra", 0, QApplication::UnicodeUTF8));
        label_Master->setText(QApplication::translate("APU_channels", "Master", 0, QApplication::UnicodeUTF8));
        label_Triangle->setText(QApplication::translate("APU_channels", "Triangle", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("APU_channels", "active", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("APU_channels", "volume", 0, QApplication::UnicodeUTF8));
        pushButton_Active_all->setText(QApplication::translate("APU_channels", "Active all", 0, QApplication::UnicodeUTF8));
        pushButton_Disable_all->setText(QApplication::translate("APU_channels", "Disable all", 0, QApplication::UnicodeUTF8));
        pushButton_Defaults->setText(QApplication::translate("APU_channels", "Defaults", 0, QApplication::UnicodeUTF8));
        pushButton_Apply->setText(QApplication::translate("APU_channels", "Apply", 0, QApplication::UnicodeUTF8));
        pushButton_Discard->setText(QApplication::translate("APU_channels", "Discard", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class APU_channels: public Ui_APU_channels {};
} // namespace Ui

QT_END_NAMESPACE

#endif // DLGAPUCHANNELS_H
