#ifndef RANDOM_SAMPLE__OCTREE_H
#define RANDOM_SAMPLE__OCTREE_H

#include <time.h>
#include "Octree.h"

namespace model
{
	/** Octree which inner nodes have points randomly sampled from child nodes. Provides a smooth transition
	 * between level of detail (LOD), but has the cost of more points being rendered per frame. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class RandomSampleOctree
	: public Octree< MortonPrecision, Float, Vec3, Point >
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		using RenderingState = model::RenderingState< Vec3, Float >;
		using RandomPointAppender = model::RandomPointAppender< MortonPrecision, Float, Vec3, Point >;
		
	public:
		RandomSampleOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		template <typename M, typename F, typename V, typename P >
		friend ostream& operator<<( ostream& out, const RandomSampleOctree< M, F, V, P >& octree );
		
	protected:
		/** Creates a new inner node by randomly sampling the points of the child nodes. */
		OctreeNodePtr< MortonPrecision, Float, Vec3 > buildInnerNode( const PointVector& childrenPoints ) const;
		
		/** Put all points of the inner nodes inside the rendering lists. */
		void setupInnerNodeRendering( OctreeNodePtr< MortonPrecision, Float, Vec3 > innerNode,
									  MortonCodePtr< MortonPrecision > code, RenderingState& renderingState );
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	RandomSampleOctree< MortonPrecision, Float, Vec3, Point >::RandomSampleOctree( const int& maxPointsPerNode,
																				   const int& maxLevel )
	: Octree< MortonPrecision, Float, Vec3, Point >::Octree( maxPointsPerNode, maxLevel )
	{
		Octree< MortonPrecision, Float, Vec3, Point >::m_pointAppender = new RandomPointAppender();
		srand( 1 );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline OctreeNodePtr< MortonPrecision, Float, Vec3 > RandomSampleOctree< MortonPrecision, Float, Vec3, Point >
		::buildInnerNode( const PointVector& childrenPoints ) const
	{
		unsigned int numChildrenPoints = childrenPoints.size();
		
		auto node = make_shared< InnerNode< MortonPrecision, Float, Vec3, PointVector > >();
		int numSamplePoints = std::max( 1., numChildrenPoints * 0.125 );
		PointVector selectedPoints( numSamplePoints );
		
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
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void RandomSampleOctree< MortonPrecision, Float, Vec3, Point >::setupInnerNodeRendering(
		OctreeNodePtr< MortonPrecision, Float, Vec3 > innerNode, MortonCodePtr< MortonPrecision > code,
		RenderingState& renderingState )
	{
		assert( !innerNode->isLeaf() && "innerNode cannot be leaf." );
		
		PointVectorPtr points = innerNode-> template getContents< PointVector >();
		renderingState.handleNodeRendering( points );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	ostream& operator<<( ostream& out, const RandomSampleOctree< MortonPrecision, Float, Vec3, Point >& octree )
	{
		using PointVector = vector< shared_ptr< Point > >;
		
		out << endl << "=========== Begin Octree ============" << endl << endl
			<< "origin: " << glm::to_string(*octree.m_origin) << endl
			<< "size: " << glm::to_string(*octree.m_size) << endl
			<< "leaf size: " << glm::to_string(*octree.m_leafSize) << endl
			<< "max points per node: " << octree.m_maxPointsPerNode << endl << endl;
		
		/** Maximum level of this octree. */
		unsigned int m_maxLevel;
		OctreeMapPtr< MortonPrecision, Float, Vec3 > hierarchy = octree.getHierarchy();
		for( auto nodeIt = hierarchy->begin(); nodeIt != hierarchy->end(); ++nodeIt )
		{
			MortonCodePtr< MortonPrecision > code = nodeIt->first;
			OctreeNodePtr< MortonPrecision, Float, Vec3 > genericNode = nodeIt->second;
			
			out << "Node: {" << endl << *code << "," << endl;
			operator<< < MortonPrecision, Float, Vec3, PointVector >( out, *genericNode );
			out << endl << "}" << endl << endl;
		}
		out << "=========== End Octree ============" << endl << endl;
		return out;
	}
	
	// ====================== Type Sugar ================================ /
	
	/** RandomSampleOctree with 32 bit morton code. */
	template< typename Float, typename Vec3, typename Point >
	using ShallowRandomSampleOctree = RandomSampleOctree< unsigned int, Float, Vec3, Point >;
	
	template< typename Float, typename Vec3, typename Point >
	using ShallowRandomSampleOctreePtr = shared_ptr< ShallowRandomSampleOctree< Float, Vec3, Point > >;
	
	/** RandomSampleOctree with 64 bit morton code. */
	template< typename Float, typename Vec3, typename Point >
	using MediumRandomSampleOctree = RandomSampleOctree< unsigned long, Float, Vec3, Point >;
	
	template< typename Float, typename Vec3, typename Point >
	using MediumRandomSampleOctreePtr = shared_ptr< MediumRandomSampleOctree< Float, Vec3, Point > >;
}

#endif