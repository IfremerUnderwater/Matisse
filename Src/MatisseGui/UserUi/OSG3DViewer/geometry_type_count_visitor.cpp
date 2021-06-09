#include "geometry_type_count_visitor.h"

#include <osg/Array>
#include <osg/Geometry>
#include <osg/Vec4>


GeometryTypeCountVisitor::GeometryTypeCountVisitor(): osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    m_nb_points = 0;
    m_nb_lignes = 0;
    m_nb_triangles = 0;
    m_nb_others = 0;
}


GeometryTypeCountVisitor::~GeometryTypeCountVisitor()
{
}

void GeometryTypeCountVisitor::apply (osg::Node &_node )
{
    traverse(_node);
}

void GeometryTypeCountVisitor::apply(osg::Geode &_geode )
{
    // Count geometry type (our model 3D)

    unsigned int num_drawables = _geode.getNumDrawables();
    for( unsigned int i = 0; i < num_drawables; i++ )
    {
        // Use 'asGeometry' as its supposed to be faster than a dynamic_cast
        // every little saving counts
        osg::Geometry *current_geometry = _geode.getDrawable(i)->asGeometry();

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
                    m_nb_points++;
                break;

                case osg::PrimitiveSet::LINES:
                case osg::PrimitiveSet::LINE_STRIP:
                case osg::PrimitiveSet::LINE_LOOP:
                    m_nb_lignes++;

                case osg::PrimitiveSet::TRIANGLE_STRIP:
                case osg::PrimitiveSet::TRIANGLES:
                    m_nb_triangles++;
                    break;

                default:
                    m_nb_others++;
                    break;
                }
            }
        }
    }
}
