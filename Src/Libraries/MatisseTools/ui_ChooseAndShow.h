/********************************************************************************
** Form generated from reading UI file 'ChooseAndShow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHOOSEANDSHOW_H
#define UI_CHOOSEANDSHOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChooseAndShow
{
public:
    QHBoxLayout *horizontalLayout;
    QLineEdit *_LE_show;
    QPushButton *_PB_choose;

    void setupUi(QWidget *ChooseAndShow)
    {
        if (ChooseAndShow->objectName().isEmpty())
            ChooseAndShow->setObjectName(QString::fromUtf8("ChooseAndShow"));
        ChooseAndShow->resize(125, 23);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ChooseAndShow->sizePolicy().hasHeightForWidth());
        ChooseAndShow->setSizePolicy(sizePolicy);
        horizontalLayout = new QHBoxLayout(ChooseAndShow);
        horizontalLayout->setSpacing(0);
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        _LE_show = new QLineEdit(ChooseAndShow);
        _LE_show->setObjectName(QString::fromUtf8("_LE_show"));

        horizontalLayout->addWidget(_LE_show);

        _PB_choose = new QPushButton(ChooseAndShow);
        _PB_choose->setObjectName(QString::fromUtf8("_PB_choose"));

        horizontalLayout->addWidget(_PB_choose);


        retranslateUi(ChooseAndShow);

        QMetaObject::connectSlotsByName(ChooseAndShow);
    } // setupUi

    void retranslateUi(QWidget *ChooseAndShow)
    {
        ChooseAndShow->setWindowTitle(QApplication::translate("ChooseAndShow", "Form", 0, QApplication::UnicodeUTF8));
        _PB_choose->setText(QApplication::translate("ChooseAndShow", "PushButton", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ChooseAndShow: public Ui_ChooseAndShow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHOOSEANDSHOW_H
