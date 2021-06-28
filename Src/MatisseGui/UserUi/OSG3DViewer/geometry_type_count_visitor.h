#ifndef GEOMETRYTYPECOUNTVISITOR_H
#define GEOMETRYTYPECOUNTVISITOR_H

#include <osg/Geode>
#include <osg/NodeVisitor>

class GeometryTypeCountVisitor : public osg::NodeVisitor
{
public:
    GeometryTypeCountVisitor();
    virtual ~GeometryTypeCountVisitor();

    virtual void apply( osg::Node &_node );
    virtual void apply( osg::Geode &_geode );

    long getNbPoints() const { return m_nb_points; }
    long getNbLigne() const { return m_nb_lignes; }
    long getNbTriangles() const { return m_nb_triangles; }
    long getNbOthers() const { return m_nb_others; }


private :
    double m_nb_points;
    double m_nb_lignes;
    double m_nb_triangles;
    double m_nb_others;
};

#endif // GEOMETRYTYPECOUNTVISITOR_H

