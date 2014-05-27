#include "Dim2FileReader.h"
using namespace MatisseCommon;

// on n'utilise pas la regexp car le modèle fourni ne correspond pas à la définition...
//QRegExp Dim2FileReader::_lineRegExp("^;(\\d{2}\\/){2}\\d{4}\\s;(\\d{2}:){2}\\d{2}\\.\\d{3}(\\s;){2}\\s\w+;\\s\\w+([\\w\\-])+\\.\\w+;(\\s\\-?\\d+\\.\\d+;){7}(\\s;){13}$");

Dim2FileReader::Dim2FileReader(QString filename, quint32 firstLine, quint32 lastLine, quint32 step):
    _filename(filename),
    _fileIsValid(false)
{
        readDim2File(filename, firstLine, lastLine, step);
}

Dim2FileReader::~Dim2FileReader()
{
}

bool Dim2FileReader::readDim2File(QString filename, quint32 firstLine, quint32 lastLine, quint32 step)
{
    // au cas où...
    if (firstLine == 0) {
        firstLine = 1;
    }
    _filename = filename.simplified();

    if (_filename.isEmpty()) {
        return false;
    }

    QFile fi(_filename);
    if (!fi.exists()) {
        qDebug() << "File not exists:" << _filename;
        _fileIsValid = false;
        return false;
    }
    if (!fi.open(QIODevice::ReadOnly)) {
        qDebug() << "File not opened" << _filename;
        _fileIsValid = false;
        return false;
    }
    QTextStream reader(&fi);

    // lecture du fichier et remplissage des listes
   _fileIsValid = false;
   _dim2Lines.clear();
   quint32 lineCount = 0;
           // on suppose que la premiere ligne, correspondant à l'index est 1
    while (!reader.atEnd()) {
        QString newLine = reader.readLine();
        lineCount++;
        // test borne min et step
        if ((lineCount >= firstLine) && ((lineCount-firstLine)% step == 0)) {
            // test borne max
            if (lineCount > lastLine) {
                break;
            }
            _fileIsValid = addInfo(newLine) && _fileIsValid;
        }
    }

    fi.close();

    return _fileIsValid;
}

NavInfo Dim2FileReader::getNavInfo(quint32 imageId)
{
    NavInfo navInfo;
    if ((quint32)_dim2Lines.length() > imageId) {
        Dim2 dim2 = _dim2Lines.at(imageId);
        navInfo.setInfo(dim2.diveNumber(),
                        dim2.dateTime(),
                        dim2.longitude(),
                        dim2.latitude(),
                        dim2.depth(),
                        dim2.altitude(),
                        dim2.yaw(),
                        dim2.roll(),
                        dim2.pitch(),
                        dim2.vx(),
                        dim2.vy(),
                        dim2.vz());
    }

    return navInfo;
}

int Dim2FileReader::getNumberOfImages()
{
    return _dim2Lines.length();
}

QString Dim2FileReader::getImageFilename(quint32 imageId)
{
    QString filename;
    if ((quint32)_dim2Lines.length() > imageId ) {
        filename = _dim2Lines.at(imageId).filename();
    }

    return filename;
}

QString Dim2FileReader::getImageSource(quint32 imageId)
{
    QString src;
    if ((quint32)_dim2Lines.length() > imageId ) {
        src = _dim2Lines.at(imageId).source();
    }

    return src;
}

QString Dim2FileReader::getImageFormat(quint32 imageId)
{
    QString format;
    if ((quint32)_dim2Lines.length() > imageId ) {
        format = _dim2Lines.at(imageId).format();
    }

    return format;
}

bool Dim2FileReader::isFileValid()
{
    return _fileIsValid;
}

QString Dim2FileReader::getFilename()
{
    return _filename;
}

bool Dim2FileReader::addInfo(QString datasLine)
{
    // TODO: decodage fichier....
    datasLine = datasLine.simplified();
    if (datasLine.isEmpty()) {
        return false;
    }

    Dim2 newLine(datasLine);
    _dim2Lines.append(newLine);

    // ancien code....
    // on garde pour l'instant....
//    if (args.length() < 16) {
//        qDebug() << "Dim2FileReader: no more args" << datasLine;
//        return false;
//    }
//    // on enleve les infos fichier et on reformate la date
//    // on force le dive si non present dans le fichier...
//    if (args[0].trimmed().isEmpty()) {
//        args[0] = QString("%1").arg(_diveNumber);
//    }
//    QString imageSource = args[3].trimmed();
//    QString imageType = args[4].trimmed();
//    QString imageFilename = args[5].trimmed();
//    args[1].append(args[2]);
//    args.removeAt(5);
//    args.removeAt(4);
//    args.removeAt(3);
//    args.removeAt(2);
//    NavInfo newNavInfo;
//    newNavInfo.setInfo(args.mid(0, 13));

//    // Pour l'instant, le fichier fourni ne contient pas de valeurs vx, vy et vz...
//    if (!newNavInfo.isValid("111111111000")) {
//        qDebug() << "Dim2FileReader: invalid args" << args;
//        return false;
//    }
//        _navInfoList.append(NavInfo());
//        _imagesNamesList << imageFilename;

    return true;
}
