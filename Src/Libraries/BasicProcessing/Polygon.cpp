#include "Polygon.h"
#include "stdvectoperations.h"
#include <cfloat>

using namespace basicproc;
using namespace std;


Polygon::Polygon():_modifSinceUpdate(true)
{
    _gpcPolygon.contour = NULL;
    _gpcPolygon.hole = NULL;
}

Polygon::~Polygon()
{
    int c;
    // delete old polygon
    if(_gpcPolygon.contour){

        for (c=0; c < _gpcPolygon.num_contours; c++) {

            delete[] _gpcPolygon.contour[c].vertex;
            _gpcPolygon.contour[c].vertex = NULL;

        }
        delete[] _gpcPolygon.contour;
        _gpcPolygon.contour = NULL;
    }
    if (_gpcPolygon.hole){
        delete[] _gpcPolygon.hole;
        _gpcPolygon.hole = NULL;
    }
    //gpc_free_polygon(&_gpcPolygon);
}

bool Polygon::addContour(std::vector<double> x_p, std::vector<double> y_p, bool hole_p)
{
    vertexList vertices;

    vertices.x = x_p;
    vertices.y = y_p;

    _contours.push_back(vertices);
    _contoursHole.push_back(hole_p);

    _modifSinceUpdate = true;

    return true;

}

std::vector<vertexList> Polygon::contours() const
{
    return _contours;
}

bool Polygon::isEmpty()
{
    return (_contours.size()==0);
}

void Polygon::updateGpcPolygon()
{
    int c, v;

    if (_modifSinceUpdate){

        // delete old polygon
        if(_gpcPolygon.contour){

            for (c=0; c < _gpcPolygon.num_contours; c++) {

                delete[] _gpcPolygon.contour[c].vertex;
                _gpcPolygon.contour[c].vertex = NULL;

            }
            delete[] _gpcPolygon.contour;
            _gpcPolygon.contour = NULL;
        }
        if (_gpcPolygon.hole){
            delete[] _gpcPolygon.hole;
            _gpcPolygon.hole = NULL;
        }

        if (this->isEmpty()){
            _gpcPolygon.num_contours=0;
            return;
        }


        _gpcPolygon.num_contours = _contours.size();

        _gpcPolygon.hole = new int[_gpcPolygon.num_contours];
        _gpcPolygon.contour = new gpc_vertex_list[_gpcPolygon.num_contours];

        for (c=0; c < _gpcPolygon.num_contours; c++) {

            _gpcPolygon.contour[c].num_vertices = _contours[c].x.size();
            _gpcPolygon.contour[c].vertex = new gpc_vertex[_gpcPolygon.contour[c].num_vertices];


            for (v= 0; v < _gpcPolygon.contour[c].num_vertices; v++) {
                _gpcPolygon.contour[c].vertex[v].x = _contours[c].x[v];
                _gpcPolygon.contour[c].vertex[v].y = _contours[c].y[v];
            }

            _gpcPolygon.hole[c] = _contoursHole[c];

        }
        _modifSinceUpdate = false;
    }else{
        return;
    }
}

void Polygon::updatePolygonFromGpc()
{
    int c, v;
    _contours.clear();
    _contoursHole.clear();

    for (c=0; c < _gpcPolygon.num_contours; c++)
    {
        vertexList vList;

        for (v= 0; v < _gpcPolygon.contour[c].num_vertices; v++)
        {
            vList.x.push_back(_gpcPolygon.contour[c].vertex[v].x);
            vList.y.push_back(_gpcPolygon.contour[c].vertex[v].y);
        }
        _contours.push_back( vList );
        vList.x.clear();
        vList.y.clear();

        _contoursHole.push_back(_gpcPolygon.hole[c]);
    }
}

gpc_polygon* Polygon::gpcPolygon()
{
    return &_gpcPolygon;
}

void Polygon::clip(Polygon &poly2_p, Polygon &result_p, poly_op operation)
{
    gpc_op GPC_ARG;

    if (operation == DIFF){
        GPC_ARG = GPC_DIFF;
    }else if (operation == INT){
        GPC_ARG = GPC_INT;
    }else if (operation == XOR){
        GPC_ARG = GPC_XOR;
    }else if (operation == UNION){
        GPC_ARG = GPC_UNION;
    }

    // Update polygon definition (does nothing if already up to date)
    updateGpcPolygon();
    poly2_p.updateGpcPolygon();

    // Clip polygon
    gpc_polygon_clip(GPC_ARG, gpcPolygon(), poly2_p.gpcPolygon(), result_p.gpcPolygon());

    // Update result description from its gpc polygon
    result_p.updatePolygonFromGpc();

}

void Polygon::getBoundingBox(double &tlx_p, double &tly_p, double &brx_p, double &bry_p)
{

    //Init
    tlx_p=DBL_MAX;
    tly_p=DBL_MAX;
    brx_p=-DBL_MAX;
    bry_p=-DBL_MAX;

    std::vector<double> xArray, yArray;
    std::vector<double>::iterator min_x_it, min_y_it, max_x_it, max_y_it;



    for (unsigned int i=0; i<_contours.size(); i++){
        xArray.clear();
        yArray.clear();

        for (unsigned int j=0; j<_contours[i].x.size(); j++){
            // Fill x & y array
            xArray.push_back(_contours[i].x[j]);
            yArray.push_back(_contours[i].y[j]);

        }
        // Compute min,max
        min_x_it = std::min_element(xArray.begin(), xArray.end());
        min_y_it = std::min_element(yArray.begin(), yArray.end());
        max_x_it = std::max_element(xArray.begin(), xArray.end());
        max_y_it = std::max_element(yArray.begin(), yArray.end());

        if (*min_x_it < tlx_p)
            tlx_p = *min_x_it;

        if (*min_y_it < tly_p)
            tly_p = *min_y_it;

        if (*max_x_it > brx_p)
            brx_p = *max_x_it;

        if (*max_y_it > bry_p)
            bry_p = *max_y_it;

    }

}

void Polygon::getContourCenter(double &cx_p, double &cy_p, int contourIndex_p)
{

    cx_p = doubleVectorMean(_contours[contourIndex_p].x);
    cy_p = doubleVectorMean(_contours[contourIndex_p].y);

}

double Polygon::area()
{
    // Initialze area
    double area = 0.0;

    for (unsigned int k=0; k<_contours.size(); k++){

        // Calculate value of shoelace formula
        int j = _contours[k].x.size() - 1;

        for (unsigned int i = 0; i < _contours[k].x.size(); i++)
        {
            area += (_contours[k].x[j] + _contours[k].x[i]) * (_contours[k].y[j] - _contours[k].y[i]);
            j = i;  // j is previous vertex to i
        }

    }

    // Return absolute value
    return abs(area / 2.0);
}

double Polygon::clipArea(Polygon &poly2_p, poly_op operation)
{
    Polygon result;

    this->clip(poly2_p, result, operation);

    if (result.isEmpty())
        return 0;
    else
        return result.area();
}

bool Polygon::operator ==(const Polygon &polyB_p)
{
    if (_contours.size() == polyB_p.contours().size()){

        for (unsigned int i=0; i<_contours.size(); i++){

            if(_contours[i].x.size() != polyB_p.contours().at(i).x.size())
                return false;
        }

        bool x_areEquals = false;
        bool y_areEquals = false;
        bool xy_areEquals = false;
        bool contours_areEquals = true;

        for (unsigned int i=0; i<_contours.size(); i++){

            for (unsigned int j=0; j<_contours[i].x.size(); j++){

                xy_areEquals = false;

                for (unsigned int k=0; k<_contours[i].x.size(); k++){

                    x_areEquals = (_contours[i].x[j]==polyB_p.contours().at(i).x[k]);
                    y_areEquals = (_contours[i].y[j]==polyB_p.contours().at(i).y[k]);

                    xy_areEquals = xy_areEquals || (x_areEquals && y_areEquals);

                }

                contours_areEquals = contours_areEquals && xy_areEquals;

                if(!contours_areEquals)
                    return false;
            }
        }

    }else{
        return false;
    }

    return true;

}

bool Polygon::operator !=(const Polygon &polyB_p)
{
    return !(this->operator ==(polyB_p));
}

void Polygon::operator =(Polygon &polyB_p)
{


    // Remove previously defined polygon

    int c,v;

    if(_gpcPolygon.contour){

        for (c=0; c < _gpcPolygon.num_contours; c++) {

            delete[] _gpcPolygon.contour[c].vertex;
            _gpcPolygon.contour[c].vertex = NULL;

        }
        delete[] _gpcPolygon.contour;
        _gpcPolygon.contour = NULL;
    }
    if (_gpcPolygon.hole){
        delete[] _gpcPolygon.hole;
        _gpcPolygon.hole = NULL;
    }

    // Complete Gpc polygon from polyB
    _gpcPolygon.num_contours = polyB_p.gpcPolygon()->num_contours;

    _gpcPolygon.hole = new int[_gpcPolygon.num_contours];
    _gpcPolygon.contour = new gpc_vertex_list[_gpcPolygon.num_contours];

    for (c=0; c < _gpcPolygon.num_contours; c++) {

        _gpcPolygon.contour[c].num_vertices = polyB_p.gpcPolygon()->contour[c].num_vertices;
        _gpcPolygon.contour[c].vertex = new gpc_vertex[_gpcPolygon.contour[c].num_vertices];


        for (v= 0; v < _gpcPolygon.contour[c].num_vertices; v++) {
            _gpcPolygon.contour[c].vertex[v].x = polyB_p.gpcPolygon()->contour[c].vertex[v].x;
            _gpcPolygon.contour[c].vertex[v].y = polyB_p.gpcPolygon()->contour[c].vertex[v].y;
        }

        _gpcPolygon.hole[c] = polyB_p.gpcPolygon()->hole[c];
    }

    this->updatePolygonFromGpc();

}


