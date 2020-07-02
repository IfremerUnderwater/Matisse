#ifndef AREA_COMPUTATION_VISITOR_H
#define AREA_COMPUTATION_VISITOR_H

#include <osg/Geode>
#include <osg/NodeVisitor>

class AreaComputationVisitor : public osg::NodeVisitor
{
public :
    AreaComputationVisitor();
    virtual ~AreaComputationVisitor();

    virtual void apply( osg::Node &node );
    virtual void apply( osg::Geode &geode );

    double getArea() const { return m_area; }

private :
    double m_area;
};

#endif // AREA_COMPUTATION_VISITOR_H
