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
    explicit PreferencesDialog(QWidget *_parent, MatisseIconFactory *_icon_factory, MatissePreferences *_prefs, bool _allow_programming_mode_activation = true);
    ~PreferencesDialog();

protected:
    void changeEvent(QEvent *_event); // overriding event handler for dynamic translation

private:
    Ui::PreferencesDialog *m_ui;
    MatissePreferences* m_prefs;
    QString m_prefix_buffer;

protected slots:
    void sl_close();
    void sl_selectDir();
    void sl_validatePrefixInput();
};

} // namespace matisse

#endif // MATISSE_PREFERENCES_DIALOG_H_
