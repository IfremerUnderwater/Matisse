#include "geometry_type_count_visitor.h"

#include <osg/Array>
#include <osg/Geometry>
#include <osg/Vec4>


GeometryTypeCountVisitor::GeometryTypeCountVisitor(): osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    m_nbPoints = 0;
    m_nbLignes = 0;
    m_nbTriangles = 0;
    m_nbOthers = 0;
}


GeometryTypeCountVisitor::~GeometryTypeCountVisitor()
{
}

void GeometryTypeCountVisitor::apply ( osg::Node &node )
{
    traverse(node);
}

void GeometryTypeCountVisitor::apply( osg::Geode &geode )
{
    // Count geometry type (our model 3D)

    unsigned int num_drawables = geode.getNumDrawables();
    for( unsigned int i = 0; i < num_drawables; i++ )
    {
        // Use 'asGeometry' as its supposed to be faster than a dynamic_cast
        // every little saving counts
        osg::Geometry *current_geometry = geode.getDrawable(i)->asGeometry();

        // Only process if the drawable is geometry
        if ( current_geometry )
        {
            // get the list of different geometry mode which were created
            osg::Geometry::PrimitiveSetList primitive_list = current_geometry->getPrimitiveSetList();

            for(unsigned int j = 0; j < primitive_list.size(); j++)
            {

                osg::PrimitiveSet *primitive_set = primitive_list[j];

                switch(primitive_set->getMode())
                {
                case osg::PrimitiveSet::POINTS:
                    m_nbPoints++;
                break;

                case osg::PrimitiveSet::LINES:
                case osg::PrimitiveSet::LINE_STRIP:
                case osg::PrimitiveSet::LINE_LOOP:
                    m_nbLignes++;

                case osg::PrimitiveSet::TRIANGLE_STRIP:
                case osg::PrimitiveSet::TRIANGLES:
                    m_nbTriangles++;
                    break;

                default:
                    m_nbOthers++;
                    break;
                }
            }
        }
    }
}
