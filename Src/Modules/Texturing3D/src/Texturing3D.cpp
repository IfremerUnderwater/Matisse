#include "Texturing3D.h"
//#include "MosaicContext.h"
#include "NavImage.h"

#include <QProcess>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
static const char* TexreconExe = "texrecon.exe";
#define MVE_EMBEDDED "MVE==undistorted "
#else
static const char* TexreconExe = "texrecon";
#define MVE_EMBEDDED "MVE::undistorted "
#endif



#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(Texturing3D, Texturing3D)
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

Texturing3D::Texturing3D() :
    Processor(NULL, "Texturing3D", "T 3D points", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "output_filename");

    addExpectedParameter("algo_param", "keep_unseen_faces");
}

Texturing3D::~Texturing3D(){

}

bool Texturing3D::configure()
{
    return true;
}

void Texturing3D::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);
}

bool Texturing3D::start()
{
    setOkStatus();

    return true;
}

bool Texturing3D::stop()
{
    return true;
}

void Texturing3D::onFlush(quint32 port)
{
    Q_UNUSED(port)

    static const QString SEP = QDir::separator();

    emit signal_processCompletion(0);
    emit signal_userInformation("Texturing3D start");

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    QString outDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    if(outDirnameStr.isEmpty())
        outDirnameStr = "outReconstruction";

    QString fileNamePrefixStr = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    // Convert model to mve
    emit signal_userInformation("Texturing3D convert");

    QProcess featuresProc;
    featuresProc.setWorkingDirectory(rootDirnameStr);
    featuresProc.start("openMVG_main_openMVG2MVE2 -i ."+SEP+ outDirnameStr +SEP+"sfm_data.bin -o ."+SEP+ outDirnameStr +SEP);

    int starcount = 0;
    int lastpct = 0;
    while(featuresProc.waitForReadyRead(-1))
    {
        if(!isStarted())
        {
            featuresProc.kill();
            fatalErrorExit("openMVG2MVE2 Cancelled");
            return;
        }

        QString output = featuresProc.readAllStandardOutput();

        if(output.contains("Exporting views..."))
        {
            emit signal_userInformation("Text Convert : views");
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

    // Texture model
    emit signal_userInformation("Texturing3D texturing");

    QString cmdLine = TexreconExe;
    bool ok = false;
    bool keep_unseen_faces = _matisseParameters->getBoolParamValue("algo_param", "keep_unseen_faces", ok);
    if(ok && keep_unseen_faces)
        cmdLine += " --keep_unseen_faces";
    cmdLine += " ."+SEP+ outDirnameStr +SEP+ MVE_EMBEDDED;
    cmdLine += "."+SEP+ outDirnameStr +SEP+fileNamePrefixStr + "_dense_mesh.ply ";
    cmdLine += "."+SEP+ outDirnameStr +SEP+fileNamePrefixStr + "_texrecon";
    QProcess textureProc;
    textureProc.setWorkingDirectory(rootDirnameStr);
    textureProc.start(cmdLine);

    bool initer = false;
    while(textureProc.waitForReadyRead(-1)){
        if(!isStarted())
        {
            textureProc.kill();
            fatalErrorExit("texrecon Cancelled");
            return;
        }

        QString output = textureProc.readAllStandardOutput();

        if(output.contains("Load and prepare mesh:"))
        {
            // init
            emit signal_processCompletion((qint8)-1);
            emit signal_userInformation("texrecon : Load mesh...");
        }

        if(output.contains("Generating texture views:"))
        {
            emit signal_userInformation("texrecon : Gen. texture views...");
        }

        if(output.contains("Building adjacency graph:"))
        {
            emit signal_userInformation("texrecon : Build. adj graphs...");
        }

        if(output.contains("View selection:"))
        {
            emit signal_userInformation("texrecon : View selection...");
        }

        if(output.contains("\tCalculating face qualities "))
        {
            emit signal_userInformation("texrecon : calc face qual...");
        }

        if(output.contains("\tWriting data cost file"))
        {
            initer = true;
            emit signal_userInformation("texrecon : writing data cost...");
        }

        if(initer && output.startsWith("\t"))
        {
            // format
            // "    Comp	Iter	Energy		Runtime"
            // "	0000	0	1.10237e+06	0.017"
            QStringList list = output.mid(1).split("\t");
            if(list.count() > 2)
                emit signal_userInformation("texrecon : view sel. iter " + list.at(1));
        }

        if(output.contains("Generating texture patches:"))
        {
            initer = false;
            emit signal_userInformation("texrecon : gen text. patches...");
        }

        if(output.contains("Running global seam leveling:"))
        {
            initer = false;
            emit signal_userInformation("texrecon : global seam level...");
        }

        if(output.contains("\tAdjusting texture patches"))
        {
            initer = false;
            emit signal_userInformation("texrecon : adj text. patches...");
        }

        if(output.contains("Running local seam leveling:"))
        {
            initer = false;
            emit signal_userInformation("texrecon : local seam level...");
        }

        if(output.contains("Generating texture atlases:"))
        {
            initer = false;
            emit signal_userInformation("texrecon : gen. text. atlas...");
        }

        if(output.contains("\tFinalizing texture atlases"))
        {
            initer = false;
            emit signal_userInformation("texrecon : final. text. atlas...");
        }

        if(output.contains("Building objmodel:"))
        {
            initer = false;
            emit signal_userInformation("texrecon : build objmodel...");
        }


        qDebug() << output;
    }

    emit signal_processCompletion(100);
    emit signal_userInformation("Texturing3D end");

    // Flush next module port
    flush(0);

}

