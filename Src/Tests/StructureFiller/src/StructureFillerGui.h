#ifndef STRUCTUREFILLERGUI_H
#define STRUCTUREFILLERGUI_H

#include <QMainWindow>
#include <QModelIndex>
#include <QFileDialog>
#include <QWidget>
#include <QMessageBox>

#include "MatisseTools.h"

namespace Ui {
class StructureFillerGui;
}

class StructureFillerGui : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit StructureFillerGui(QWidget *parent = 0);
    ~StructureFillerGui();
    
    void init();

private:
    Ui::StructureFillerGui *ui;
    MatisseTools * _wrapper;
    QWidget * _currentWidget;
    QString _currentStructName;

    QString _xmlPath;

    protected slots:
        void slot_selectStructure(QModelIndex index);
        void slot_quit();
        void slot_openXML();
        void slot_openParametersFile();
        void slot_saveParametersFile();
        void slot_showUserParams(bool user);
};

#endif // STRUCTUREFILLERGUI_H
