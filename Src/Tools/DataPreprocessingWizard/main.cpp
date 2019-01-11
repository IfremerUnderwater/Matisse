#include "data_preprocessing_wizard.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DataPreprocessingWizard w;
    w.show();

    return a.exec();
}
