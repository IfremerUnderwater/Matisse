
#include <QList>
#include <QtDebug>

#include "Server.h"
#include "FileImage.h"
#include "ImageSet.h"
#include "Dim2FileReader.h"
#include "PictureFileSet.h"
#include "Context.h"
#include "MatisseParameters.h"
#include "AssemblyGui.h"
#include <qgsapplication.h>

using namespace MatisseServer;
using namespace MatisseTools;

int main(int argc, char *argv[])
{

    // Init QGIS
    QProcessEnvironment env;
    QString oswgeo4w = env.systemEnvironment().value("OSGEO4W_ROOT");
    QgsApplication::setPrefixPath(oswgeo4w+"\\apps\\qgis", true);
#ifdef QT_DEBUG
    qDebug() << "Load Debug versions of plugins";
    QgsApplication::setPluginPath(oswgeo4w+"\\apps\\qgis\\pluginsd");
#endif

    QgsApplication::initQgis();
    QgsApplication a(argc, argv, true);

    // Pour messages en français...
    QString locale = QLocale::system().name();
    QTranslator translator;
    QString libLocation = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    translator.load("qt_fr", ".", QString(), ".ts");
    a.installTranslator(&translator);


    qDebug() << QgsApplication::showSettings();
    QString testLaunch("testLaunch");
    if (argc==2 && testLaunch== argv[1])
    {
        qDebug() << testLaunch;
        Server server;
        server.setSettingsFile();
        server.init();

        server.xmlTool().readAssemblyFile("Assemblage_1.xml");
        JobDefinition * jobDef = server.xmlTool().getJob("job2");
        //AssemblyDefinition *assemblyDef = server.xmlTool().getAssembly("Assemblage 1");
        //JobDefinition *jobDef = server.xmlTool().getJob("Job 1");

        //server.processJob(*jobDef);

//        SourceDefinition *sourceDef= new SourceDefinition("DTPictureFileSetImageProvider");
//        assemblyDef.setSourceDefinition(sourceDef);

//        ProcessorDefinition *processorDef=new ProcessorDefinition("Module1", 1);
//        assemblyDef.addProcessorDef(processorDef);
//        ProcessorDefinition *processorDef2=new ProcessorDefinition("Module2", 2);
//        assemblyDef.addProcessorDef(processorDef2);

//        ConnectionDefinition *connectionDef=new ConnectionDefinition(0, 0, 1, 0);
//        assemblyDef.addConnectionDef(connectionDef);
//        ConnectionDefinition *connectionDef2=new ConnectionDefinition(1, 1, 2, 1);
//        assemblyDef.addConnectionDef(connectionDef2);

        server.processJob(*jobDef);

        // Attente 1 seconde pour les flusher les logs
        QTime dieTime= QTime::currentTime().addSecs(1);
           while( QTime::currentTime() < dieTime ) {
               QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
           }

           delete jobDef;

        return 0;
    }

    QString settingsFile = "";
    if (argc > 1) {
        settingsFile = argv[1];
    }
    AssemblyGui w(settingsFile);

    if (!w.isShowable()) {
        return -1;
    }

    w.show();

    return a.exec();
    //   MatisseServer w;
    
//    return a.exec();

//    PictureFileSet pictureFileSet("C:/WorkspaceMatisse/Test_dataset", "navFile_mv.txt");

//    FileImage fileImage;
//    fileImage.setPictureFileSet(pictureFileSet);
//    FileImage fileImage2;
//    fileImage2.setPictureFileSet(pictureFileSet);
//    fileImage2.setId(342);
//    fileImage2.setNavInfo(NavInfo(1,2,3,4,5,6,7,8,9));
//    qDebug() << "Image1:";
//    qDebug() << fileImage.dumpAttr();
//    qDebug() << "Image2:";
//    qDebug() << fileImage2.dumpAttr();

//    qDebug() << "ImageSet...";
//    ImageSet set;
//    set.addImage(&fileImage);
//    set.addImage(&fileImage2);

////    qDebug() << "set dump...";
//    qDebug() << set.dumpAttr();

//    Dim2FileReader fileReader;
//    qDebug() << "File ok" << fileReader.readDim2File("C:/WorkspaceMatisse/Test_dataset/navFile_mv.txt");
//    qDebug() << "size=" << fileReader.getNumberOfImages();

//    DTPictureFileSetImageProvider provider;
//    provider.setPictureFileSet(pictureFileSet);
//    ImageSet *  imageSet = provider.getImageSet();
//    if (imageSet != 0) {
//        qDebug() << "ImageSet size = " << imageSet->getNumberOfImages();
//        // chargement image n�2...
//        FileImage *imageIndex2 = static_cast<FileImage *>(imageSet->getImage(2));
//        qDebug() << "Size=" << imageIndex2->getImageData()->rows << ", " <<  imageIndex2->getImageData()->cols;
//        qDebug() << "NAME=" << imageIndex2->getFileName();
//    } else {
//        qDebug() << "ImageSet size 0";
//    }

//    ProcessorTest processorTest;
//    ProcessorTest processorTest2;
//    Context context;
//    MosaicData mosaicParameters;
//    QList<imageSet *> imagesSetIn;
//    QList<imageSet *> outImages = processorTest.assemble(context, mosaicParameters, imagesSetIn);
//   // QList<imageSet *> imagesSetIn;
//    QList<imageSet *> outImages2 = processorTest2.assemble(context, mosaicParameters, outImages);

}
