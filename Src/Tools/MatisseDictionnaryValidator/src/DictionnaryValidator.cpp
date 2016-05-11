#include "DictionnaryValidator.h"
#include "ui_DictionnaryValidator.h"

const QString DictionnaryValidator::HEADER_TEMPLATE_PATH = QString("../../template/MatisseDictionnaryLabels.h");
const QString DictionnaryValidator::HEADER_GEN_PATH = QString("../../gen/MatisseDictionnaryLabels.h");
const QString DictionnaryValidator::HEADER_DEST_PATH = QString("../../../../Libraries/MatisseTools/src/MatisseDictionnaryLabels.h");
const QString DictionnaryValidator::DICTIONNARY_SRC_PATH = QString("../../config/MatisseParametersDictionnary.xml");
const QString DictionnaryValidator::DICTIONNARY_DEST_PATH = QString("../../../../../Config/config/MatisseParametersDictionnary.xml");
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
}

void DictionnaryValidator::slot_generateTranslationFiles()
{
    qDebug() << "Generating translation files...";

    QDateTime timestamp = QDateTime::currentDateTime();
    QString timestampText = timestamp.toString("dd/MM/yyyy hh:mm:ss");

    /* Reading template header file */
    QFile headerTemplateFile(HEADER_TEMPLATE_PATH);
    headerTemplateFile.open(QIODevice::ReadOnly);
    QTextStream templateReader(&headerTemplateFile);
    QString  templateContent = templateReader.readAll();
    headerTemplateFile.close();

    QFile previousHeaderFile(HEADER_GEN_PATH);

    if (previousHeaderFile.exists()) {
        qDebug() << "Backing up previous dictionnary header file...";
        QString backupSuffix = timestamp.toString(BACKUP_TIMESTAMP_FORMAT);
        previousHeaderFile.rename(HEADER_GEN_PATH + backupSuffix);
    }


    /* Substitute timestamp placeholder */
    QString genContent = templateContent.replace("${timestamp}", timestampText);

    /* Generate label declaration */
    QString genDeclareLabels;

    QFile dicoFile("config/MatisseParametersDictionnary.xml");
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
    QFile headerFile(HEADER_GEN_PATH);

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

    QFile generatedHeaderFile(HEADER_GEN_PATH);

    if (!generatedHeaderFile.exists()) {
        qCritical() << "Matisse dictionnary labels header file was not generated";
        return;
    }

    QFile currentDictionnaryFile(DICTIONNARY_SRC_PATH);
    qDebug() << "Updating dictionnary timestamp...";
    currentDictionnaryFile.open(QIODevice::ReadOnly);
    QTextStream reader(&currentDictionnaryFile);
    QString content = reader.readAll();
    currentDictionnaryFile.close();

    qDebug() << "Backing up previous dictionnary definition file...";
    QString backupSuffix = timestamp.toString(BACKUP_TIMESTAMP_FORMAT);
    currentDictionnaryFile.rename(DICTIONNARY_SRC_PATH + backupSuffix);

    QFile newDictionnaryFile(DICTIONNARY_SRC_PATH);

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

    QFile previousDestDictionnaryFile(DICTIONNARY_DEST_PATH);
    if (previousDestDictionnaryFile.exists()) {
        qDebug() << "Replacing dictionnary definition file...";
        previousDestDictionnaryFile.remove();
    }

    QFile previousDestHeaderFile(HEADER_DEST_PATH);
    if (previousDestHeaderFile.exists()) {
        qDebug() << "Replacing dictionnary header file...";
        previousDestHeaderFile.remove();
    }

    newDictionnaryFile.copy(DICTIONNARY_DEST_PATH);
    generatedHeaderFile.copy(HEADER_DEST_PATH);

    qDebug() << "Dictionnary published on " << timestamp.toString("dd/MM/yyyy hh:mm:ss");

    ui->_TB_publish->setEnabled(false);
}
