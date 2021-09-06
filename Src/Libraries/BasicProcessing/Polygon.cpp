#include "Polygon.h"
#include "stdvectoperations.h"
#include <cfloat>

using namespace std;

namespace basic_processing {

Polygon::Polygon():
    m_modif_since_update(true)
{
    m_gpc_polygon.contour = NULL;
    m_gpc_polygon.hole = NULL;
}

Polygon::~Polygon()
{
    int c;
    // delete old polygon
    if(m_gpc_polygon.contour){

        for (c=0; c < m_gpc_polygon.num_contours; c++) {

            delete[] m_gpc_polygon.contour[c].vertex;
            m_gpc_polygon.contour[c].vertex = NULL;

        }
        delete[] m_gpc_polygon.contour;
        m_gpc_polygon.contour = NULL;
    }
    if (m_gpc_polygon.hole){
        delete[] m_gpc_polygon.hole;
        m_gpc_polygon.hole = NULL;
    }
    //gpc_free_polygon(&_gpcPolygon);
}

bool Polygon::addContour(std::vector<double> _x_p, std::vector<double> _y_p, bool _hole_p)
{
    vertexList vertices;

    vertices.x = _x_p;
    vertices.y = _y_p;

    m_contours.push_back(vertices);
    m_contours_hole.push_back(_hole_p);

    m_modif_since_update = true;

    updateGpcPolygon();

    return true;

}

std::vector<vertexList> Polygon::contours() const
{
    return m_contours;
}

bool Polygon::isEmpty()
{
    return (m_contours.size()==0);
}

void Polygon::updateGpcPolygon()
{
    int c, v;

    if (m_modif_since_update){

        // delete old polygon
        if(m_gpc_polygon.contour){

            for (c=0; c < m_gpc_polygon.num_contours; c++) {

                delete[] m_gpc_polygon.contour[c].vertex;
                m_gpc_polygon.contour[c].vertex = NULL;

            }
            delete[] m_gpc_polygon.contour;
            m_gpc_polygon.contour = NULL;
        }
        if (m_gpc_polygon.hole){
            delete[] m_gpc_polygon.hole;
            m_gpc_polygon.hole = NULL;
        }

        if (this->isEmpty()){
            m_gpc_polygon.num_contours=0;
            return;
        }


        m_gpc_polygon.num_contours = m_contours.size();

        m_gpc_polygon.hole = new int[m_gpc_polygon.num_contours];
        m_gpc_polygon.contour = new gpc_vertex_list[m_gpc_polygon.num_contours];

        for (c=0; c < m_gpc_polygon.num_contours; c++) {

            m_gpc_polygon.contour[c].num_vertices = m_contours[c].x.size();
            m_gpc_polygon.contour[c].vertex = new gpc_vertex[m_gpc_polygon.contour[c].num_vertices];


            for (v= 0; v < m_gpc_polygon.contour[c].num_vertices; v++) {
                m_gpc_polygon.contour[c].vertex[v].x = m_contours[c].x[v];
                m_gpc_polygon.contour[c].vertex[v].y = m_contours[c].y[v];
            }

            m_gpc_polygon.hole[c] = m_contours_hole[c];

        }
        m_modif_since_update = false;
    }else{
        return;
    }
}

void Polygon::updatePolygonFromGpc()
{
    int c, v;
    m_contours.clear();
    m_contours_hole.clear();

    for (c=0; c < m_gpc_polygon.num_contours; c++)
    {
        vertexList vList;

        for (v= 0; v < m_gpc_polygon.contour[c].num_vertices; v++)
        {
            vList.x.push_back(m_gpc_polygon.contour[c].vertex[v].x);
            vList.y.push_back(m_gpc_polygon.contour[c].vertex[v].y);
        }
        m_contours.push_back( vList );
        vList.x.clear();
        vList.y.clear();

        m_contours_hole.push_back(m_gpc_polygon.hole[c]);
    }
}

gpc_polygon* Polygon::gpcPolygon()
{
    return &m_gpc_polygon;
}

void Polygon::clip(Polygon &_poly2_p, Polygon &_result_p, poly_op _operation)
{
    gpc_op gpc_arg;

    if (_operation == DIFF){
        gpc_arg = GPC_DIFF;
    }else if (_operation == INT){
        gpc_arg = GPC_INT;
    }else if (_operation == XOR){
        gpc_arg = GPC_XOR;
    }else if (_operation == UNION){
        gpc_arg = GPC_UNION;
    }

    // Update polygon definition (does nothing if already up to date)
    updateGpcPolygon();
    _poly2_p.updateGpcPolygon();

    // Clip polygon
    gpc_polygon_clip(gpc_arg, gpcPolygon(), _poly2_p.gpcPolygon(), _result_p.gpcPolygon());

    // Update result description from its gpc polygon
    _result_p.updatePolygonFromGpc();

}

void Polygon::getBoundingBox(double &_tlx_p, double &_tly_p, double &_brx_p, double &_bry_p)
{

    //Init
    _tlx_p=DBL_MAX;
    _tly_p=DBL_MAX;
    _brx_p=-DBL_MAX;
    _bry_p=-DBL_MAX;

    std::vector<double> x_array, y_array;
    std::vector<double>::iterator min_x_it, min_y_it, max_x_it, max_y_it;



    for (unsigned int i=0; i<m_contours.size(); i++){
        x_array.clear();
        y_array.clear();

        for (unsigned int j=0; j<m_contours[i].x.size(); j++){
            // Fill x & y array
            x_array.push_back(m_contours[i].x[j]);
            y_array.push_back(m_contours[i].y[j]);

        }
        // Compute min,max
        min_x_it = std::min_element(x_array.begin(), x_array.end());
        min_y_it = std::min_element(y_array.begin(), y_array.end());
        max_x_it = std::max_element(x_array.begin(), x_array.end());
        max_y_it = std::max_element(y_array.begin(), y_array.end());

        if (*min_x_it < _tlx_p)
            _tlx_p = *min_x_it;

        if (*min_y_it < _tly_p)
            _tly_p = *min_y_it;

        if (*max_x_it > _brx_p)
            _brx_p = *max_x_it;

        if (*max_y_it > _bry_p)
            _bry_p = *max_y_it;

    }

}

void Polygon::getContourCenter(double &_cx_p, double &_cy_p, int _contour_index_p)
{

    _cx_p = doubleVectorMean(m_contours[_contour_index_p].x);
    _cy_p = doubleVectorMean(m_contours[_contour_index_p].y);

}

double Polygon::area()
{
    // Initialze area
    double area = 0.0;

    for (unsigned int k=0; k<m_contours.size(); k++){

        // Calculate value of shoelace formula
        int j = m_contours[k].x.size() - 1;

        for (unsigned int i = 0; i < m_contours[k].x.size(); i++)
        {
            area += (m_contours[k].x[j] + m_contours[k].x[i]) * (m_contours[k].y[j] - m_contours[k].y[i]);
            j = i;  // j is previous vertex to i
        }

    }

    // Return absolute value
    return abs(area / 2.0);
}

double Polygon::clipArea(Polygon &_poly2_p, poly_op _operation)
{
    Polygon result;

    this->clip(_poly2_p, result, _operation);

    if (result.isEmpty())
        return 0;
    else
        return result.area();
}

bool Polygon::operator ==(const Polygon &_polyB_p)
{
    if (m_contours.size() == _polyB_p.contours().size()){

        for (unsigned int i=0; i<m_contours.size(); i++){

            if(m_contours[i].x.size() != _polyB_p.contours().at(i).x.size())
                return false;
        }

        bool x_are_equals = false;
        bool y_are_equals = false;
        bool xy_are_equals = false;
        bool contours_are_equals = true;

        for (unsigned int i=0; i<m_contours.size(); i++){

            for (unsigned int j=0; j<m_contours[i].x.size(); j++){

                xy_are_equals = false;

                for (unsigned int k=0; k<m_contours[i].x.size(); k++){

                    x_are_equals = (m_contours[i].x[j]==_polyB_p.contours().at(i).x[k]);
                    y_are_equals = (m_contours[i].y[j]==_polyB_p.contours().at(i).y[k]);

                    xy_are_equals = xy_are_equals || (x_are_equals && y_are_equals);

                }

                contours_are_equals = contours_are_equals && xy_are_equals;

                if(!contours_are_equals)
                    return false;
            }
        }

    }else{
        return false;
    }

    return true;

}

bool Polygon::operator !=(const Polygon &_polyB_p)
{
    return !(this->operator ==(_polyB_p));
}

void Polygon::operator =(Polygon &_polyB_p)
{


    // Remove previously defined polygon

    int c,v;

    if(m_gpc_polygon.contour){

        for (c=0; c < m_gpc_polygon.num_contours; c++) {

            delete[] m_gpc_polygon.contour[c].vertex;
            m_gpc_polygon.contour[c].vertex = NULL;

        }
        delete[] m_gpc_polygon.contour;
        m_gpc_polygon.contour = NULL;
    }
    if (m_gpc_polygon.hole){
        delete[] m_gpc_polygon.hole;
        m_gpc_polygon.hole = NULL;
    }

    // Complete Gpc polygon from polyB
    m_gpc_polygon.num_contours = _polyB_p.gpcPolygon()->num_contours;

    m_gpc_polygon.hole = new int[m_gpc_polygon.num_contours];
    m_gpc_polygon.contour = new gpc_vertex_list[m_gpc_polygon.num_contours];

    for (c=0; c < m_gpc_polygon.num_contours; c++) {

        m_gpc_polygon.contour[c].num_vertices = _polyB_p.gpcPolygon()->contour[c].num_vertices;
        m_gpc_polygon.contour[c].vertex = new gpc_vertex[m_gpc_polygon.contour[c].num_vertices];


        for (v= 0; v < m_gpc_polygon.contour[c].num_vertices; v++) {
            m_gpc_polygon.contour[c].vertex[v].x = _polyB_p.gpcPolygon()->contour[c].vertex[v].x;
            m_gpc_polygon.contour[c].vertex[v].y = _polyB_p.gpcPolygon()->contour[c].vertex[v].y;
        }

        m_gpc_polygon.hole[c] = _polyB_p.gpcPolygon()->hole[c];
    }

    this->updatePolygonFromGpc();

}

} // namespace basic_processing


