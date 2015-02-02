/********************************************************************************
** Form generated from reading UI file 'videopreview.ui'
**
** Created by: Qt User Interface Compiler version 4.8.5
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIDEOPREVIEW_H
#define UI_VIDEOPREVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QToolButton>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VideoPreview
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *videoWidget;
    QHBoxLayout *horizontalLayout;
    QToolButton *toolButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;

    void setupUi(QDialog *VideoPreview)
    {
        if (VideoPreview->objectName().isEmpty())
            VideoPreview->setObjectName(QString::fromUtf8("VideoPreview"));
        VideoPreview->resize(640, 480);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(VideoPreview->sizePolicy().hasHeightForWidth());
        VideoPreview->setSizePolicy(sizePolicy);
        VideoPreview->setMinimumSize(QSize(640, 480));
        VideoPreview->setMaximumSize(QSize(640, 480));
        verticalLayout = new QVBoxLayout(VideoPreview);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        videoWidget = new QWidget(VideoPreview);
        videoWidget->setObjectName(QString::fromUtf8("videoWidget"));
        sizePolicy.setHeightForWidth(videoWidget->sizePolicy().hasHeightForWidth());
        videoWidget->setSizePolicy(sizePolicy);
        videoWidget->setMinimumSize(QSize(622, 431));

        verticalLayout->addWidget(videoWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        toolButton = new QToolButton(VideoPreview);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));

        horizontalLayout->addWidget(toolButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(VideoPreview);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(VideoPreview);

        QMetaObject::connectSlotsByName(VideoPreview);
    } // setupUi

    void retranslateUi(QDialog *VideoPreview)
    {
        VideoPreview->setWindowTitle(QApplication::translate("VideoPreview", "Preview", 0, QApplication::UnicodeUTF8));
        toolButton->setText(QApplication::translate("VideoPreview", "...", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("VideoPreview", "Fermer", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class VideoPreview: public Ui_VideoPreview {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIDEOPREVIEW_H
