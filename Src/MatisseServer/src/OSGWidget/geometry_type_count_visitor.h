#ifndef GEOMETRYTYPECOUNTVISITOR_H
#define GEOMETRYTYPECOUNTVISITOR_H

#include <osg/Geode>
#include <osg/NodeVisitor>

class GeometryTypeCountVisitor : public osg::NodeVisitor
{
public:
    GeometryTypeCountVisitor();
    virtual ~GeometryTypeCountVisitor();

    virtual void apply( osg::Node &node );
    virtual void apply( osg::Geode &geode );

    long getNbPoints() const { return m_nbPoints; }
    long getNbLigne() const { return m_nbLignes; }
    long getNbTriangles() const { return m_nbTriangles; }
    long getNbOthers() const { return m_nbOthers; }


private :
    double m_nbPoints;
    double m_nbLignes;
    double m_nbTriangles;
    double m_nbOthers;
};

#endif // GEOMETRYTYPECOUNTVISITOR_H

