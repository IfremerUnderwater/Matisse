#ifndef POLYGON_H
#define POLYGON_H


#include "gpc.h"

#include <vector>
#include <QString>
#include <QMetaType>

namespace basicproc {


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
    /// \param poly2_p polygon with the one clipping operation is done
    /// \param result_p clipping result
    /// \param operation : values "DIFF", "INT", "XOR" and "UNION" for corresponding boolean operation Difference, Intersection, Exclusive or, Union
    ///
    void clip(Polygon &poly2_p, Polygon & result_p, poly_op operation);

    ///
    /// \brief getBoundingBox return the polygon bounding box
    /// \param tlx_p Top left x coordinate
    /// \param tly_p Top left y coordinate
    /// \param brx_p Bottom right x coordinate
    /// \param bry_p Bottom right y coordinate
    ///
    void getBoundingBox(double &tlx_p, double &tly_p, double &brx_p, double &bry_p);

    ///
    /// \brief getContourCenter get the polygon center
    /// \param cx_p x coord
    /// \param cy_p y coord
    /// \param contourIndex contour for which you want the center
    ///
    void getContourCenter(double &cx_p, double &cy_p, int contourIndex_p=0);

    ///
    /// \brief area this function compute the signed polygon area
    /// \return area
    ///
    double area();

    ///
    /// \brief clipArea compute area of the clipped polygons
    /// \param poly2_p polygon with the one clipping operation is done
    /// \param operation : values "DIFF", "INT", "XOR" and "UNION" for corresponding boolean operation Difference, Intersection, Exclusive or, Union
    /// \return area of clipped polygons
    ///
    double clipArea(Polygon &poly2_p, poly_op operation);

    bool operator ==(const Polygon &polyB_p);
    bool operator !=(const Polygon &polyB_p);

    void operator =(Polygon &polyB_p);

private:
    gpc_polygon _gpcPolygon;

    std::vector<vertexList> _contours;
    std::vector<bool> _contoursHole;
    bool _modifSinceUpdate;

};

}

Q_DECLARE_METATYPE(basicproc::Polygon)

#endif // POLYGON_H
