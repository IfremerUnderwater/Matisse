#include "SfmBundleAdjustment.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include <QProcess>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>


Q_EXPORT_PLUGIN2(SfmBundleAdjustment, SfmBundleAdjustment)

// traiter les étoiles (total 51 étoiles pour 100 %)
// 0%   10   20   30   40   50   60   70   80   90   100%
// |----|----|----|----|----|----|----|----|----|----|
// ***************************************************
// entrée : message
// mis à jour : starcount
// retourne : le pourcentage d'avancement
static int progressStarCountPct(QString message, int &starcount)
{
    int n = message.count("*");
    starcount += n;
    double pct = starcount / 51.0 * 100.0;
    if(starcount > 51)
        starcount %= 52;
    return int(pct);
}

SfmBundleAdjustment::SfmBundleAdjustment() :
    Processor(NULL, "SfmBundleAdjustment", "Estimate camera position and 3D sparse points", 1, 1)
{
    addExpectedParameter("dataset_param", "dataset_dir");
}

SfmBundleAdjustment::~SfmBundleAdjustment(){

}

bool SfmBundleAdjustment::configure()
{
    return true;
}

void SfmBundleAdjustment::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);
}

bool SfmBundleAdjustment::start()
{
    setOkStatus();

    return true;
}

bool SfmBundleAdjustment::stop()
{
    return true;
}

void SfmBundleAdjustment::onFlush(quint32 port)
{
    Q_UNUSED(port)

    static const QString SEP = QDir::separator();

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    emit signal_processCompletion(0);
    emit signal_userInformation("Compute Sfm bundle adj");

    // Compute Sfm bundle adjustment
    QProcess sfmProc;
    sfmProc.setWorkingDirectory(rootDirnameStr);
    sfmProc.start("openMVG_main_IncrementalSfM -i ."+SEP+ "matches"+SEP+ "sfm_data.json -m ."
                  +SEP+ "matches"+SEP+ " -o ."+SEP+ "outReconstruction"+SEP);

    int starcount = 0;
    int lastpct = 0;
    while(sfmProc.waitForReadyRead(-1))
    {
        if(!isStarted())
        {
            sfmProc.kill();
            fatalErrorExit("Compute Sfm bundle adj Cancelled");
            return;
        }

        // détecter les étapes
        QString output = sfmProc.readAllStandardOutput();
        if(output.contains("- Features Loading -"))
        {
            emit signal_userInformation("Sfm bndl adj : EXTRACT FEATURES");
            starcount = 0;
        }
        if(output.contains("Automatic selection of an initial pair:"))
        {
            emit signal_userInformation("Sfm bndl adj : sel. init pair");
            starcount = 0;
        }

        // traiter les étoiles
        int pct = progressStarCountPct(output, starcount);
        if(pct != lastpct)
        {
            emit signal_processCompletion(pct);
            lastpct = pct;
        }

        if(output.contains("MSE Residual InitialPair"))
        {
            // init
            emit signal_processCompletion((qint8)-1);
        }
        if(output.contains("-- Robust Resection of view:"))
        {
            int pos = output.indexOf("-- Robust Resection of view:");
            QString substr = output.mid(pos);
            pos = substr.indexOf("\n");
            if(pos != -1)
                substr = substr.left(pos);
            pos = substr.indexOf(":");
            if(pos != -1)
                substr = substr.mid(pos);
            // comment avoir le nombre de vues ici ???? - on ne fait plus de progression.
            // uniquement affichage du n° de la vue traitée (ordre quelconque)
            signal_userInformation("Compute Sfm bndl : view " + substr);
        }

        qDebug() << output;
    }

    emit signal_processCompletion(100);
    emit signal_userInformation("Compute Sfm bundle adj. ended");

    // Flush next module port
    flush(0);

}

