#ifndef POLYGON_H
#define POLYGON_H

#include "gpc.h"
#include <vector>
#include <QString>

namespace basicproc {


typedef struct                      /* Vertex list structure             */
{
  std::vector<double> x;       /* Vertex array pointer              */
  std::vector<double> y;
} vertexList;


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
    bool addContour(std::vector<double> x_p, std::vector<double> y_p, bool hole_p);

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
    void clip(Polygon &poly2_p, Polygon & result_p, QString operation);

private:
    gpc_polygon _gpcPolygon;

    std::vector<vertexList> _contours;
    std::vector<bool> _contoursHole;
    bool _modifSinceUpdate;

};

}

#endif // POLYGON_H
