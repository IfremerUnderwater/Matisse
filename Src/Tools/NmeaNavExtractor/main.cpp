#include "nmea_extractor_wizard.h"
#include <QApplication>
#include <QProcess>
#include <QDebug>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    NmeaExtractorWizard w;
    w.show();

    return a.exec();
}
