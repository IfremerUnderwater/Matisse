#include "dim2_file_reader.h"

#define D2R (3.14159265358979323846 / 180.0)

namespace nav_tools {

Dim2FileReader::Dim2FileReader(QString filename, quint32 firstLine, quint32 lastLine, quint32 step):
    _filename(filename),
    _fileIsValid(false)
{
        readDim2File(firstLine, lastLine, step);
}

Dim2FileReader::~Dim2FileReader()
{
}

bool Dim2FileReader::readDim2File(quint32 firstLine, quint32 lastLine, quint32 step)
{

    Q_ASSERT( (firstLine>0) && (firstLine<=lastLine) );
    _fileIsValid = false;

    if (_filename.isEmpty()) {
        return false;
    }

    QFile fi(_filename);
    if (!fi.exists()) {
        qDebug() << "File does not exists: " << _filename;
        _fileIsValid = false;
        return false;
    }
    if (!fi.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file" << _filename;
        _fileIsValid = false;
        return false;
    }
    QTextStream reader(&fi);

    // lecture du fichier et remplissage des listes
   _fileIsValid = true;
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
            _fileIsValid = addDim2Line(newLine) && _fileIsValid;
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
                        D2R*dim2.yaw(),
                        D2R*dim2.roll(),
                        D2R*dim2.pitch(),
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

bool Dim2FileReader::addDim2Line(QString datasLine)
{
    datasLine = datasLine.simplified();
    if (datasLine.isEmpty()) {
        return false;
    }

    Dim2 newLine(datasLine);
    _dim2Lines.append(newLine);

    return true;
}

} // namespace nav_tools
