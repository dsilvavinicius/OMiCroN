#ifndef RANDOM_SAMPLE__OCTREE
#define RANDOM_SAMPLE__OCTREE

#include <time.h>
#include "Octree.h"

namespace model
{
	/** Octree which inner nodes have points randomly sampled from child nodes. Provides a smooth transition
	 * between level of detail (LOD), but has the cost of more points being rendered per frame. */
	template< typename MortonPrecision, typename Float, typename Vec3 >
	class RandomSampleOctree
	: public Octree< MortonPrecision, Float, Vec3 >
	{
	public:
		RandomSampleOctree( const int& maxPointsPerNode );
	protected:
		/** Creates a new inner node by randomly sampling the points of the child nodes. */
		OctreeNodePtr< MortonPrecision, Float, Vec3 > buildInnerNode(
			const PointVector< Float, Vec3 >& childrenPoints ) const;
		
		/** Put all points of the inner nodes inside the rendering lists. */
		void setupInnerNodeRendering( OctreeNodePtr< MortonPrecision, Float, Vec3 > innerNode,
									  vector< Vec3 >& pointsToDraw, vector< Vec3 >& colorsToDraw ) const;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	RandomSampleOctree< MortonPrecision, Float, Vec3 >::RandomSampleOctree( const int& maxPointsPerNode )
	: Octree< MortonPrecision, Float, Vec3 >::Octree( maxPointsPerNode )
	{
		srand( 1 );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	inline OctreeNodePtr< MortonPrecision, Float, Vec3 > RandomSampleOctree< MortonPrecision, Float, Vec3 >
		::buildInnerNode( const PointVector< Float, Vec3 >& childrenPoints ) const
	{
		unsigned int numChildrenPoints = childrenPoints.size();
		
		auto node = make_shared< RamdomInnerNode< MortonPrecision, Float, Vec3 > >();
		PointVector< Float, Vec3 > selectedPoints( numChildrenPoints * 0.125 );
		
		// Gets random 1/8 of the number of points.
		for( int i = 0; i < numChildrenPoints * 0.125; ++i )
		{
			selectedPoints[ i ] = childrenPoints[ rand() % numChildrenPoints ];
		}
		
		node->setContents( selectedPoints );
		return node;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	inline void RandomSampleOctree< MortonPrecision, Float, Vec3 >::setupInnerNodeRendering(
		OctreeNodePtr< MortonPrecision, Float, Vec3 > innerNode,
		vector< Vec3 >& pointsToDraw, vector< Vec3 >& colorsToDraw ) const
	{
		assert( !innerNode.isLeaf() );
		
		PointVectorPtr< Float, Vec3 > points = innerNode-> template getContents< PointVector< Float, Vec3 > >();
		for( PointPtr< Float, Vec3 > point : *points )
		{
			pointsToDraw.push_back( *point->getPos() );
			colorsToDraw.push_back( *point->getColor() );
		}
	}
	
	/** RandomSampleOctree with 32 bit morton code. */
	template< typename Float, typename Vec3 >
	using ShallowRandomSampleOctree = RandomSampleOctree< unsigned int, Float, Vec3 >;
	
	template< typename Float, typename Vec3 >
	using ShallowRandomSampleOctreePtr = shared_ptr< ShallowRandomSampleOctree< Float, Vec3 > >;
	
	/** RandomSampleOctree with 64 bit morton code. */
	template< typename Float, typename Vec3 >
	using MediumRandomSampleOctree = RandomSampleOctree< unsigned long, Float, Vec3 >;
	
	template< typename Float, typename Vec3 >
	using MediumRandomSampleOctreePtr = shared_ptr< MediumRandomSampleOctree< Float, Vec3 > >;
}

#endif