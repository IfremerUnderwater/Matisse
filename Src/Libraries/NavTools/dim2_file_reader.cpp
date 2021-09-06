#include "dim2_file_reader.h"

#define D2R (3.14159265358979323846 / 180.0)

namespace nav_tools {

Dim2FileReader::Dim2FileReader(QString _filename, quint32 _first_line, quint32 _last_line, quint32 _step):
    m_filename(_filename),
    m_file_is_valid(false)
{
        readDim2File(_first_line, _last_line, _step);
}

Dim2FileReader::~Dim2FileReader()
{
}

bool Dim2FileReader::readDim2File(quint32 _first_line, quint32 _last_line, quint32 _step)
{

    Q_ASSERT( (_first_line>0) && (_first_line<=_last_line) );
    m_file_is_valid = false;

    if (m_filename.isEmpty()) {
        return false;
    }

    QFile fi(m_filename);
    if (!fi.exists()) {
        qDebug() << "File does not exists: " << m_filename;
        m_file_is_valid = false;
        return false;
    }
    if (!fi.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file" << m_filename;
        m_file_is_valid = false;
        return false;
    }
    QTextStream reader(&fi);

    // lecture du fichier et remplissage des listes
   m_file_is_valid = true;
   m_dim2_lines.clear();
   quint32 line_count = 0;
           // on suppose que la premiere ligne, correspondant à l'index est 1
    while (!reader.atEnd()) {
        QString newLine = reader.readLine();
        line_count++;
        // test borne min et step
        if ((line_count >= _first_line) && ((line_count-_first_line)% _step == 0)) {
            // test borne max
            if (line_count > _last_line) {
                break;
            }
            m_file_is_valid = addDim2Line(newLine) && m_file_is_valid;
        }
    }

    fi.close();

    return m_file_is_valid;
}

NavInfo Dim2FileReader::getNavInfo(quint32 _image_id)
{
    NavInfo nav_info;
    if ((quint32)m_dim2_lines.length() > _image_id) {
        Dim2 dim2 = m_dim2_lines.at(_image_id);
        nav_info.setInfo(dim2.diveNumber(),
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

    return nav_info;
}

int Dim2FileReader::getNumberOfImages()
{
    return m_dim2_lines.length();
}

QString Dim2FileReader::getImageFilename(quint32 _image_id)
{
    QString filename;
    if ((quint32)m_dim2_lines.length() > _image_id ) {
        filename = m_dim2_lines.at(_image_id).filename();
    }

    return filename;
}

QString Dim2FileReader::getImageSource(quint32 _image_id)
{
    QString src;
    if ((quint32)m_dim2_lines.length() > _image_id ) {
        src = m_dim2_lines.at(_image_id).source();
    }

    return src;
}

QString Dim2FileReader::getImageFormat(quint32 _image_id)
{
    QString format;
    if ((quint32)m_dim2_lines.length() > _image_id ) {
        format = m_dim2_lines.at(_image_id).format();
    }

    return format;
}

bool Dim2FileReader::isFileValid()
{
    return m_file_is_valid;
}

QString Dim2FileReader::getFilename()
{
    return m_filename;
}

bool Dim2FileReader::addDim2Line(QString _data_line)
{
    _data_line = _data_line.simplified();
    if (_data_line.isEmpty()) {
        return false;
    }

    Dim2 newLine(_data_line);
    m_dim2_lines.append(newLine);

    return true;
}

} // namespace nav_tools
