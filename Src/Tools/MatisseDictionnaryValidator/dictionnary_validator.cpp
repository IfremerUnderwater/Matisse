#include "dictionnary_validator.h"
#include "ui_dictionnary_validator.h"
#include <QFileDialog>
#include <QDir>

const QString DictionnaryValidator::HEADER_TEMPLATE_PATH = QString("Src/Tools/MatisseDictionnaryValidator/template/MatisseDictionnaryLabels.h");
const QString DictionnaryValidator::HEADER_GEN_PATH = QString("Src/Tools/MatisseDictionnaryValidator/gen/MatisseDictionnaryLabels.h");
const QString DictionnaryValidator::HEADER_DEST_PATH = QString("Src/Libraries/MatisseCore/Parameters/MatisseDictionnaryLabels.h");
//const QString DictionnaryValidator::DICTIONNARY_SRC_PATH = QString("Src/Tools/MatisseDictionnaryValidator/config/MatisseParametersDictionnary.xml");
const QString DictionnaryValidator::DICTIONNARY_SRC_PATH = QString("Deploy/config/MatisseParametersDictionnary.xml");
const QString DictionnaryValidator::DICTIONNARY_DEST_PATH = QString("Deploy/config/MatisseParametersDictionnary.xml");
const QString DictionnaryValidator::BACKUP_TIMESTAMP_FORMAT = QString(".yyyyMMdd.hhmmss");
const QString DictionnaryValidator::XML_TIMESTAMP_REGEXP = QString("publicationTimestamp=\"[^\"]*\"");
const QString DictionnaryValidator::XML_TIMESTAMP_PREFIX = QString("publicationTimestamp=\"");
const QString DictionnaryValidator::XML_TIMESTAMP_SUFFIX = QString("\"");
const QString DictionnaryValidator::XML_TIMESTAMP_FORMAT = QString("yyyy-MM-dd'T'hh:mm:ss");

DictionnaryValidator::DictionnaryValidator(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DictionnaryValidator)
{
    ui->setupUi(this);
    init();
}

DictionnaryValidator::~DictionnaryValidator()

{
    delete ui;
}

void DictionnaryValidator::init()
{
    /* check dictionnary integrity */


    ui->_TB_publish->setEnabled(false);

    connect(ui->_TB_Generate, SIGNAL(clicked(bool)), this, SLOT(slot_generateTranslationFiles()));
    connect(ui->_TB_publish, SIGNAL(clicked(bool)), this, SLOT(slot_publishDictionnary()));
    connect(ui->select_path, SIGNAL(clicked(bool)), this, SLOT(slot_selectWorkspace()));

}

void DictionnaryValidator::slot_generateTranslationFiles()
{
    qDebug() << "Generating translation files...";

    QDateTime timestamp = QDateTime::currentDateTime();
    QString timestampText = timestamp.toString("dd/MM/yyyy hh:mm:ss");

    /* Reading template header file */
    QFile headerTemplateFile(m_matisse_worspace_path+QDir::separator()+HEADER_TEMPLATE_PATH);
    headerTemplateFile.open(QIODevice::ReadOnly);
    QTextStream templateReader(&headerTemplateFile);
    QString  templateContent = templateReader.readAll();
    headerTemplateFile.close();

    QFile previousHeaderFile(m_matisse_worspace_path + QDir::separator() + HEADER_GEN_PATH);

    if (previousHeaderFile.exists()) {
        qDebug() << "Backing up previous dictionnary header file...";
        QString backupSuffix = timestamp.toString(BACKUP_TIMESTAMP_FORMAT);
        previousHeaderFile.rename(m_matisse_worspace_path + QDir::separator() + HEADER_GEN_PATH + backupSuffix);
    }


    /* Substitute timestamp placeholder */
    QString genContent = templateContent.replace("${timestamp}", timestampText);

    /* Generate label declaration */
    QString genDeclareLabels;

    QFile dicoFile(m_matisse_worspace_path + QDir::separator() + DICTIONNARY_SRC_PATH);
    dicoFile.open(QIODevice::ReadOnly);

    QXmlStreamReader xmlReader(&dicoFile);

    while(!xmlReader.atEnd()) {
        QXmlStreamReader::TokenType type = xmlReader.readNext();

        if (type == QXmlStreamReader::StartElement) {

            QString elementName = xmlReader.name().toString();
            QXmlStreamAttributes attributes = xmlReader.attributes();

            if ("Structure" ==  elementName) {
                QString structureName = attributes.value("name").toString();
                genDeclareLabels.append("\n\t\t/*\n\t\t * STRUCTURE : ").append(structureName).append("\n\t\t */\n");
            } else if ("ParametersGroup" ==  elementName) {
                QString groupName = attributes.value("name").toString();
                genDeclareLabels.append("\n\t\t/* GROUP : ").append(groupName).append(" */\n");

                if (attributes.hasAttribute("text")) {
                    QString textKey = attributes.value("text").toString();
                    qDebug() << "Line n\u00B0 " << xmlReader.lineNumber() << " - Declaring group key " << textKey;
                    genDeclareLabels.append("\t\t_groupLabelKeys.insert(\"").append(groupName).append("\", QT_TRANSLATE_NOOP(\"MatisseTools::MatisseDictionnaryLabels\", \"").append(textKey).append("\"));\n");
                }

            } else if ("Parameter" ==  elementName) {
                QString paramName = attributes.value("name").toString();
                QString textKey = attributes.value("text").toString();
                qDebug() << "Line n\u00B0 " << xmlReader.lineNumber() << " - Declaring param key " << textKey;
                genDeclareLabels.append("\t\t_paramLabelKeys.insert(\"").append(paramName).append("\", QT_TRANSLATE_NOOP(\"MatisseTools::MatisseDictionnaryLabels\", \"").append(textKey).append("\"));\n");
            }

        }

        if (xmlReader.hasError()) {
            qWarning() << QString("Error while parsing XML dictionnary file :\n%1").arg(xmlReader.errorString());
        }
    }

    xmlReader.clear();
    dicoFile.close();

    /* substitute placeholder */
    genContent.replace("${declare_labels}", genDeclareLabels);

    /* write contents */
    QFile headerFile(m_matisse_worspace_path + QDir::separator() + HEADER_GEN_PATH);

    headerFile.open(QIODevice::WriteOnly);
    QTextStream writer(&headerFile);

    writer << genContent;

    writer.flush();
    headerFile.close();

    ui->_TB_publish->setEnabled(true);
    ui->_TB_Generate->setEnabled(false);
}

void DictionnaryValidator::slot_publishDictionnary()
{
    qDebug() << "Publishing dictionnary...";

    QDateTime timestamp = QDateTime::currentDateTime();

    QFile generatedHeaderFile(m_matisse_worspace_path + QDir::separator() + HEADER_GEN_PATH);

    if (!generatedHeaderFile.exists()) {
        qCritical() << "Matisse dictionnary labels header file was not generated";
        return;
    }

    QFile currentDictionnaryFile(m_matisse_worspace_path + QDir::separator() + DICTIONNARY_SRC_PATH);
    qDebug() << "Updating dictionnary timestamp...";
    currentDictionnaryFile.open(QIODevice::ReadOnly);
    QTextStream reader(&currentDictionnaryFile);
    QString content = reader.readAll();
    currentDictionnaryFile.close();

    /*qDebug() << "Backing up previous dictionnary definition file...";
    QString backupSuffix = timestamp.toString(BACKUP_TIMESTAMP_FORMAT);
    currentDictionnaryFile.rename(m_matisse_worspace_path + QDir::separator() + DICTIONNARY_SRC_PATH + backupSuffix);*/

    QFile newDictionnaryFile(m_matisse_worspace_path + QDir::separator() + DICTIONNARY_SRC_PATH);

    QRegExp timestampRex(XML_TIMESTAMP_REGEXP);
    QString timestampReplacement = XML_TIMESTAMP_PREFIX;
    timestampReplacement.append(timestamp.toString(XML_TIMESTAMP_FORMAT)).append(XML_TIMESTAMP_SUFFIX);

    content.replace(timestampRex, timestampReplacement);

    newDictionnaryFile.open(QIODevice::WriteOnly);
    QTextStream writer(&newDictionnaryFile);
    writer << content;
    writer.flush();
    newDictionnaryFile.close();

    qDebug("Copying dictionnary files...");

    QFile previousDestDictionnaryFile(m_matisse_worspace_path + QDir::separator() + DICTIONNARY_DEST_PATH);
    /*if (previousDestDictionnaryFile.exists()) {
        qDebug() << "Replacing dictionnary definition file...";
        previousDestDictionnaryFile.remove();
    }*/

    QFile previousDestHeaderFile(m_matisse_worspace_path + QDir::separator() + HEADER_DEST_PATH);
    if (previousDestHeaderFile.exists()) {
        qDebug() << "Replacing dictionnary header file...";
        previousDestHeaderFile.remove();
    }

    //newDictionnaryFile.copy(m_matisse_worspace_path + QDir::separator() + DICTIONNARY_DEST_PATH);
    generatedHeaderFile.copy(m_matisse_worspace_path + QDir::separator() + HEADER_DEST_PATH);

    qDebug() << "Dictionnary published on " << timestamp.toString("dd/MM/yyyy hh:mm:ss");

    ui->_TB_publish->setEnabled(false);
}

void DictionnaryValidator::slot_selectWorkspace()
{
    m_matisse_worspace_path = QFileDialog::getExistingDirectory(this, "Select Matisse workspace folder (containing Src folder)");
    ui->workspace_path->setText(m_matisse_worspace_path);
}
