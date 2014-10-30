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
		RandomSampleOctree( const int& maxPointsPerNode, const int& maxLevel );
	protected:
		/** Creates a new inner node by randomly sampling the points of the child nodes. */
		OctreeNodePtr< MortonPrecision, Float, Vec3 > buildInnerNode(
			const PointVector< Float, Vec3 >& childrenPoints ) const;
		
		/** Put all points of the inner nodes inside the rendering lists. */
		void setupInnerNodeRendering( OctreeNodePtr< MortonPrecision, Float, Vec3 > innerNode,
									  vector< Vec3 >& pointsToDraw, vector< Vec3 >& colorsToDraw ) const;

		void appendPoints(OctreeNodePtr< MortonPrecision, Float, Vec3 > node, PointVector< Float, Vec3 >& vec,
						  int& numChildren, int& numLeaves) const;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	RandomSampleOctree< MortonPrecision, Float, Vec3 >::RandomSampleOctree( const int& maxPointsPerNode, const int& maxLevel )
	: Octree< MortonPrecision, Float, Vec3 >::Octree( maxPointsPerNode, maxLevel )
	{
		srand( 1 );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	inline OctreeNodePtr< MortonPrecision, Float, Vec3 > RandomSampleOctree< MortonPrecision, Float, Vec3 >
		::buildInnerNode( const PointVector< Float, Vec3 >& childrenPoints ) const
	{
		unsigned int numChildrenPoints = childrenPoints.size();
		
		auto node = make_shared< RamdomInnerNode< MortonPrecision, Float, Vec3 > >();
		int numSamplePoints = std::max( 1., numChildrenPoints * 0.125 );
		PointVector< Float, Vec3 > selectedPoints( numSamplePoints );
		
		//cout << "num Sample points: " << numSamplePoints << endl << "Children points size:" << childrenPoints.size() << endl;
		
		// Gets random 1/8 of the number of points.
		for( int i = 0; i < numSamplePoints; ++i )
		{
			int choosenPoint = rand() % numChildrenPoints;
			//cout << "Iter " << i << ". Choosen point index: " << choosenPoint << endl;
			selectedPoints[ i ] = childrenPoints[ choosenPoint ];
		}
		
		node->setContents( selectedPoints );
		return node;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	inline void RandomSampleOctree< MortonPrecision, Float, Vec3 >::setupInnerNodeRendering(
		OctreeNodePtr< MortonPrecision, Float, Vec3 > innerNode,
		vector< Vec3 >& pointsToDraw, vector< Vec3 >& colorsToDraw ) const
	{
		assert( !innerNode->isLeaf() );
		
		PointVectorPtr< Float, Vec3 > points = innerNode-> template getContents< PointVector< Float, Vec3 > >();
		for( PointPtr< Float, Vec3 > point : *points )
		{
			pointsToDraw.push_back( *point->getPos() );
			colorsToDraw.push_back( *point->getColor() );
		}
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	inline void RandomSampleOctree< MortonPrecision, Float, Vec3 >::appendPoints(OctreeNodePtr< MortonPrecision,
																				 Float, Vec3 > node,
																				 PointVector< Float, Vec3 >& vec,
																				 int& numChildren, int& numLeaves) const
	{
		++numChildren;
		if( node->isLeaf() )
		{
			++numLeaves;
		}
		
		PointVectorPtr< Float, Vec3 > childPoints = node-> template getContents< PointVector< Float, Vec3 > >();
		vec.insert( vec.end(), childPoints->begin(), childPoints->end() );
	}
	
	// ====================== Type Sugar ================================ /
	
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