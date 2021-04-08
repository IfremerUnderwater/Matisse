#ifndef KML_HANDLER_H
#define KML_HANDLER_H

#include <iostream>
#include <string>
#include "kml/dom.h"
#include "kml/base/file.h"

class KMLHandler
{
public:
    KMLHandler();
    bool readFile(std::string _kmlfile);
    std::string getModelPath();
    double getModelLat();
    double getModelLon();
    double getModelAlt();
private:
    kmldom::FeaturePtr getRootFeature(const kmldom::ElementPtr &_root);
    bool visitGeometry(const kmldom::GeometryPtr &_geometry);
    bool visitPlacemark(const kmldom::PlacemarkPtr &_placemark);
    bool visitFeature(const kmldom::FeaturePtr &_feature);

    kmldom::ModelPtr m_model;
    kmldom::LocationPtr m_model_location;
};

#endif // KML_HANDLER_H
