#include "Polygon.h"


using namespace basicproc;

Polygon::Polygon():_modifSinceUpdate(true)
{
}

Polygon::~Polygon()
{
    int c;
    // delete old polygon
    if(_gpcPolygon.contour){

        for (c=0; c < _gpcPolygon.num_contours; c++) {

            delete[] _gpcPolygon.contour[c].vertex;

        }
        delete[] _gpcPolygon.contour;
    }
    if (_gpcPolygon.hole)
        delete[] _gpcPolygon.hole;
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

            }
            delete[] _gpcPolygon.contour;
        }
        if (_gpcPolygon.hole)
            delete[] _gpcPolygon.hole;

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

void Polygon::clip(Polygon &poly2_p, Polygon &result_p, QString operation)
{
    gpc_op GPC_ARG;

    if (operation == QString("DIFF")){
        GPC_ARG = GPC_DIFF;
    }else if (operation == QString("INT")){
        GPC_ARG = GPC_INT;
    }else if (operation == QString("XOR")){
        GPC_ARG = GPC_XOR;
    }else if (operation == QString("UNION")){
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


