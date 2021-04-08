#include <osg/Array>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/NodeVisitor>
#include <osg/Vec4>

#include "area_computation_visitor.h"

#include "math.h"

static inline double sqr(const double _x) { return _x * _x; }

AreaComputationVisitor::AreaComputationVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN)
{
    m_area=0;
}

AreaComputationVisitor::~AreaComputationVisitor()
{
}

void AreaComputationVisitor::apply ( osg::Node &node )
{
    traverse(node);
}

void AreaComputationVisitor::apply( osg::Geode &geode )
{
    // Compute Total Area from a node (our model 3D)

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
                unsigned int inc=0;
                if (primitive_set->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP)
                {
                    inc = 1;
                }
                else if (primitive_set->getMode() == osg::PrimitiveSet::TRIANGLES)
                {
                    inc = 3;
                }
                else
                {
                    return;
                }
                if(primitive_set->getNumIndices() < 3)
                {
                    // avoid segv
                    continue;
                }
                for(unsigned int k = 0; k < primitive_set->getNumIndices()-2; k=k+inc)
                {
                    osg::Vec3f point_a;
                    osg::Vec3f point_b;
                    osg::Vec3f point_c;

                    // we use set->index because we want to have the good value for all our triangles from the table of all points
                    point_a = osg::Vec3f((* vertices)[primitive_set->index(k)].x(), (* vertices)[primitive_set->index(k)].y(), (* vertices)[primitive_set->index(k)].z()) ;
                    point_b = osg::Vec3f((* vertices)[primitive_set->index(k+1)].x(), (* vertices)[primitive_set->index(k+1)].y(), (* vertices)[primitive_set->index(k+1)].z()) ;
                    point_c = osg::Vec3f((* vertices)[primitive_set->index(k+2)].x(), (* vertices)[primitive_set->index(k+2)].y(), (* vertices)[primitive_set->index(k+2)].z()) ;

                    double AB = sqrt( sqr(point_a.x()-point_b.x()) + sqr(point_a.y()-point_b.y()) + sqr(point_a.z()-point_b.z()) );
                    double BC = sqrt( sqr(point_b.x()-point_c.x()) + sqr(point_b.y()-point_c.y()) + sqr(point_b.z()-point_c.z()) );
                    double AC = sqrt( sqr(point_a.x()-point_c.x()) + sqr(point_a.y()-point_c.y()) + sqr(point_a.z()-point_c.z()) );

                    double perimeter = (AB + BC + AC)/2;

                    double area = sqrt(perimeter*(perimeter-AB)*(perimeter-BC)*(perimeter-AC));

                    m_area = m_area + area;
                }

            }

        }
    }
}
