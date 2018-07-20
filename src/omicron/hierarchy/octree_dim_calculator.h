#ifndef OCTREE_DIM_CALCULATOR_H
#define OCTREE_DIM_CALCULATOR_H

#include "omicron/hierarchy/dim_origin_scale.h"

namespace omicron::hierarchy
{
    using namespace basic;
    
    /** Calculates the normalized dimensions of an octree given its points. The normalization is inside the interval [(0, 0, 0) , (1, 1, 1)], with origin at (0, 0, 0). It preserves the aspect ratio, i.e. just one of the coordinates will have max value equals to 1. */
    template < typename Morton >
    class OctreeDimCalculator
    {
    public:
        using OctreeDim = OctreeDimensions< Morton >;
        using DimOriginScale = hierarchy::DimOriginScale< Morton >;
        
        /** Ctor.
         *  @param onPointReceived is an event called whenever a point is inserted by this.
         */
        OctreeDimCalculator( const function< void( const Point& ) >& onPointInserted );
    
        /** Inserts a point, expanding the octree boundary if needed. */
        void insertPoint( const Point& p );
    
        /** @param maxLevel is the maximum level of the octree.
         * @returns the current dimensions of the octree and the scale used for normalization, given the current points inserted by insertPoint(). */
        DimOriginScale dimensions( uint maxLevel ) const;
        
    private:
        Vec3 m_origin;
		Vec3 m_maxCoords;
        function< void( const Point& ) > m_onPointInserted;
    };
    
    template< typename Morton >
    OctreeDimCalculator< Morton >::OctreeDimCalculator( const function< void( const Point& ) >& onPointInserted )
    : m_origin( numeric_limits< Float >::max(), numeric_limits< Float >::max(), numeric_limits< Float >::max() ),
    m_maxCoords( -m_origin ),
    m_onPointInserted( onPointInserted ) {}
    
    template< typename Morton >
    void OctreeDimCalculator< Morton >::insertPoint( const Point& p )
    {
        const Vec3& pos = p.getPos();
        for( int i = 0; i < 3; ++i )
        {
            m_origin[ i ] = std::min( m_origin[ i ], pos[ i ] );
            m_maxCoords[ i ] = std::max( m_maxCoords[ i ], pos[ i ] );
        }
        
        m_onPointInserted( p );
    }
    
    template< typename Morton >
    DimOriginScale< Morton > OctreeDimCalculator< Morton >::dimensions( uint maxLevel ) const
    {
        Vec3 octreeSize = m_maxCoords - m_origin;
        float scale = 1.f / std::max( std::max( octreeSize.x(), octreeSize.y() ), octreeSize.z() );
        
        OctreeDim dim( Vec3( 0.f, 0.f, 0.f ), octreeSize * scale, maxLevel );
        
        return DimOriginScale( dim, m_origin, scale );
    }
}

#endif
