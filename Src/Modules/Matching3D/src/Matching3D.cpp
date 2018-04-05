#include "Matching3D.h"
//#include "MosaicContext.h"
#include "NavImage.h"

#include <QProcess>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Matching3D, Matching3D)
#endif
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

Matching3D::Matching3D() :
    Processor(NULL, "Matching3D", "Match images and filter with geometric transformation", 1, 1)
{
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("algo_param", "force_recompute");
    addExpectedParameter("algo_param", "describer_method");
    addExpectedParameter("algo_param", "describer_preset");
    addExpectedParameter("algo_param", "nearest_matching_method");
    addExpectedParameter("algo_param", "video_mode_matching");
}

Matching3D::~Matching3D(){

}

bool Matching3D::configure()
{
    return true;
}

void Matching3D::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);
}

bool Matching3D::start()
{
    setOkStatus();

    return true;
}

bool Matching3D::stop()
{
    return true;
}

void Matching3D::onFlush(quint32 port)
{
    Q_UNUSED(port)

    static const QString SEP = QDir::separator();

    emit signal_processCompletion(0);
    emit signal_userInformation("Match 3D : Compute Features");

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    bool ok = true;
    bool force_recompute = _matisseParameters->getBoolParamValue("algo_param", "force_recompute", ok);

    // get nb of threads
    int nbthreads = QThread::idealThreadCount();
    QString threadsParam = " -n ";
    threadsParam += QString::number(nbthreads);

    // describer preset parameter
    QString presetParam = " -p ";
    QString presetParamval = _matisseParameters->getStringParamValue("algo_param", "describer_preset");
    presetParam += presetParamval;

    QString commandLine = "openMVG_main_ComputeFeatures -i ."
            +SEP + "matches" +SEP+ "sfm_data.json -o ."
            +SEP + "matches";
    if(force_recompute)
        commandLine += " -f 1";
    commandLine += threadsParam;
    commandLine += presetParam;

    // Compute Features
    QProcess computeFeatureProcess;
    computeFeatureProcess.setWorkingDirectory(rootDirnameStr);
    computeFeatureProcess.start(commandLine);
    int starcount = 0;
    int lastpct = 0;
    while(computeFeatureProcess.waitForReadyRead(-1)){
        if(!isStarted())
        {
            computeFeatureProcess.kill();
            fatalErrorExit("ComputeFeatures Cancelled");
            return;
        }

        // détecter les étapes
        QString output = computeFeatureProcess.readAllStandardOutput();
        if(output.contains("- EXTRACT FEATURES -"))
        {
            emit signal_userInformation("Match 3D : EXTRACT FEATURES");
            starcount = 0;
        }

        // traiter les étoiles
        int pct = progressStarCountPct(output, starcount);
        if(pct != lastpct)
        {
            emit signal_processCompletion(pct);
            lastpct = pct;
        }
        qDebug() << output;
    }

    emit signal_userInformation("Match 3D : Compute Matches");
    emit signal_processCompletion(0);

    // Compute Matches
    commandLine = "openMVG_main_ComputeMatches -i ."
            +SEP+ "matches"+SEP+ "sfm_data.json -o ."+SEP+ "matches";
    if(force_recompute)
        commandLine += " -f 1";
    // nearest matching method
    QString nmmParam = " -n ";
    QString nmmParamValue = _matisseParameters->getStringParamValue("algo_param", "nearest_matching_method");
    nmmParam += nmmParamValue;

    commandLine += nmmParam;

    // video mode matching
    QString vmmParam = " -v ";
    ok = true;
    int vmmParamVal = _matisseParameters->getIntParamValue("algo_param", "video_mode_matching", ok);
    vmmParam +=  QString::number(vmmParamVal);
    if(ok)
        commandLine += vmmParam;

    QProcess matchingProcess;
    matchingProcess.setWorkingDirectory(rootDirnameStr);
    matchingProcess.start(commandLine);

    starcount = 0;
    lastpct = 0;
    while(matchingProcess.waitForReadyRead(-1)){
        if(!isStarted())
        {
            matchingProcess.kill();
            fatalErrorExit("ComputeMatches Cancelled");
            return;
        }
        QString output = matchingProcess.readAllStandardOutput();

        // détecter les étapes
        if(output.contains("- Regions Loading -"))
        {
            emit signal_userInformation("Match 3D : Regions Loading");
            starcount = 0;
        }
        if(output.contains("- Matching -"))
        {
            emit signal_userInformation("Match 3D : Matching");
            starcount = 0;
        }
        if(output.contains("- Geometric filtering -"))
        {
            emit signal_userInformation("Match 3D : Geometric filtering");
            starcount = 0;
        }

        // traiter les étoiles
        int pct = progressStarCountPct(output, starcount);
        if(pct != lastpct)
        {
            emit signal_processCompletion(pct);
            lastpct = pct;
        }

        qDebug() << output;
    }

    emit signal_processCompletion(100);
    emit signal_userInformation("Match 3D - ended");

    // Flush next module port
    flush(0);

}
