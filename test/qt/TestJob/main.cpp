#include <QCoreApplication>

#include "JobServer.h"

using namespace MatisseTools;


int main(int argc, char *argv[])
{


    QCoreApplication app(argc, argv);
    Xml *xml =new Xml("../../../main/qt/xml","../../../main/qt/xml");
    xml->readJobFile("job1.xml");
    xml->readJobFile("job2.xml");
    JobServer server(8888,xml);
    return app.exec();

}
