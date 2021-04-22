#ifndef PROCESSOREXTRACTOR_H
#define PROCESSOREXTRACTOR_H

#include <QDialog>
#include <QtDebug>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QComboBox>
#include <QUuid>
#include <QByteArray>
#include <QDateTime>
#include <QProcess>
#include <QMessageBox>

namespace Ui {
class ProcessorExtractor;
}

class ProcessorExtractor : public QDialog
{
    Q_OBJECT
    
public:
    explicit ProcessorExtractor(QWidget *parent = 0);
    ~ProcessorExtractor();

protected slots:
    void slot_chooseHeader();
    void slot_chooseXml();
    void slot_selectMethod(QString method);
    void slot_createFiles();
    void slot_manageProcess(int ret);

private:
    Ui::ProcessorExtractor *ui;
    bool loadHeaderFile(QString filename);
    static QString _startExp;
    QStringList _types;
    QStringList _shows;
    QProcess _process;
    bool _qmakeLaunched;

};

#endif // PROCESSOREXTRACTOR_H
