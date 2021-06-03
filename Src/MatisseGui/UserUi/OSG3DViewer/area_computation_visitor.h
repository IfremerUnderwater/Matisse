#ifndef MATISSE_AREA_COMPUTATION_VISITOR_H_
#define MATISSE_AREA_COMPUTATION_VISITOR_H_

#include <osg/Geode>
#include <osg/NodeVisitor>

namespace matisse {

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

} // namespace matisse

#endif // MATISSE_AREA_COMPUTATION_VISITOR_H_
