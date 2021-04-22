#include "threadudpclientgui.h"
#include "ui_threadudpclientgui.h"


ThreadUdpClientGUI::ThreadUdpClientGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ThreadUdpClientGUI)
{
    ui->setupUi(this);
    ui->buttonStop->setEnabled(false);
    QThread::currentThread()->setObjectName("GUI");
}

ThreadUdpClientGUI::~ThreadUdpClientGUI()
{
    delete ui;
}

void ThreadUdpClientGUI::on_buttonStart_clicked()
{

    _task = new QThread;
    _worker = new Worker;
    _task->setObjectName("Task");

    connect(_task, SIGNAL(finished()), this, SLOT(slot_deleteTask()));
    _dim2UdpListener = new Dim2UDPListener();
    _dim2UdpListener->slot_configure(7777);

    connect(_dim2UdpListener, SIGNAL(signal_newline(QString)), _worker, SLOT(slot_processLine(QString)), Qt::QueuedConnection);
    _worker->moveToThread(_task);

    _task->start(QThread::IdlePriority);



    ui->buttonStart->setEnabled(false);
    ui->buttonStop->setEnabled(true);
}

void ThreadUdpClientGUI::on_buttonStop_clicked()
{
    qDebug("Ask to stop");
    _worker->_started = false;
    _task->quit();
}

void ThreadUdpClientGUI::slot_deleteTask()
{
    qDebug("Thread finished");
    ui->buttonStart->setEnabled(true);
    ui->buttonStop->setEnabled(false);
    _task->deleteLater();
    _dim2UdpListener->deleteLater();
    _worker->deleteLater();
}

void ThreadUdpClientGUI::on_buttonDo_clicked()
{
    qDebug("Try to do?");
    emit signal_doSomething();
}

