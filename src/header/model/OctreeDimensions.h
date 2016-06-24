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
		using Morton = M;
		
		OctreeDimensions() {}
		
		OctreeDimensions( const Vec3& origin, const Vec3& octreeSize, uint nodeLvl )
		: m_origin( origin ),
		m_size( octreeSize ),
		m_nodeSize( m_size * ( ( Float ) 1 / ( ( unsigned long long ) 1 << nodeLvl ) ) ),
		m_nodeLvl( nodeLvl )
		{}
		
		OctreeDimensions( const OctreeDimensions& other, uint newLvl )
		: m_origin( other.m_origin ),
		m_size( other.m_size ),
		m_nodeSize( m_size * ( ( Float ) 1 / ( ( unsigned long long ) 1 << newLvl ) ) ),
		m_nodeLvl( newLvl )
		{}
		
		void init( const Vec3& origin, const Vec3& octreeSize, uint nodeLvl )
		{
			m_origin = origin;
			m_size = octreeSize;
			m_nodeSize = m_size * ( ( Float ) 1 / ( ( unsigned long long ) 1 << nodeLvl ) );
			m_nodeLvl = nodeLvl;
		}
		
		M calcMorton( const P& point ) const
		{
			const Vec3& pos = point.getPos();
			Vec3 index = ( pos - m_origin ).array() / m_nodeSize.array();
			M code;
			code.build( index.x(), index.y(), index.z(), m_nodeLvl );
			
			return code;
		}
		
		template< typename Node >
		M calcMorton( const Node& node ) const
		{
			return calcMorton( *( node.getContents()[ 0 ] ) );
		}
		
		/** Returns the boundaries of the node identified by the given MortonCode */
		AlignedBox3f getMortonBoundaries( const M& code ) const
		{
			uint level = code.getLevel();
			
			assert( level == m_nodeLvl && "Morton code level should be equal than OctreeDimension's." );
			
			auto nodeCoordsVec = code.decode( level );
			Vec3 nodeCoords( nodeCoordsVec[ 0 ], nodeCoordsVec[ 1 ], nodeCoordsVec[ 2 ] );
			Float nodeSizeFactor = Float( 1 ) / Float( 1 << level );
			Vec3 levelNodeSize = m_size * nodeSizeFactor;
			
			Vec3 minBoxVert = m_origin + ( nodeCoords.array() * levelNodeSize.array() ).matrix();
			Vec3 maxBoxVert = minBoxVert + levelNodeSize;
			
			return AlignedBox3f( minBoxVert, maxBoxVert );
		}
		
		/** Returns the boundaries of the node. */
		template< typename Node >
		AlignedBox3f getNodeBoundaries( const Node& node )
		{
			return getMortonBoundaries( calcMorton( node ) );
		}
		
		bool operator()( const P& p0, const P& p1 ) const
		{
			// Debug
// 			{
// 				cout << calcMorton( p0 ).getPathToRoot( true ) << endl << calcMorton( p1 ).getPathToRoot( true ) << endl;
// 			}
			
			return calcMorton( p0 ) < calcMorton( p1 );
		}
		
		friend ostream& operator<<( ostream& out, const OctreeDimensions& dim )
		{
			out << "origin: " << dim.m_origin << endl
				<< "octree size: " << dim.m_size << endl
				<< "node lvl size:" << dim.m_nodeSize << endl
				<< "node lvl:" << dim.m_nodeLvl << endl;
			return out;
		}
		
		Vec3 m_origin;
		Vec3 m_size;
		Vec3 m_nodeSize;
		uint m_nodeLvl;
	};
}

#endif