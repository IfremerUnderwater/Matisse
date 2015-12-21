#ifndef THREADUDPCLIENTGUI_H
#define THREADUDPCLIENTGUI_H

#include <QMainWindow>
#include <QThread>


#include "dim2udplistener.h"
#include "worker.h"

namespace Ui {
class ThreadUdpClientGUI;
}

class ThreadUdpClientGUI : public QMainWindow
{
    Q_OBJECT

public:
    explicit ThreadUdpClientGUI(QWidget *parent = 0);
    ~ThreadUdpClientGUI();

signals:
    void signal_doSomething();

private slots:
    void on_buttonStart_clicked();
    void on_buttonStop_clicked();

    void slot_deleteTask();
    void on_buttonDo_clicked();

private:
    Ui::ThreadUdpClientGUI *ui;
    QThread *_task;
    Dim2UDPListener *_dim2UdpListener;
    Worker *_worker;
};

#endif // THREADUDPCLIENTGUI_H
