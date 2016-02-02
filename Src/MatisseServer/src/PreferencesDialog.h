#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "MatissePreferences.h"

#include <QDialog>
#include <QFileDialog>

#include <QtDebug>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent, MatissePreferences *prefs);
    ~PreferencesDialog();

protected:
    void changeEvent(QEvent *event); // overriding event handler for dynamic translation

private:
    Ui::PreferencesDialog *_ui;
    MatissePreferences* _prefs;
    QString _prefixBuffer;

protected slots:
    void slot_close();
    void slot_selectDir();
    void slot_validatePrefixInput();
    //void slot_restorePrefixInput(QString newText);
};

#endif // PREFERENCESDIALOG_H
