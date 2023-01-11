#ifndef DICTIONNARYVALIDATOR_H
#define DICTIONNARYVALIDATOR_H

#include <QMainWindow>
#include <QtDebug>
#include <QXmlQuery>
#include <QXmlStreamWriter>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QRegExp>

namespace Ui {
class DictionnaryValidator;
}

class DictionnaryValidator : public QMainWindow
{
    Q_OBJECT

public:
    explicit DictionnaryValidator(QWidget *parent = 0);
    ~DictionnaryValidator();

protected slots:
    void slot_generateTranslationFiles();
    void slot_publishDictionnary();
    void slot_selectWorkspace();
private:
    Ui::DictionnaryValidator *ui;
    void init();

    static const QString HEADER_TEMPLATE_PATH;
    static const QString HEADER_GEN_PATH;
    static const QString HEADER_DEST_PATH;
    static const QString DICTIONNARY_SRC_PATH;
    static const QString DICTIONNARY_DEST_PATH;
    static const QString BACKUP_TIMESTAMP_FORMAT;
    static const QString XML_TIMESTAMP_REGEXP;
    static const QString XML_TIMESTAMP_PREFIX;
    static const QString XML_TIMESTAMP_SUFFIX;
    static const QString XML_TIMESTAMP_FORMAT;

    QString m_matisse_worspace_path;
};

#endif // DICTIONNARYVALIDATOR_H
