#include "SfmBundleAdjustment.h"
#include "reconstructioncontext.h"
#include <QProcess>

#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>


#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN2(SfmBundleAdjustment, SfmBundleAdjustment)
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

SfmBundleAdjustment::SfmBundleAdjustment() :
    Processor(NULL, "SfmBundleAdjustment", "Estimate camera position and 3D sparse points", 1, 1)
{
    addExpectedParameter("dataset_param", "dataset_dir");
    addExpectedParameter("dataset_param", "output_dir");
    addExpectedParameter("dataset_param", "usePrior");
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

void SfmBundleAdjustment::checkForNewFiles()
{
    QDir export_folder(m_out_complete_path_str);
    export_folder.setNameFilters(QStringList()<<"*.ply");
    QStringList file_list = export_folder.entryList();

    foreach(QString ply_file,file_list)
    {
        QFileInfo ply_file_info(m_out_complete_path_str+QDir::separator()+ply_file);
        QDateTime ply_last_mod = ply_file_info.lastModified();
        if (ply_last_mod>m_start_time && ply_last_mod>m_last_ply_time)
        {
            emit signal_show3DFileOnMainView(ply_file_info.absoluteFilePath());
            m_last_ply_time = ply_last_mod;
        }
    }
}

void SfmBundleAdjustment::splitMatchesFiles()
{
    static const QString SEP = QDir::separator();
    QDir splitted_matches_path_dir(m_splitted_matches_path);

    // Check if path exists
    if(!splitted_matches_path_dir.exists())
        splitted_matches_path_dir.mkpath(splitted_matches_path_dir.absolutePath());

    QProcess split_matches_proc;
    split_matches_proc.setWorkingDirectory(m_root_dirname_str);

    split_matches_proc.start("openMVG_main_SplitMatchFileIntoMatchFiles.exe -i ."+SEP+ "matches"+SEP+ "sfm_data.json -m ."
                             +SEP+ "matches"+SEP+"matches.f.bin -n 30 -o ."+SEP+"splitted_matches"+SEP+"matches_list.txt");

    // run process
    while(split_matches_proc.waitForReadyRead(-1))
    {
        QString output = split_matches_proc.readAllStandardOutput() + split_matches_proc.readAllStandardError();
        qDebug() << output;
    }

    // get resulting matches files
    QStringList name_filter = {"*.bin"};
    m_matches_files_list = splitted_matches_path_dir.entryList(name_filter);

}

bool SfmBundleAdjustment::start()
{
    setOkStatus();
    m_start_time = QDateTime::currentDateTime();
    m_last_ply_time = m_start_time;

    static const QString SEP = QDir::separator();

    // Dir checks
    m_root_dirname_str = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");

    m_out_dirname_str = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
    if(m_out_dirname_str.isEmpty())
        m_out_dirname_str = "outReconstruction";

    m_splitted_matches_path = m_root_dirname_str + SEP + QString("splitted_matches");

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
    QDir splitted_matches_path_dir(m_splitted_matches_path);
    QDir matches_path_dir(m_root_dirname_str + SEP + "matches");

    bool Ok;
    bool use_prior = _matisseParameters->getBoolParamValue("dataset_param", "usePrior",Ok);
    if(!Ok)
        use_prior = true;

    // Split matches into matches files
    splitMatchesFiles();

    // Get context
    QVariant *object = _context->getObject("reconstruction_context");
    reconstructionContext * rc=NULL;
    if (object)
        rc = object->value<reconstructionContext*>();

    // Reconstruct sparse for each connected components
    for (int i=0; i<m_matches_files_list.size(); i++)
    {

        // Get match file id
        QRegExp match_file_rex(".+_(\\d+)_(\\d+)");

        if (m_matches_files_list[i].contains(match_file_rex))
        {
            rc->components_ids.push_back(match_file_rex.cap(1).toInt());
        }
        else
        {
            fatalErrorExit("Match file not valid");
            return;
        }

        if (matches_path_dir.exists("matches.f.bin"))
        {
            QFile::remove(matches_path_dir.absoluteFilePath("matches.f.bin"));
        }

        // Copy to current match file
        QFile::copy(splitted_matches_path_dir.absoluteFilePath(m_matches_files_list[i]),
                    matches_path_dir.absoluteFilePath("matches.f.bin"));

        emit signal_processCompletion(0);
        emit signal_userInformation(QString("Sfm bundle adj %1/%2").arg(i+1).arg(m_matches_files_list.size()));

        // Fill out path
        m_out_complete_path_str = m_root_dirname_str + SEP + m_out_dirname_str+QString("_%1").arg(rc->components_ids[i]);

        // Compute Sfm bundle adjustment
        QString prior_arg("");
        if(use_prior)
            prior_arg = QString(" -P");

        QProcess sfmProc;
        sfmProc.setWorkingDirectory(m_root_dirname_str);
        sfmProc.start("openMVG_main_IncrementalSfM -i ."+SEP+ "matches"+SEP+ "sfm_data.json -m ."
                      +SEP+ "matches"+SEP+ " -o ."+SEP+ m_out_dirname_str+QString("_%1").arg(rc->components_ids[i]) + prior_arg);

        int starcount = 0;
        int lastpct = 0;
        while(sfmProc.waitForReadyRead(-1))
        {
            if(!isStarted())
            {
                sfmProc.kill();
                fatalErrorExit("Sfm bundle adj Cancelled");
                return;
            }

            // detect steps
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
                // how to get the number of views ???? - no more progression.
                // showing the n° of the view
                signal_userInformation("Sfm bndl : view " + substr);
            }
            checkForNewFiles();
            qDebug() << output;
        }

        emit signal_processCompletion(100);
        emit signal_userInformation("Sfm bundle adj. ended");

    }

    // Flush next module port
    flush(0);

}

