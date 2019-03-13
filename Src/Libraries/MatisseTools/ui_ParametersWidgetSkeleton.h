/********************************************************************************
** Form generated from reading UI file 'ParametersWidgetSkeleton.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PARAMETERSWIDGETSKELETON_H
#define UI_PARAMETERSWIDGETSKELETON_H

#include <QtCore/QVariant>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>
#else
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#endif
QT_BEGIN_NAMESPACE

class Ui_ParametersWidgetSkeleton
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *_HLAY_parameters;
    QVBoxLayout *_VLAY_parameters;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *ParametersWidgetSkeleton)
    {
        if (ParametersWidgetSkeleton->objectName().isEmpty())
            ParametersWidgetSkeleton->setObjectName(QString::fromUtf8("ParametersWidgetSkeleton"));
        ParametersWidgetSkeleton->resize(20, 23);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ParametersWidgetSkeleton->sizePolicy().hasHeightForWidth());
        ParametersWidgetSkeleton->setSizePolicy(sizePolicy);
        verticalLayout_2 = new QVBoxLayout(ParametersWidgetSkeleton);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        _HLAY_parameters = new QHBoxLayout();
        _HLAY_parameters->setObjectName(QString::fromUtf8("_HLAY_parameters"));
        _VLAY_parameters = new QVBoxLayout();
        _VLAY_parameters->setSpacing(3);
        _VLAY_parameters->setObjectName(QString::fromUtf8("_VLAY_parameters"));

        _HLAY_parameters->addLayout(_VLAY_parameters);

        horizontalSpacer = new QSpacerItem(1, 20, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

        _HLAY_parameters->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(_HLAY_parameters);

        verticalSpacer = new QSpacerItem(20, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

        verticalLayout_2->addItem(verticalSpacer);


        retranslateUi(ParametersWidgetSkeleton);

        QMetaObject::connectSlotsByName(ParametersWidgetSkeleton);
    } // setupUi

    void retranslateUi(QWidget *ParametersWidgetSkeleton)
    {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        ParametersWidgetSkeleton->setWindowTitle(QApplication::translate("ParametersWidgetSkeleton", "Form", 0));
#else
        ParametersWidgetSkeleton->setWindowTitle(QApplication::translate("ParametersWidgetSkeleton", "Form", 0, QApplication::UnicodeUTF8));
#endif
    } // retranslateUi

};

namespace Ui {
    class ParametersWidgetSkeleton: public Ui_ParametersWidgetSkeleton {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PARAMETERSWIDGETSKELETON_H
