#include "Texturing3D.h"
#include "reconstructioncontext.h"
#include "NavImage.h"

#include <QFile>
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

void Texturing3D::writeKml(QString model_path, QString model_prefix)
{
    // Write kml for model
    QVariant *object = _context->getObject("reconstruction_context");
    if (object) {
        reconstructionContext * rc = object->value<reconstructionContext*>();
        QFile kml_file(model_path+QDir::separator()+model_prefix+QString(".kml"));
        if( !kml_file.open(QIODevice::WriteOnly) )
        {
            fatalErrorExit("Error saving " + model_prefix);
        }

        // Save kml info
        QTextStream outputGeoStream(&kml_file);

        outputGeoStream << QString("<kml>\n");
        outputGeoStream << QString("  <Placemark>\n");
        outputGeoStream << QString("    <name>%1</name>\n").arg(model_prefix);
        outputGeoStream << QString("    <Model>\n");
        outputGeoStream << QString("      <altitudeMode>absolute</altitudeMode>\n");
        outputGeoStream << QString("      <Location>\n");
        outputGeoStream << QString("        <longitude>%1</longitude>\n").arg(QString::number(rc->lat_origin, 'f', 8));
        outputGeoStream << QString("        <latitude>%1</latitude>\n").arg(QString::number(rc->lon_origin, 'f', 8));
        outputGeoStream << QString("        <altitude>%1</altitude>\n").arg(QString::number(rc->alt_origin, 'f', 3));
        outputGeoStream << QString("      </Location>\n");
        outputGeoStream << QString("      <Orientation>\n");
        outputGeoStream << QString("        <heading>0</heading>\n");
        outputGeoStream << QString("        <tilt>0</tilt>\n");
        outputGeoStream << QString("        <roll>0</roll>\n");
        outputGeoStream << QString("      </Orientation>\n");
        outputGeoStream << QString("      <Scale>\n");
        outputGeoStream << QString("        <x>1</x>\n");
        outputGeoStream << QString("        <y>1</y>\n");
        outputGeoStream << QString("        <z>1</z>\n");
        outputGeoStream << QString("      </Scale>\n");
        outputGeoStream << QString("      <Link>\n");
        outputGeoStream << QString("        <href>%1.obj</href>\n").arg(model_prefix);
        outputGeoStream << QString("      </Link>\n");
        outputGeoStream << QString("    </Model>\n");
        outputGeoStream << QString("  </Placemark>\n");
        outputGeoStream << QString("</kml>\n");

        kml_file.close();
    }
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

    QString completeOutPath = rootDirnameStr + SEP + outDirnameStr;

    QString fileNamePrefixStr = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

    // Get context
    QVariant *object = _context->getObject("reconstruction_context");
    reconstructionContext * rc=NULL;
    if (object)
        rc = object->value<reconstructionContext*>();

    for(unsigned int i=0; i<rc->components_ids.size(); i++)
    {

        // Convert model to mve
        emit signal_userInformation("Texturing3D convert");

        QProcess featuresProc;
        featuresProc.setWorkingDirectory(rootDirnameStr);
        featuresProc.start("openMVG_main_openMVG2MVE2 -i ."+SEP+ outDirnameStr+QString("_%1").arg(rc->components_ids[i]) +SEP+"sfm_data.bin -o ."+SEP+ outDirnameStr+QString("_%1").arg(rc->components_ids[i]) +SEP);

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
        cmdLine += " ."+SEP+ outDirnameStr+QString("_%1").arg(rc->components_ids[i]) +SEP+ MVE_EMBEDDED;
        cmdLine += "."+SEP+ outDirnameStr+QString("_%1").arg(rc->components_ids[i]) +SEP+fileNamePrefixStr+QString("_%1").arg(rc->components_ids[i]) + "_dense_mesh.ply ";
        cmdLine += "."+SEP+ outDirnameStr+QString("_%1").arg(rc->components_ids[i]) +SEP+fileNamePrefixStr+QString("_%1").arg(rc->components_ids[i]) + "_texrecon";
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

        // Write kml associated to model
        writeKml(completeOutPath+QString("_%1").arg(rc->components_ids[i]), fileNamePrefixStr+QString("_%1").arg(rc->components_ids[i]) + "_texrecon");

        emit signal_processCompletion(100);
        emit signal_userInformation("Texturing3D end");

    }

    // Flush next module port
    flush(0);

}

