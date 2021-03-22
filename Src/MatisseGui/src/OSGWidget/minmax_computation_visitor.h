#ifndef MINMAXCOMPUTATIONVISITOR_H
#define MINMAXCOMPUTATIONVISITOR_H

#include <osg/Geode>
#include <osg/NodeVisitor>

class MinMaxComputationVisitor : public osg::NodeVisitor
{
public:
    MinMaxComputationVisitor();
    virtual ~MinMaxComputationVisitor();

    virtual void apply ( osg::Node &node );
    virtual void apply( osg::Geode &geode );

    float getMin() const { return m_min; }
    float getMax() const { return m_max; }

private:
    float m_min;
    float m_max;
};

#endif // MINMAXCOMPUTATIONVISITOR_H
