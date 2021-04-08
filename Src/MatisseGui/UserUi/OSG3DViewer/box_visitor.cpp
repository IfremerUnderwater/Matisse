#include <osg/Geode>
#include <osg/NodeVisitor>
#include <osg/Vec4>
#include <osg/Geometry>
#include <vector>
#include <limits>
#include "box_visitor.h"
using namespace std;

#include "math.h"

BoxVisitor::BoxVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    m_x_min = numeric_limits<double>::max();
    m_x_max = numeric_limits<double>::min();
    m_y_min = numeric_limits<double>::max();
    m_y_max = numeric_limits<double>::min();
}

BoxVisitor::~BoxVisitor()
{
}

void BoxVisitor::apply ( osg::Node &node )
{
    traverse(node);
}

void BoxVisitor::apply( osg::Geode &geode )
{
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
            // For our model 3D, we only have osg::PrimitiveSet::Triangle

            for(unsigned int j = 0; j < primitive_list.size(); j++)
            {

                osg::PrimitiveSet *primitive_set = primitive_list[j];

                for(unsigned int k = 0; k < primitive_set->getNumIndices(); k++)
                {
                    // we use set->index because we want to have the good value for all our triangles from the table of all points
                    double x = (* vertices)[primitive_set->index(k)].x();
                    double y = (* vertices)[primitive_set->index(k)].y();
                    double z = (* vertices)[primitive_set->index(k)].z();
                    if(x < m_x_min) m_x_min = x;
                    if(y < m_y_min) m_y_min = y;
                    if(z < m_z_min) m_z_min = z;
                    if(x > m_x_max) m_x_max = x;
                    if(y > m_y_max) m_y_max = y;
                    if(z > m_z_max) m_z_max = z;

                }
            }
        }
    }
    m_box = osg::BoundingBox(m_x_min, m_y_min, m_z_min, m_x_max, m_y_max, m_z_max);

}
