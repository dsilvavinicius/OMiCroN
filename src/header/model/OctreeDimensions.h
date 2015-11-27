#ifndef OCTREE_DIMENSIONS_H
#define OCTREE_DIMENSIONS_H

#include "BasicTypes.h"

namespace model
{
	/** Contains dimensional info of an octree and utilities for Point and MortonCode octree-dimension-dependent info
	 * acquisition and comparison. */
	template< typename M, typename P >
	class OctreeDimensions
	{
	public:
		OctreeDimensions() {}
		
		OctreeDimensions( const Vec3& origin, const Vec3& octreeSize, const Vec3& leafSize, uint leafLvl )
		: m_origin( m_origin ),
		m_size( octreeSize ),
		m_leafSize( leafSize ),
		m_leafLvl( leafLvl )
		{}
		
		void init( const Vec3& origin, const Vec3& octreeSize, const Vec3& leafSize, uint leafLvl )
		{
			m_origin = origin ;
			m_size = octreeSize;
			m_leafSize = leafSize;
			m_leafLvl = leafLvl;
		}
		
		M calcMorton( const P& point ) const
		{
			const Vec3& pos = point.getPos();
			Vec3 index = ( pos - m_origin ) / m_leafSize;
			M code;
			code.build( index.x, index.y, index.z, m_leafLvl );
			
			return code;
		}
		
		/** Returns the boundaries of the node identified by the given MortonCode */
		pair< Vec3, Vec3 > getBoundaries( const M& code ) const
		{
			uint level = code->getLevel();
			auto nodeCoordsVec = code->decode(level);
			Vec3 nodeCoords( nodeCoordsVec[ 0 ], nodeCoordsVec[ 1 ], nodeCoordsVec[ 2 ] );
			Float nodeSizeFactor = Float( 1 ) / Float( 1 << level );
			Vec3 levelNodeSize = m_size * nodeSizeFactor;
			
			Vec3 minBoxVert = m_origin + nodeCoords * levelNodeSize;
			Vec3 maxBoxVert = minBoxVert + levelNodeSize;
			
			/*cout << "Boundaries for node 0x" << hex << code->getBits() << dec << endl
				<< "level = " << level << endl
				<< "node coordinates = " << glm::to_string(nodeCoords) << endl
				<< "node size factor = " << nodeSizeFactor << endl
				<< "level node size = " << glm::to_string(levelNodeSize) << endl
				<< "min coords = " << glm::to_string(minBoxVert) << endl
				<< "max coords = " << glm::to_string(maxBoxVert) << endl;*/
			
			pair< Vec3, Vec3 > box( minBoxVert, maxBoxVert );
			
			return box;
		}
		
		bool operator()( const P& p0, const P& p1 ) const
		{
			return calcMorton( p0 ) < calcMorton( p1 );
		}
		
		Vec3 m_origin;
		Vec3 m_size;
		Vec3 m_leafSize;
		uint m_leafLvl;
	};
}

#endif