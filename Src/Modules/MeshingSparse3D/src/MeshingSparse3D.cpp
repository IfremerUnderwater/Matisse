#include "MeshingSparse3D.h"
#include "reconstructioncontext.h"
#include "NavImage.h"

#include <QProcess>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(MeshingSparse3D, MeshingSparse3D)
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

// type
// "Estimated depth-maps 1 (6.25%, 499ms, ETA 7s)..."
//
static double getPctVal(QString message, QString key)
{
    int pos = message.indexOf(key);
    QString substr = message.mid(pos);
    pos = substr.indexOf("\n");
    if(pos != -1)
        substr = substr.left(pos);
    pos = substr.indexOf("(");
    if(pos != -1)
        substr = substr.mid(pos+1);
    pos = substr.indexOf("%");
    if(pos != -1)
        substr = substr.left(pos);
    bool ok;
    double val = substr.toDouble(&ok);

    if(ok)
        return val;
    else
        return -1;
}

#ifdef _WIN32
static const char* ReconstructMeshExe = "ReconstructMesh.exe";
#else
static const char* ReconstructMeshExe = "ReconstructMesh";
#endif

MeshingSparse3D::MeshingSparse3D() :
    Processor(NULL, "MeshingSparse3D", "Create a mesh from 3D points", 1, 1)
{

    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "output_filename");

    addExpectedParameter("algo_param", "resolution_level");
    addExpectedParameter("algo_param", "number_views_fuse");
    addExpectedParameter("algo_param", "decimate_factor");
}

MeshingSparse3D::~MeshingSparse3D(){

}

bool MeshingSparse3D::configure()
{
    return true;
}

void MeshingSparse3D::onNewImage(quint32 port, Image &image)
{
    Q_UNUSED(port)

    // Forward image
    postImage(0, image);

}

bool MeshingSparse3D::start()
{
    setOkStatus();

    return true;
}

bool MeshingSparse3D::stop()
{
    return true;
}

void MeshingSparse3D::onFlush(quint32 port)
{
    Q_UNUSED(port)

    static const QString SEP = QDir::separator();

    emit signal_processCompletion(0);
    emit signal_userInformation("MeshingSparse3D - start");

    // Dir checks
    QString rootDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    QString outDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    if(outDirnameStr.isEmpty())
        outDirnameStr = "outReconstruction";

    QString fileNamePrefixStr = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    // Get context
    QVariant *object = _context->getObject("reconstruction_context");
    reconstructionContext * rc=NULL;
    if (object)
        rc = object->value<reconstructionContext*>();

    for (unsigned int i=0; i<rc->components_ids.size(); i++)
    {

        // convert
        emit signal_userInformation("MeshingSparse3D - convert");
        emit signal_processCompletion(0);

        QString cmdline = "openMVG_main_openMVG2openMVS -i ."+SEP+ outDirnameStr+QString("_%1").arg(rc->components_ids[i]) +SEP+ "sfm_data.bin -o ";
        cmdline += "."+SEP+ outDirnameStr+QString("_%1").arg(rc->components_ids[i]) +SEP+ fileNamePrefixStr+QString("_%1").arg(rc->components_ids[i]) + "_dense.mvs";
        QProcess cvtProc;
        cvtProc.setWorkingDirectory(rootDirnameStr);
        cvtProc.start(cmdline);

        int starcount = 0;
        int lastpct = 0;
        while(cvtProc.waitForReadyRead(-1)){

            if(!isStarted())
            {
                cvtProc.kill();
                fatalErrorExit("openMVG2openMVS Cancelled");
                return;
            }

            QString output = cvtProc.readAllStandardOutput();

            // traiter les étoiles
            int pct = progressStarCountPct(output, starcount);
            if(pct != lastpct)
            {
                emit signal_processCompletion(pct);
                lastpct = pct;
            }

            qDebug() << output;
        }

        // Compute Mesh
        emit signal_userInformation("MeshingSparse3D - Compute Mesh");
        emit signal_processCompletion(0);

        cmdline = ReconstructMeshExe;
        bool ok;
        double decimatearg = _matisseParameters->getDoubleParamValue("algo_param", "decimate_factor", ok);
        if(ok)
            cmdline += " --decimate " + QString::number(decimatearg);

        cmdline +=  " ."+SEP+ outDirnameStr+QString("_%1").arg(rc->components_ids[i]) +SEP+ fileNamePrefixStr+QString("_%1").arg(rc->components_ids[i]) + "_dense.mvs";
        QProcess meshProc;
        meshProc.setWorkingDirectory(rootDirnameStr);
        meshProc.start(cmdline);

        while(meshProc.waitForReadyRead(-1)){

            if(!isStarted())
            {
                meshProc.kill();
                fatalErrorExit("ReconstructMesh Cancelled");
                return;
            }

            QString output = meshProc.readAllStandardOutput();

            // étape "Points inserted "
            if(output.contains("Points inserted "))
            {
                double val = getPctVal(output,"Points inserted ");
                if(val >= 0)
                {
                    emit signal_userInformation("Rec. Mesh - Points inserted");
                    emit signal_processCompletion((int)val);
                }
            }

            // étape "Points weighted "
            if(output.contains("Points weighted "))
            {
                double val = getPctVal(output,"Points weighted ");
                if(val >= 0)
                {
                    emit signal_userInformation("Rec. Mesh - Points weighted");
                    emit signal_processCompletion((int)val);
                }
            }

            if(output.contains("Delaunay tetrahedras weighting completed:"))
            {
                emit signal_processCompletion((quint8)-1);
                emit signal_userInformation("Rec. Mesh - tetra. weighting");
            }

            if(output.contains("Delaunay tetrahedras graph-cut completed"))
            {
                emit signal_processCompletion((quint8)-1);
                emit signal_userInformation("Rec. Mesh - tetra. graph-cut");
            }

            if(output.contains("Mesh reconstruction completed:"))
            {
                emit signal_processCompletion((quint8)-1);
                emit signal_userInformation("Rec. Mesh - reconstr. complete");
            }

            qDebug() << output;
        }

        emit signal_userInformation("MeshingSparse3D - end");
        emit signal_processCompletion(100);

    }

    // Flush next module port
    flush(0);

}

