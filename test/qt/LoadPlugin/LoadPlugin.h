#ifndef LOADPLUGIN_H
#define LOADPLUGIN_H

#include <QDialog>
#include <QPluginLoader>
#include <QList>
#include <QVariant>
#include <QtDebug>
#include <QMap>
#include <QIcon>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>

#include "PluginInterface.h"
#include "ChooseAndShow.h"

namespace Ui {
class LoadPlugin;
}

class LoadPlugin : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoadPlugin(QWidget *parent = 0);
    ~LoadPlugin();
    
    void load(QString libname);
    void startProcess(QPluginLoader *loader);

protected slots:
    void slot_startProcess();
    void slot_processorStarted();
    void slot_processorEnded();
    void slot_selectProcess(int noRow, int noCol);
    void slot_showLibs();
    void slot_customize();

private:
    Ui::LoadPlugin *ui;
    QMap<QPluginLoader*, PluginInterface*> _plugins;
    QIcon _redIcon;
    QIcon _greenIcon;
    QIcon _greenOffIcon;
    QString _libsDir;
};

#endif // LOADPLUGIN_H
