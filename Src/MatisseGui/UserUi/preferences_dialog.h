#ifndef MATISSE_PREFERENCES_DIALOG_H_
#define MATISSE_PREFERENCES_DIALOG_H_

#include "matisse_preferences.h"
#include "matisse_icon_factory.h"
#include "iconized_button_wrapper.h"

#include <QDialog>
#include <QFileDialog>

#include <QtDebug>

namespace Ui {
class PreferencesDialog;
}

namespace matisse {

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent, MatisseIconFactory *iconFactory, MatissePreferences *prefs, bool allowProgrammingModeActivation = true);
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

} // namespace matisse

#endif // MATISSE_PREFERENCES_DIALOG_H_
