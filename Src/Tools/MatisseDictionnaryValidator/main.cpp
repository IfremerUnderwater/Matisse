#include "dictionnary_validator.h"
#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    DictionnaryValidator w;
    w.show();

    return a.exec();
}
