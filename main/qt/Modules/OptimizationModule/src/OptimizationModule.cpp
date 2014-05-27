#include "OptimizationModule.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include "MatisseCppLib.h"


// Export de la classe OptimizationModule dans la bibliotheque de plugin OptimizationModule
Q_EXPORT_PLUGIN2(OptimizationModule, OptimizationModule)



OptimizationModule::OptimizationModule() :
    Processor(NULL, "OptimizationModule", "Optimization module", 1, 1)
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
}

OptimizationModule::~OptimizationModule(){

    MatisseCppLibTerminate();
    mclTerminateApplication();

}

bool OptimizationModule::configure()
{
    return true;
}

void OptimizationModule::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;
    qDebug() << logPrefix() << "Process OptimizationModule";

}

bool OptimizationModule::start()
{
    return true;
}

bool OptimizationModule::stop()
{
    return true;
}

void OptimizationModule::onFlush(quint32 port)
{

    // Get MosaicData from context
    QVariant *vmosaicData = _context->getObject("MosaicData");
    MosaicData * pmosaicData;
    if (vmosaicData) {
        pmosaicData = vmosaicData->value<MosaicData*>();
        qDebug()<< logPrefix() << "Receiving MosaicData on port : " << port;
    }

    // Get MatchingData from context
    QVariant *vmatchingData = _context->getObject("MatchingData");
    MatchingData * pmatchingData;
    if (vmatchingData) {
        pmatchingData = vmatchingData->value<MatchingData*>();
        qDebug()<< logPrefix() << "Receiving MatchingData on port : " << port;
    }


    mwArray mosaicData;
    mwArray algo_param;
    mwArray matchingData;
    QString tempString;
    int tempInt;

    // Generate algo_param
    MatlabCppInterface::generate_algo_param(_matisseParameters, algo_param);

    // Fill utm parameters from mosaicData
    tempString = pmosaicData->init.utmHemisphere;
    algo_param.Get("utm_hemisphere",1,1).Set(mwArray(tempString.toLocal8Bit().data()));
    tempInt = pmosaicData->init.utmZone;
    algo_param.Get("utm_zone",1,1).Set(mwArray(tempInt));

    // Convert C++ type to matlab
    MatlabCppInterface::mosaicDataCppToMatlab (*pmosaicData, mosaicData);
    if (pmatchingData!=NULL){
        MatlabCppInterface::matchingDataCppToMatlab (*pmatchingData, matchingData);
    }else{
        matchingData = mwArray(mxCELL_CLASS);
        qDebug() << "matchingData is NULL !!!";
    }


    // Main function of the module (Init, Sift and Matching)
    mosaic_optimization(2,mosaicData,algo_param,mosaicData,algo_param,matchingData);

    // Convert from Matlab to Cpp types
    MatlabCppInterface::mosaicDataMatlabToCpp (mosaicData, *pmosaicData);

    // Flush next module port
    flush(0);

}



