#ifndef MATISSE_MIN_MAX_COMPUTATION_VISITOR_H_
#define MATISSE_MIN_MAX_COMPUTATION_VISITOR_H_

#include <osg/Geode>
#include <osg/NodeVisitor>

namespace matisse {

class MinMaxComputationVisitor : public osg::NodeVisitor
{
public:
    MinMaxComputationVisitor();
    virtual ~MinMaxComputationVisitor();

    virtual void apply ( osg::Node &_node );
    virtual void apply( osg::Geode &_geode );

    float getMin() const { return m_min; }
    float getMax() const { return m_max; }

private:
    float m_min;
    float m_max;
};

} // namespace matisse

#endif // MATISSE_MIN_MAX_COMPUTATION_VISITOR_H_
