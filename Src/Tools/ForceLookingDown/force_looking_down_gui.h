#ifndef FORCELOOKINGDOWNGUI_H
#define FORCELOOKINGDOWNGUI_H

#include <QMainWindow>
#include "openMVG/numeric/eigen_alias_definition.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class ForceLookingDownGUI; }
QT_END_NAMESPACE

class ForceLookingDownGUI : public QMainWindow
{
    Q_OBJECT

public:
    ForceLookingDownGUI(QWidget *parent = nullptr);
    ~ForceLookingDownGUI();

private:
    void processSfmObjFile(QString _sfm_data_file, QString _obj_file);
    void transformObjFile(QString _obj_file, openMVG::Vec3 &t, openMVG::Mat3 &R, double &S);

    Ui::ForceLookingDownGUI *ui;

private slots:
    void sl_selectDataPath();
    void sl_processData();
};
#endif // FORCELOOKINGDOWNGUI_H
