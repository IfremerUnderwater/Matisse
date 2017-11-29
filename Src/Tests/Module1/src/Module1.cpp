#include "Module1.h"
#include "MosaicContext.h"
#include "NavImage.h"
#include "GeoTransform.h"
#include "RasterGeoreferencer.h"

#include "MosaicDescriptor.h"
#include "MosaicDrawer.h"

#include "Polygon.h"

#ifdef WIN32
#include <windows.h>
#endif

// Exportation de la classe Module1 dans la bibliotheque de plugin TestModule1
Q_EXPORT_PLUGIN2(Module1, Module1)


Module1::Module1() :
    Processor(NULL, "Module1", "Module d'essai", 1, 2)
{
    addExpectedParameter("algo_param", "sift_code");
    addExpectedParameter("algo_param", "min_matches");
    addExpectedParameter("algo_param", "max_matches");
    addExpectedParameter("algo_param", "max_overlap");
    addExpectedParameter("algo_param", "max_Pitch");
    //addExpectedParameter("algo_param", "ba_method");
}

bool Module1::configure()
{
    return true;
}

void Module1::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;
    QString sift_code = _matisseParameters->getStringParamValue("algo_param", "sift_code");
    qDebug() << logPrefix() << "sift_code: " << sift_code;
    quint8 count = 0;
    while (count < 5) {
        qDebug() << logPrefix() << "COUNT=" << count;
        count++;
    }

    GeoTransform TGeo;
    NavImage * ImageNav = (NavImage*)(&image);
    qDebug()<< ImageNav->navInfo().dump();

    double x,y,lat,lon;
    QString utmZone;
    TGeo.LatLongToUTM(ImageNav->navInfo().latitude(),ImageNav->navInfo().longitude(),x,y,utmZone);
    qDebug()<< "X = " << x << " Y = " << y << " UTMzone = " << utmZone;

    TGeo.UTMToLatLong(x,y,utmZone,lat,lon);
    qDebug()<< "lat = " << lat << " lon = " << lon ;

    // Forward image
    postImage(1, image);
}

bool Module1::start()
{
    setOkStatus();

    qDebug() << logPrefix() << "Enter start";

    MosaicData * pm = new MosaicData();
    QVariant * v = new QVariant();

    pm->init.filename = QString("Test\n");
    v->setValue(pm);

    _context->addObject("MosaicData", v);
    qDebug() << logPrefix() << "Exit start";
    return true;
}



bool Module1::stop()
{
    qDebug() << logPrefix() << "Inside stop";
    return true;
}


void Module1::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "flush port " << port;
    qDebug() << logPrefix() << "OPTIM" ;

    basicproc::Polygon P1,P2,P3;

    std::vector<double> x,y;

    // Construct P1
    x.push_back(0); x.push_back(0); x.push_back(1); x.push_back(1);
    y.push_back(0); y.push_back(1); y.push_back(1); y.push_back(0);
    P1.addContour(x,y);
    x.clear(); y.clear();

    // Construct P2
    x.push_back(0.5); x.push_back(0.5); x.push_back(1.5); x.push_back(1.5);
    y.push_back(0); y.push_back(1); y.push_back(1); y.push_back(0);
    P2.addContour(x,y);
    x.clear(); y.clear();

    P1.clip(P2, P3, basicproc::INT);

    // Test polygon plotting
    /*emit signal_addPolygonToMap(P3,"blue","P3");
    emit signal_addPolygonToMap(P1,"red","P1");
    emit signal_addPolygonToMap(P2,"green","P2");*/

    //Test 3D file reading
#ifdef WIN32
    QString filePath("F:\\DATA\\3D_DATASETS\\Morph\\rock02_corr\\rock02_dense.nvm.cmvs\\00\\models\\wallMeshTex.obj");
    qDebug() << logPrefix() << filePath;
    emit signal_show3DFileOnMainView(filePath);
    Sleep(60000);
#else
    emit signal_show3DFileOnMainView("./3DTestData/wallMeshTex.obj");
    sleep(60);
#endif

    /*    MosaicDescriptor mosaicD;
    QVector<ProjectiveCamera*> cams;
    ImageSet * imageSet;

    // Find imageSet corresponding to this port
    foreach (ImageSetPort *ImSet,*_inputPortList){
        if (ImSet->portNumber == port){
            imageSet = ImSet->imageSet;
            break;
        }
    }

    // Get camera matrix
    bool Ok;
    QMatrix3x3 qK = _matisseParameters->getMatrix3x3ParamValue("cam_param",  "K",  Ok);
    cv::Mat K(3,3,CV_64F);
    if (Ok){
        qDebug() << "K Ok =" << Ok;

        for (int i=0; i<3; i++){
            for(int j=0; j<3; j++){
                K.at<double>(i,j) = qK(i,j);
            }
        }

    }else{
        qDebug() << "K Ok =" << Ok;
        exit(1);
    }

    std::cerr <<"K = " << K;

    double scaleFactor = _matisseParameters->getDoubleParamValue("algo_param", "scale_factor", Ok);

    if (!Ok){
        qDebug() << "scale factor not provided Ok \n";
        exit(1);
    }

    // Get camera lever arm
    cv::Mat V_T_C(3,1,CV_64F), V_R_C(3,3,CV_64F);

    Matrix6x1 V_Pose_C = _matisseParameters->getMatrix6x1ParamValue("cam_param",  "V_Pose_C",  Ok);
    if (Ok){

        for (int i=0; i<3; i++){
            V_T_C.at<double>(i,0) = V_Pose_C(0,i);
        }

        GeoTransform T;

        V_R_C = T.RotZ(V_Pose_C(0,5))*T.RotY(V_Pose_C(0,4))*T.RotX(V_Pose_C(0,3));

    }


    if (imageSet){

        // Create cameras
        QList<Image *> imageList = imageSet->getAllImages();
        foreach (Image* image, imageList) {

            NavImage *navImage = dynamic_cast<NavImage*>(image);
            if (navImage){
                cams.push_back(new ProjectiveCamera(navImage , K, V_T_C, V_R_C, (double)scaleFactor));
            }else{
                qDebug() << "cannot cast as navImage \n";
                exit(1);
            }
        }

        // Init cameras
        mosaicD.initCamerasAndFrames(cams,true);
        qDebug() << "Init done";

        // Compute extents
        mosaicD.computeMosaicExtentAndShiftFrames();
        qDebug() << "Extent computation done";

        //Draw mosaic
        MosaicDrawer mosaicDrawer;
        cv::Mat mosaicImage,mosaicMask;
        mosaicDrawer.drawAndBlend(mosaicD, mosaicImage, mosaicMask);

        // Write geofile
        QString datasetDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "dataset_dir");
        QString outputDirnameStr = _matisseParameters->getStringParamValue("dataset_param", "output_dir");
        QString outputFilename = _matisseParameters->getStringParamValue("dataset_param", "output_filename");

        if (outputDirnameStr.isEmpty()
                || datasetDirnameStr.isEmpty()
                || outputFilename.isEmpty())
            return;

        QFileInfo outputDirInfo(outputDirnameStr);
        QFileInfo datasetDirInfo(datasetDirnameStr);

        bool isRelativeDir = outputDirInfo.isRelative();

        if (isRelativeDir) {
            outputDirnameStr = QDir::cleanPath( datasetDirInfo.absoluteFilePath() + QDir::separator() + outputDirnameStr);
        }

        qDebug() << "output_dir = " << outputDirnameStr;
        qDebug() << "output_filename = " << outputFilename;

        mosaicD.writeToGeoTiff(mosaicImage,mosaicMask,outputDirnameStr + QDir::separator() + outputFilename);

    }else{
        qDebug()<<"No ImageSet acquired !";
        exit(1);
    }*/

    flush(1);

}




