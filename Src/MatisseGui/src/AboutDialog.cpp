#include "AboutDialog.h"
#include "ui_AboutDialog.h"


using namespace MatisseServer;

AboutDialog::AboutDialog(QWidget *parent, KeyValueList meta) :
    QDialog(parent),
    _ui(new Ui::AboutDialog)
{
    _ui->setupUi(this);

    connect(_ui->_PB_closeAbout, SIGNAL(clicked()), this, SLOT(slot_close()));

    QString version = meta.getValue("version");

    QString templateText("<table>\
                         <tr><td colspan=\"2\"><h1>MATISSE 3D %1</h1></td></tr>\
                         <tr><td colspan=\"2\"><em>Technical support:</em></td></tr>\
                         <tr><td>Tel.:</td><td>+33(0)4 94 30 44 35</td></tr>\
                         <tr><td>Mail:</td><td><a href=\"mailto:aurelien.arnaubec@ifremer.fr\">aurelien.arnaubec@ifremer.fr</a></td></tr>\
                        </table>");
    QString mainText = templateText.arg(version);
    _ui->_LA_aboutMainText->setText(mainText);
}

void AboutDialog::slot_close()
{
    accept();
}

