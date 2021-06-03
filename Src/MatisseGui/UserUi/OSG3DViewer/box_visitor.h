#ifndef MATISSE_BOX_VISITOR_H_
#define MATISSE_BOX_VISITOR_H_

#include <osg/Array>
#include <osg/Geode>
#include <osg/NodeVisitor>

namespace matisse {

class BoxVisitor : public osg::NodeVisitor
{
public :
    BoxVisitor();
    virtual ~BoxVisitor();

    virtual void apply ( osg::Node &node );
    virtual void apply( osg::Geode &geode );

    osg::BoundingBox getBoundingBox() { return m_box; }

private :
    osg::BoundingBox m_box;
    double m_x_min;
    double m_x_max;
    double m_y_min;
    double m_y_max;
    double m_z_min;
    double m_z_max;
};

} // namespace matisse

#endif // MATISSE_BOX_VISITOR_H_
