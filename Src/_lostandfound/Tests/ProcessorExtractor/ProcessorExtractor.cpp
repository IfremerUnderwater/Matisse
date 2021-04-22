#include "ProcessorExtractor.h"
#include "ui_ProcessorExtractor.h"

QString ProcessorExtractor::_startExp = "^\\w+ (\\w+ ){0,1}\\w+\\(";

ProcessorExtractor::ProcessorExtractor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProcessorExtractor)
{
    ui->setupUi(this);
    //ui->_
    connect(ui->_PB_headerFile, SIGNAL(clicked()), this, SLOT(slot_chooseHeader()));
    connect(ui->_PB_xmlFile, SIGNAL(clicked()), this, SLOT(slot_chooseXml()));
    connect(ui->_PB_saveFunction, SIGNAL(clicked()), this, SLOT(slot_createFiles()));
    connect(ui->_LW_header, SIGNAL(currentTextChanged(QString)), this, SLOT(slot_selectMethod(QString)));
    // remplissage des listes pour comboboxes
    _types << "integer" << "real" << "string" << "enum";
    _shows << "edit line" << "slider" << "spin box" << "combo box" << "check box" << "file chooser" << "dir chooser";

    connect(&_process, SIGNAL(finished(int)), this, SLOT(slot_manageProcess(int)));

}

ProcessorExtractor::~ProcessorExtractor()
{
    delete ui;
}

void ProcessorExtractor::slot_chooseHeader()
{
    QString filename = QFileDialog::getOpenFileName(this, "Selection du header");
    ui->_LE_headerFile->setText(filename);
    loadHeaderFile(filename);
}

void ProcessorExtractor::slot_chooseXml()
{

}

void ProcessorExtractor::slot_selectMethod(QString method)
{
    qDebug() << "slot_select...";
    ui->_TAW_parameters->clearContents();
    ui->_TAW_parameters->setRowCount(0);
    ui->_GB_function->setTitle("Fonction: " + method);
    // decoupage de la ligne...
    QString methodName;
    int beginArgs = method.indexOf("(");
    int endArgs = method.lastIndexOf(")");
    if (beginArgs < 0) {
        return;
    }
    if (endArgs < 0) {
        return;
    }

    QStringList args = method.mid(beginArgs+1, endArgs - beginArgs -1).split(", ");
    foreach(QString arg, args) {
        if (arg.trimmed() == "") {
            continue;
        }
        int noRow = ui->_TAW_parameters->rowCount();
        ui->_TAW_parameters->insertRow(noRow);
        ui->_TAW_parameters->setItem(noRow, 0, new QTableWidgetItem(arg));
        QComboBox * cbTypes = new QComboBox(ui->_TAW_parameters);
        cbTypes->addItems(_types);
        QComboBox * cbShows = new QComboBox(ui->_TAW_parameters);
        cbShows->addItems(_shows);
        ui->_TAW_parameters->setCellWidget(noRow, 3, cbTypes);
        ui->_TAW_parameters->setCellWidget(noRow, 6, cbShows);
    }

}

void ProcessorExtractor::slot_createFiles()
{
    QString uid = QUuid::createUuid().toString().replace("-", "_");
    uid = uid.mid(1, uid.length()-2);
    QString processorName = "Test";
    QString config = "debug";
    QString notConfig = "release";

    QStringList includeFilesList; // = QStringList() << "toto.h" << "tutu.h" << "titi.h";
    QStringList sourceFilesList; // = QStringList() << "azerty.cpp" << "qsdfg.cpp";

    QString moreInclude;
    if (includeFilesList.size() >0) {
        moreInclude = "#include \"" + includeFilesList.join("\"\n#include \"") + "\"\n";
    }

    if (config == "release") {
        notConfig = "debug";
    }

    // creation du repertoire de sortie
    QDir pluginsDir("");
    pluginsDir.mkpath("generatedPlugins/" + uid);
    pluginsDir.cd("generatedPlugins/" + uid);

    QStringList exts;
    exts << ".h" << ".cpp" << ".pro";
    foreach(QString ext, exts) {
        QFile inFile("templates/template" + ext + ".txt");
        QFile outFile(pluginsDir.absolutePath() + "/PLUGIN_" + uid + ext);
        inFile.open(QIODevice::ReadOnly);
        outFile.open(QIODevice::ReadWrite);
        QByteArray datas = inFile.readAll();
        inFile.close();
        datas.replace("$PROCESSOR_NAME", processorName.toLocal8Bit());
        datas.replace("$PROCESSOR_ID", uid.toLocal8Bit());
        datas.replace("$DATE", QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss").toLocal8Bit());
        datas.replace("$RELEASE_OR_DEBUG", config.toLocal8Bit());
        datas.replace("$NOT_RELEASE_OR_DEBUG", notConfig.toLocal8Bit());
        datas.replace("$HEADER_FILES_LIST", includeFilesList.join("\\\n").toLocal8Bit());
        datas.replace("$SOURCE_FILES_LIST", sourceFilesList.join("\\\n").toLocal8Bit());
        datas.replace("$INCLUDES", moreInclude.toLocal8Bit());
        outFile.write(datas);
        outFile.close();
    }
    // generation de la dll
    _qmakeLaunched = true;
    _process.setWorkingDirectory(pluginsDir.absolutePath());

    _process.start("qmake.exe");

}

void ProcessorExtractor::slot_manageProcess(int ret)
{
    if (ret != 0) {
        QMessageBox::critical(this, "Erreur de génération", "Le plugin n'a pas été généré!");
        return;
    }
    if (_qmakeLaunched) {
        _process.start("mingw32-make.exe");
        _qmakeLaunched = false;
    }
}

bool ProcessorExtractor::loadHeaderFile(QString filename)
{
    ui->_LW_header->clear();
    QFile header(filename);
    if (!header.exists()) {
        qDebug() << "Fichier introuvable";
        return false;
    }
    if (!header.open(QIODevice::ReadOnly)) {
        qDebug() << "Fichier illisible";
        return false;
    }

    QTextStream fileOut(&header);
    while (!fileOut.atEnd()) {

        QString line(header.readLine());
        line = line.simplified();
        if (line == "") {
            continue;
        }
        line.replace("(", " ( ");
        line.replace(")", " ) ");
        // deuxième passe...
        line = line.simplified();

        line.replace(" ( ", "(");
        line.replace(" ) ", ")");
        qDebug() << "Ligne a analyser" << line;
        QRegExp regExp(_startExp);
        qDebug() << "regexp valid=" << regExp.isValid();
        qDebug() << "Pos=" << line.indexOf(regExp);
        if (line.indexOf(regExp) == 0) {
            if (line.startsWith("explicit")) {
                continue;
            }
            ui->_LW_header->addItem(line);
        }
    }

    header.close();
    return true;
}
