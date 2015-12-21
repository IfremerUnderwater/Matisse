/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QPushButton *buttonStart;
    QGroupBox *groupBox;
    QLineEdit *lineEditFile;
    QLabel *labelFile;
    QPushButton *pushButtonFile;
    QSpinBox *sB_Periode;
    QLabel *label;
    QLabel *labelPort;
    QLineEdit *lineEditPort;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(368, 201);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        buttonStart = new QPushButton(centralWidget);
        buttonStart->setObjectName(QString::fromUtf8("buttonStart"));
        buttonStart->setCheckable(true);

        gridLayout->addWidget(buttonStart, 3, 0, 1, 2);

        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(1);
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        lineEditFile = new QLineEdit(groupBox);
        lineEditFile->setObjectName(QString::fromUtf8("lineEditFile"));
        lineEditFile->setGeometry(QRect(75, 25, 221, 20));
        labelFile = new QLabel(groupBox);
        labelFile->setObjectName(QString::fromUtf8("labelFile"));
        labelFile->setGeometry(QRect(10, 24, 59, 16));
        pushButtonFile = new QPushButton(groupBox);
        pushButtonFile->setObjectName(QString::fromUtf8("pushButtonFile"));
        pushButtonFile->setGeometry(QRect(310, 20, 30, 23));
        pushButtonFile->setMaximumSize(QSize(30, 16777215));
        sB_Periode = new QSpinBox(groupBox);
        sB_Periode->setObjectName(QString::fromUtf8("sB_Periode"));
        sB_Periode->setGeometry(QRect(180, 60, 41, 20));
        sB_Periode->setMinimum(1);
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(122, 60, 52, 16));
        labelPort = new QLabel(groupBox);
        labelPort->setObjectName(QString::fromUtf8("labelPort"));
        labelPort->setGeometry(QRect(14, 60, 20, 16));
        lineEditPort = new QLineEdit(groupBox);
        lineEditPort->setObjectName(QString::fromUtf8("lineEditPort"));
        lineEditPort->setGeometry(QRect(40, 60, 51, 20));
        lineEditFile->raise();
        labelFile->raise();
        pushButtonFile->raise();
        sB_Periode->raise();
        label->raise();
        buttonStart->raise();

        gridLayout->addWidget(groupBox, 2, 0, 1, 2);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 368, 21));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Dim2Server", 0, QApplication::UnicodeUTF8));
        buttonStart->setText(QApplication::translate("MainWindow", "Start", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("MainWindow", "Configure", 0, QApplication::UnicodeUTF8));
        labelFile->setText(QApplication::translate("MainWindow", "Fichier DIM2", 0, QApplication::UnicodeUTF8));
        pushButtonFile->setText(QApplication::translate("MainWindow", "...", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Periode (s)", 0, QApplication::UnicodeUTF8));
        labelPort->setText(QApplication::translate("MainWindow", "Port", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
