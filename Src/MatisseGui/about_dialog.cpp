#include "about_dialog.h"
#include "ui_about_dialog.h"


namespace matisse {

AboutDialog::AboutDialog(QWidget *_parent, KeyValueList _meta) :
    QDialog(_parent),
    m_ui(new Ui::AboutDialog)
{
    m_ui->setupUi(this);

    connect(m_ui->_PB_closeAbout, SIGNAL(clicked()), this, SLOT(sl_close()));

    QString version = _meta.getValue("version");

    QString template_text("<table>\
                         <tr><td colspan=\"2\"><h1>MATISSE 3D %1</h1></td></tr>\
                         <tr><td colspan=\"2\"><em>Technical support:</em></td></tr>\
                         <tr><td>Mail:</td><td><a href=\"mailto:aurelien.arnaubec@ifremer.fr\">aurelien.arnaubec@ifremer.fr</a></td></tr>\
                         <tr><td>Mail:</td><td><a href=\"mailto:maxime.ferrera@ifremer.fr\">maxime.ferrera@ifremer.fr</a></td></tr>\
                        </table>");
    QString main_text = template_text.arg(version);
    m_ui->_LA_aboutMainText->setText(main_text);
}

void AboutDialog::sl_close()
{
    accept();
}

} // namespace matisse

