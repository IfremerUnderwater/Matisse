#include "minmax_computation_visitor.h"
#include <osg/Array>
#include <osg/Geometry>
#include <osg/Vec4>

#include <limits>

MinMaxComputationVisitor::MinMaxComputationVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    // init to "invalid" values
    m_max = -std::numeric_limits<float>::max();
    m_min = std::numeric_limits<float>::max();
}


MinMaxComputationVisitor::~MinMaxComputationVisitor()
{
}

void MinMaxComputationVisitor::apply ( osg::Node &node )
{
    traverse(node);
}

void MinMaxComputationVisitor::apply( osg::Geode &geode )
{
    // Compute min & max for a node (our model 3D)

    unsigned int num_drawables = geode.getNumDrawables();
    for( unsigned int i = 0; i < num_drawables; i++ )
    {
        // Use 'asGeometry' as its supposed to be faster than a dynamic_cast
        // every little saving counts
        osg::Geometry *current_geometry = geode.getDrawable(i)->asGeometry();

        // Only process if the drawable is geometry
        if ( current_geometry )
        {
            osg::Vec3Array *vertices = dynamic_cast<osg::Vec3Array*>(current_geometry->getVertexArray());

            // get the list of different geometry mode which were created
            osg::Geometry::PrimitiveSetList primitive_list = current_geometry->getPrimitiveSetList();

            for(unsigned int j = 0; j < primitive_list.size(); j++)
            {

                osg::PrimitiveSet *primitive_set = primitive_list[j];

                for(unsigned int k = 0; k < primitive_set->getNumIndices(); k++)
                {
                    float z = (* vertices)[primitive_set->index(k)].z();

                    if(z > m_max)
                        m_max = z;
                    if(z < m_min)
                        m_min = z;
                }
            }
        }
    }
}
