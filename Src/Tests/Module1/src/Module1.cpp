#include "Module1.h"
#include "MosaicContext.h"
#include "NavImage.h"
#include "GeoTransform.h"
#include "RasterGeoreferencer.h"

#include "MosaicDescriptor.h"
#include "MosaicDrawer.h"

// Exportation de la classe Module1 dans la bibliotheque de plugin TestModule1
Q_EXPORT_PLUGIN2(Module1, Module1)


Module1::Module1() :
    Processor(NULL, "Module1", "Module d'essai", 1, 2)
{
    addExpectedParameter("algo_param", "sift_code");
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

    qreal x,y,lat,lon;
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


    MosaicDescriptor mosaicD;
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
                K.at<qreal>(i,j) = qK(i,j);
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
            V_T_C.at<qreal>(i,0) = V_Pose_C(0,i);
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
                cams.push_back(new ProjectiveCamera(navImage , K, V_T_C, V_R_C, (qreal)scaleFactor));
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
        //cv::imshow(std::string("MosaicTest"),mosaicImage);
        //cv::waitKey();

        //Write Geofile *****************************************************
        QString utmProjParam, utmHemisphereOption,utmZoneString;

        // Construct utm proj param options
        utmZoneString = QString("%1 ").arg(mosaicD.utmZone())+ mosaicD.utmHemisphere();
        QStringList utmParams = utmZoneString.split(" ");

        if ( utmParams.at(1) == "S" ){
            utmHemisphereOption = QString(" +south");
        }else{
            utmHemisphereOption = QString("");
        }
        utmProjParam = QString("+proj=utm +zone=") + utmParams.at(0);

        QString gdalOptions =  QString("-a_srs \"")+ utmProjParam + QString("\" -of GTiff -co \"INTERLEAVE=PIXEL\" -a_ullr %1 %2 %3 %4")
                .arg(mosaicD.mosaic_ullr().at<qreal>(0,0),0,'f',2)
                .arg(mosaicD.mosaic_ullr().at<qreal>(1,0),0,'f',2)
                .arg(mosaicD.mosaic_ullr().at<qreal>(2,0),0,'f',2)
                .arg(mosaicD.mosaic_ullr().at<qreal>(3,0),0,'f',2);
        RasterGeoreferencer rasterGeoref;
        rasterGeoref.WriteGeoFile(mosaicImage,mosaicMask,QString("./output.tiff"),gdalOptions);

    }else{
        qDebug()<<"No ImageSet acquired !";
        exit(1);
    }

    flush(1);

}




