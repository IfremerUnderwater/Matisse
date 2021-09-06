#ifndef BASIC_PROCESSING_POLYGON_H_
#define BASIC_PROCESSING_POLYGON_H_


#include "gpc.h"

#include <vector>
#include <QString>
#include <QMetaType>

namespace basic_processing {


typedef struct                      /* Vertex list structure             */
{
    std::vector<double> x;       /* Vertex array pointer              */
    std::vector<double> y;
} vertexList;


typedef enum                        /* Set operation type                */
{
    DIFF,                         /* Difference                        */
    INT,                          /* Intersection                      */
    XOR,                          /* Exclusive or                      */
    UNION                         /* Union                             */
} poly_op;


///
/// \brief The Polygon class implements polygon description and boolean operations
///
class Polygon
{

public:
    Polygon(); // create an empty polygon
    ~Polygon();

    ///
    /// \brief addContour add a contour to the current polygon
    /// \param x_p : x coordinate list of vertices
    /// \param y_p : y coordinate list of vertices
    /// \param hole_p
    ///
    bool addContour(std::vector<double> x_p, std::vector<double> y_p, bool hole_p=false);

    std::vector<vertexList> contours() const;

    ///
    /// \brief isEmpty test wether the polygon has vertices or not
    /// \return true if polygon is empty (no vertices)
    ///
    bool isEmpty();

    ///
    /// \brief updateGpcPolygon update _gpcPolygon from this class polygon description
    ///
    void updateGpcPolygon();

    ///
    /// \brief updatePolygonFromGpc complete local definition from gpc polygon definition
    ///
    void updatePolygonFromGpc();

    gpc_polygon *gpcPolygon();

    ///
    /// \brief clip does a boolean operation between the current polygon and poly2_p
    /// \param _poly2_p polygon with the one clipping operation is done
    /// \param _result_p clipping result
    /// \param _operation : values "DIFF", "INT", "XOR" and "UNION" for corresponding boolean operation Difference, Intersection, Exclusive or, Union
    ///
    void clip(Polygon &_poly2_p, Polygon & _result_p, poly_op _operation);

    ///
    /// \brief getBoundingBox return the polygon bounding box
    /// \param _tlx_p Top left x coordinate
    /// \param _tly_p Top left y coordinate
    /// \param _brx_p Bottom right x coordinate
    /// \param _bry_p Bottom right y coordinate
    ///
    void getBoundingBox(double &_tlx_p, double &_tly_p, double &_brx_p, double &_bry_p);

    ///
    /// \brief getContourCenter get the polygon center
    /// \param _cx_p x coord
    /// \param _cy_p y coord
    /// \param _contour_index contour for which you want the center
    ///
    void getContourCenter(double &_cx_p, double &_cy_p, int _contour_index_p=0);

    ///
    /// \brief area this function compute the signed polygon area
    /// \return area
    ///
    double area();

    ///
    /// \brief clipArea compute area of the clipped polygons
    /// \param _poly2_p polygon with the one clipping operation is done
    /// \param _operation : values "DIFF", "INT", "XOR" and "UNION" for corresponding boolean operation Difference, Intersection, Exclusive or, Union
    /// \return area of clipped polygons
    ///
    double clipArea(Polygon &_poly2_p, poly_op _operation);

    bool operator ==(const Polygon &_polyB_p);
    bool operator !=(const Polygon &_polyB_p);

    void operator =(Polygon &_polyB_p);

private:
    gpc_polygon m_gpc_polygon;

    std::vector<vertexList> m_contours;
    std::vector<bool> m_contours_hole;
    bool m_modif_since_update;

};

} // namespace basic_processing

Q_DECLARE_METATYPE(basic_processing::Polygon)

#endif // BASIC_PROCESSING_POLYGON_H_
