/********************************************************************************
** Form generated from reading UI file 'mainwindowbase.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOWBASE_H
#define UI_MAINWINDOWBASE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindowBase
{
public:
    QAction *mpActionZoomIn;
    QAction *mpActionZoomOut;
    QAction *mpActionPan;
    QAction *mpActionAddLayer;
    QAction *mpActionAddRasterLayer;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QFrame *frameMap;
    QMenuBar *menubar;
    QMenu *menuMap;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindowBase)
    {
        if (MainWindowBase->objectName().isEmpty())
            MainWindowBase->setObjectName(QString::fromUtf8("MainWindowBase"));
        MainWindowBase->resize(579, 330);
        mpActionZoomIn = new QAction(MainWindowBase);
        mpActionZoomIn->setObjectName(QString::fromUtf8("mpActionZoomIn"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/mActionZoomIn.png"), QSize(), QIcon::Normal, QIcon::Off);
        mpActionZoomIn->setIcon(icon);
        mpActionZoomOut = new QAction(MainWindowBase);
        mpActionZoomOut->setObjectName(QString::fromUtf8("mpActionZoomOut"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/mActionZoomOut.png"), QSize(), QIcon::Normal, QIcon::Off);
        mpActionZoomOut->setIcon(icon1);
        mpActionPan = new QAction(MainWindowBase);
        mpActionPan->setObjectName(QString::fromUtf8("mpActionPan"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/mActionPan.png"), QSize(), QIcon::Normal, QIcon::Off);
        mpActionPan->setIcon(icon2);
        mpActionAddLayer = new QAction(MainWindowBase);
        mpActionAddLayer->setObjectName(QString::fromUtf8("mpActionAddLayer"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/mActionAddLayer.png"), QSize(), QIcon::Normal, QIcon::Off);
        mpActionAddLayer->setIcon(icon3);
        mpActionAddRasterLayer = new QAction(MainWindowBase);
        mpActionAddRasterLayer->setObjectName(QString::fromUtf8("mpActionAddRasterLayer"));
        mpActionAddRasterLayer->setIcon(icon3);
        centralwidget = new QWidget(MainWindowBase);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(9, 9, 9, 9);
        frameMap = new QFrame(centralwidget);
        frameMap->setObjectName(QString::fromUtf8("frameMap"));
        frameMap->setFrameShape(QFrame::StyledPanel);
        frameMap->setFrameShadow(QFrame::Raised);

        gridLayout->addWidget(frameMap, 0, 0, 1, 1);

        MainWindowBase->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindowBase);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 579, 21));
        menuMap = new QMenu(menubar);
        menuMap->setObjectName(QString::fromUtf8("menuMap"));
        MainWindowBase->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindowBase);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindowBase->setStatusBar(statusbar);

        menubar->addAction(menuMap->menuAction());
        menuMap->addAction(mpActionZoomIn);
        menuMap->addAction(mpActionZoomOut);
        menuMap->addAction(mpActionPan);
        menuMap->addAction(mpActionAddLayer);
        menuMap->addAction(mpActionAddRasterLayer);

        retranslateUi(MainWindowBase);

        QMetaObject::connectSlotsByName(MainWindowBase);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindowBase)
    {
        MainWindowBase->setWindowTitle(QApplication::translate("MainWindowBase", "MainWindowBase", 0, QApplication::UnicodeUTF8));
        mpActionZoomIn->setText(QApplication::translate("MainWindowBase", "Zoom In", 0, QApplication::UnicodeUTF8));
        mpActionZoomOut->setText(QApplication::translate("MainWindowBase", "Zoom Out", 0, QApplication::UnicodeUTF8));
        mpActionPan->setText(QApplication::translate("MainWindowBase", "Pan", 0, QApplication::UnicodeUTF8));
        mpActionAddLayer->setText(QApplication::translate("MainWindowBase", "Add Vector Layer", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        mpActionAddLayer->setToolTip(QApplication::translate("MainWindowBase", "Add Vector Layer", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        mpActionAddRasterLayer->setText(QApplication::translate("MainWindowBase", "Add Raster Layer", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        mpActionAddRasterLayer->setToolTip(QApplication::translate("MainWindowBase", "Add Raster Layer", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        menuMap->setTitle(QApplication::translate("MainWindowBase", "Map", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindowBase: public Ui_MainWindowBase {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOWBASE_H
