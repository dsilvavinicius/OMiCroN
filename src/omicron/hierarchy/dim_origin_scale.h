#ifndef DIM_ORIGIN_SCALE
#define DIM_ORIGIN_SCALE

#include "omicron/hierarchy/octree_dimensions.h"

namespace omicron::hierarchy
{
    /** Structure that contains the dimensions, origin and scale of an octree. */
    template< typename Morton >
    class DimOriginScale
    {
    public:
        using OctreeDim = OctreeDimensions< Morton >;
        
        DimOriginScale( const OctreeDim& dim, const Vec3& origin, float scale )
        : m_dim( dim ), m_origin( origin ), m_scale( scale ){}
        
        /** Scales a point. */
        Point& scale( Point& p ) const
        {
            Vec3& pos = p.getPos();
			pos = ( pos - m_origin ) * m_scale;
            
            return p;
        }
        
        const OctreeDim& dimensions() const { return m_dim; }
        
        const Vec3& origin() const { return m_origin; }
        
        const float& scale() const { return m_scale; }
        
    private:
        OctreeDim m_dim;
        Vec3 m_origin;
        float m_scale;
    };
}

#endif
