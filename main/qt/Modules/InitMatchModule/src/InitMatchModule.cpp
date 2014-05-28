#include "InitMatchModule.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include "MatisseCppLib.h"


// Export de la classe InitMatchModule dans la bibliotheque de plugin InitMatchModule
Q_EXPORT_PLUGIN2(InitMatchModule, InitMatchModule)



InitMatchModule::InitMatchModule() :
    Processor(NULL, "InitMatchModule", "Init and Matching module", 1, 1)
{
    qDebug() << logPrefix() << "Initialiazing Matlab libraries and runtime";

    // Initialize the MATLAB Compiler Runtime global state
    if (!mclInitializeApplication(NULL,0))
    {
        std::cerr << "Could not initialize the application properly."
                  << std::endl;
        return;
    }
    // Initialize the Vigenere library
    if( !MatisseCppLibInitialize() )
    {
        std::cerr << "Could not initialize the library properly."
                  << std::endl;
        return;
    }

    // Must declare all MATLAB data types after initializing the
    // application and the library, or their constructors will fail.
    //mwArray mosaicData;
    addExpectedParameter("dataset_param", "utm_hemisphere");
    addExpectedParameter("dataset_param", "utm_zone");
}

InitMatchModule::~InitMatchModule(){

    MatisseCppLibTerminate();
    mclTerminateApplication();

}

bool InitMatchModule::configure()
{
    return true;
}

void InitMatchModule::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;
    qDebug() << logPrefix() << "Process InitMatchModule";

    // Forward image
    postImage(1, image);

}

bool InitMatchModule::start()
{
    return true;
}

bool InitMatchModule::stop()
{
    return true;
}

void InitMatchModule::onFlush(quint32 port)
{
    qDebug() << logPrefix() << "flush port " << port;

    mwArray mosaicData;
    mwArray algo_param;
    mwArray matchingData;
    QString tempString;
    int tempInt;
    bool Ok;

    // Generate algo_param from GUI parameters
    MatlabCppInterface::generate_algo_param(_matisseParameters, algo_param);

    tempString = _matisseParameters->getStringParamValue("dataset_param", "utm_hemisphere");
    algo_param.Get("utm_hemisphere",1,1).Set(mwArray(tempString.toLocal8Bit().data()));

    tempInt = _matisseParameters->getIntParamValue("dataset_param", "utm_zone",Ok);
    algo_param.Get("utm_zone",1,1).Set(mwArray(tempInt));

    // Call main function of the module (Init, Sift and Matching)
    mosaic_init_and_matching(3,mosaicData,algo_param,matchingData,mosaicData,algo_param);

    // Init C++ variables
    MosaicData * pmosaicDataCpp = new MosaicData();
    MatchingData * pmatchingDataCpp = new MatchingData();

    // Convert from Matlab to Cpp types
    MatlabCppInterface::mosaicDataMatlabToCpp (mosaicData, *pmosaicDataCpp);
    MatlabCppInterface::matchingDataMatlabToCpp (matchingData, *pmatchingDataCpp);

    // Get utm params from algo_param with the next two lines of code.
    // [ Please note that in matlab utm_parameter are stored in algo_param
    // while in C++ version the utm params are stored in mosaicDataCpp.init (better choice) ]
    std::string utm_hemisphere = algo_param.Get("utm_hemisphere",1,1).ToString();
    pmosaicDataCpp->init.utmHemisphere = QString(utm_hemisphere.c_str()).split('\n').at(0); // remove \n (bug in matlab api ?)
    pmosaicDataCpp->init.utmZone = algo_param.Get("utm_zone",1,1);

    qDebug() << "Number of matched pairs" << pmatchingDataCpp->count();

    // Add mosaicData to mosaic _context
    QVariant * vmosaicDataCpp = new QVariant();
    vmosaicDataCpp->setValue(pmosaicDataCpp);
    _context->addObject("MosaicData", vmosaicDataCpp);

    // Add matchingData to mosaic _context
    QVariant * vmatchingDataCpp = new QVariant();
    vmatchingDataCpp->setValue(pmatchingDataCpp);
    _context->addObject("MatchingData", vmatchingDataCpp);

    // Flush next module port
    flush(0);

}

