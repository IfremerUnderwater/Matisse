#ifndef NETWORKCHECKDIALOG_H
#define NETWORKCHECKDIALOG_H

#include <QDialog>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QtDebug>

namespace Ui {
class NetworkCheckDialog;
}

class NetworkCheckDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkCheckDialog(QWidget *parent = 0);
    ~NetworkCheckDialog();

protected slots:
    void slot_connect(bool checked);
    void slot_clearFrames();
    void slot_quit();
    void slot_readPendingDatagrams();

private:
    void clearSocket();

    Ui::NetworkCheckDialog *_ui;
    QUdpSocket *_clientSocket;
};

#endif // NETWORKCHECKDIALOG_H
