#include "DrawAndWriteModule.h"
#include "MosaicContext.h"
#include "NavImage.h"

#include "MatisseCppLib.h"


// Export de la classe DrawAndWriteModule dans la bibliotheque de plugin DrawAndWriteModule
Q_EXPORT_PLUGIN2(DrawAndWriteModule, DrawAndWriteModule)



DrawAndWriteModule::DrawAndWriteModule() :
    Processor(NULL, "DrawAndWriteModule", "Optimization module", 1, 1)
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

DrawAndWriteModule::~DrawAndWriteModule(){

    MatisseCppLibTerminate();
    mclTerminateApplication();

}

bool DrawAndWriteModule::configure()
{
    return true;
}

void DrawAndWriteModule::onNewImage(quint32 port, Image &image)
{
    qDebug() << logPrefix() << "Receive image on port " << port;
    qDebug() << logPrefix() << "Process DrawAndWriteModule";

}

bool DrawAndWriteModule::start()
{
    return true;
}

bool DrawAndWriteModule::stop()
{
    return true;
}

void DrawAndWriteModule::onFlush(quint32 port)
{

    // Get MosaicData from context
    QVariant *vmosaicData = _context->getObject("MosaicData");
    MosaicData * pmosaicData;
    if (vmosaicData) {
        pmosaicData = vmosaicData->value<MosaicData*>();
        qDebug()<< logPrefix() << "Receiving MosaicData on port : " << port;
    }


    mwArray mosaicData;
    mwArray algo_param;
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

    // Main function of the module (Init, Sift and Matching)
    mosaic_draw_and_write(mosaicData,algo_param);

}



