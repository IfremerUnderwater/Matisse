#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "VMR_Capture.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnOpen_clicked();

    void on_btnProcess_clicked();

    void on_btnOK_clicked();

    void on_btnInitCam_clicked();

    void on_btnGrab_clicked();

private:
    Ui::MainWindow *ui;
    CVMR_Capture m_VMRCap;
};

#endif // MAINWINDOW_H
