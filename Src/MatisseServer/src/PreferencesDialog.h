#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "MatissePreferences.h"
#include "MatisseIconFactory.h"
#include "IconizedButtonWrapper.h"

#include <QDialog>
#include <QFileDialog>

#include <QtDebug>

using namespace MatisseTools;

namespace Ui {
class PreferencesDialog;
}

namespace MatisseServer {
class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent, MatisseIconFactory *iconFactory, MatissePreferences *prefs);
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
}

#endif // PREFERENCESDIALOG_H
