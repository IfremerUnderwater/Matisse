#include "kml_handler.h"


using kmldom::BasicLinkPtr;
using kmldom::ContainerPtr;
using kmldom::DocumentPtr;
using kmldom::ElementPtr;
using kmldom::FeaturePtr;
using kmldom::GeometryPtr;
using kmldom::IconStylePtr;
using kmldom::ItemIconPtr;
using kmldom::KmlPtr;
using kmldom::ListStylePtr;
using kmldom::ModelPtr;
using kmldom::LocationPtr;
using kmldom::MultiGeometryPtr;
using kmldom::NetworkLinkPtr;
using kmldom::OverlayPtr;
using kmldom::PairPtr;
using kmldom::PlacemarkPtr;
using kmldom::StylePtr;
using kmldom::StyleMapPtr;
using kmldom::StyleSelectorPtr;

KMLHandler::KMLHandler()
{

}

FeaturePtr KMLHandler::getRootFeature(const ElementPtr& _root) {
    const KmlPtr kml = kmldom::AsKml(_root);
    if (kml && kml->has_feature()) {
        return kml->get_feature();
    }
    return kmldom::AsFeature(_root);
}

bool KMLHandler::visitGeometry(const GeometryPtr& _geometry) {
    if (m_model = kmldom::AsModel(_geometry)) {

        if (m_model->has_location()){
            m_model_location = m_model->get_location();
            return false;
        }
    }
    return false;
}

bool KMLHandler::visitPlacemark(const PlacemarkPtr& _placemark) {
    if (_placemark->has_geometry()) {
        if(visitGeometry(_placemark->get_geometry()))
            return true;
    }
    return false;
}


bool KMLHandler::visitFeature(const FeaturePtr& _feature) {
    if (const PlacemarkPtr placemark = kmldom::AsPlacemark(_feature)) {
        if (visitPlacemark(placemark))
            return true;
    }
    return false;
}

bool KMLHandler::readFile(std::string _kmlfile) {

    std::string kml;
    bool status = kmlbase::File::ReadFileToString(_kmlfile, &kml);
    if (!status) {
        return false;
    }
    std::string errors;
    ElementPtr root = kmldom::Parse(kml, &errors);
    if (!root) {
        return false;
    }
    if (const FeaturePtr feature = getRootFeature(root)) {
        if(visitFeature(feature))
            return true;
    }
    return false;
}

std::string KMLHandler::getModelPath()
{
    std::string filepath, base_directory, filename;

    kmlbase::File::SplitFilePath(filepath,
                                &base_directory,
                                &filename);

    //return base_directory+std::string("/")+m_model->get_link()->get_href();
    return m_model->get_link()->get_href();

}

double KMLHandler::getModelLat()
{
    if(m_model_location->has_latitude())
        return m_model_location->get_latitude();
    else
        return 0.;
}

double KMLHandler::getModelLon()
{
    if(m_model_location->has_longitude())
        return m_model_location->get_longitude();
    else
        return 0.;
}

double KMLHandler::getModelAlt()
{
    if(m_model_location->has_altitude())
        return m_model_location->get_altitude();
    else
        return 0.;
}

