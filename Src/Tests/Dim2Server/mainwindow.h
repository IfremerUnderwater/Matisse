#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QUdpSocket>

class ProtobufInterface;

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void initServer();

private slots:
    void on_pushButtonFile_clicked();
    void on_buttonStart_toggled(bool checked);

private:
    Ui::MainWindow *ui;

    ProtobufInterface *_protoInterface;
    QUdpSocket *_udpSocket;
    QTextStream *_reader;
    QHostAddress *_udpAddress;
    QString _fileName;
    bool _isTcp;
    int _port;
    int _indTimer;

    void timerEvent(QTimerEvent *event);
};

#endif // MAINWINDOW_H
