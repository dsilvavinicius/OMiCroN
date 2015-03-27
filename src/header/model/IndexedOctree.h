#ifndef INDEXED_OCTREE_H
#define INDEXED_OCTREE_H

#include "ExtendedPoint.h"
#include "RandomSampleOctree.h"

namespace model
{
	/** An octree that uses indices to a vector of points in the hierarchy. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class IndexedOctree
	: public RandomSampleOctree< MortonPrecision, Float, Vec3, Point >
	{
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonCodePtr = shared_ptr< MortonCode >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = model::PointVector< Float, Vec3 >;
		using IndexVector = vector< unsigned int >;
		using IndexVectorPtr = shared_ptr< IndexVector >;
		using OctreeNode = model::OctreeNode< MortonPrecision, Float, Vec3 >;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		using LeafNode = model::LeafNode< MortonPrecision, Float, Vec3, IndexVector >;
		using OctreeMap = model::OctreeMap< MortonPrecision, Float, Vec3 >;
		using RandomSampleOctree = model::RandomSampleOctree< MortonPrecision, Float, Vec3, Point >;
		using RenderingState = model::RenderingState< Vec3, Float >;
		
	public:
		IndexedOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		PointVector& getPoints(){ return m_points; } 
		
	protected:
		void insertPointInLeaf( const PointPtr& point, const MortonCodePtr& code );
		
		void buildInnerNode( typename OctreeMap::iterator& firstChildIt, const typename OctreeMap::iterator& currentChildIt,
							 const MortonCodePtr& parentCode, const vector< OctreeNodePtr >& children );
		
		void setupInnerNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState );
		
		void setupLeafNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState );
		
		void setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState );
		
	private:
		OctreeNodePtr buildInnerNode( const IndexVector& childrenPoints ) const;
		
		/** Point vector which will be indexed in the actual hierarchy. */
		PointVector m_points;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	IndexedOctree< MortonPrecision, Float, Vec3, Point >::IndexedOctree( const int& maxPointsPerNode, const int& maxLevel )
	: RandomSampleOctree::RandomSampleOctree( maxPointsPerNode, maxLevel ) {}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void IndexedOctree< MortonPrecision, Float, Vec3, Point >::insertPointInLeaf( const PointPtr& point,
																						 const MortonCodePtr& code )
	{
		typename OctreeMap::iterator genericLeafIt = RandomSampleOctree::m_hierarchy->find( code );
		
		unsigned int index = m_points.size();
		m_points.push_back( point );
		
		if( genericLeafIt == RandomSampleOctree::m_hierarchy->end() )
		{
			// Creates leaf node.
			IndexVector indices;
			indices.push_back( index );
			auto leafNode = make_shared< LeafNode >();
			leafNode->setContents( indices );
			( *RandomSampleOctree::m_hierarchy )[ code ] = leafNode;
		}
		else
		{
			// Node already exists. Appends the point there.
			OctreeNodePtr leafNode = genericLeafIt->second;
			shared_ptr< IndexVector > indices = leafNode-> template getContents< IndexVector >();
			indices->push_back( index );
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void IndexedOctree< MortonPrecision, Float, Vec3, Point >::buildInnerNode(
		typename OctreeMap::iterator& firstChildIt, const typename OctreeMap::iterator& currentChildIt,
		const MortonCodePtr& parentCode, const vector< OctreeNodePtr >& children )
	{
		// These counters are used to check if the accumulated number of child node points is less than a threshold.
		// In this case, the children are deleted and their points are merged into the parent.
		int numChildren = 0;
		int numLeaves = 0;
		
		// Points to be accumulated for LOD or to be merged into the parent.
		auto childrenPoints = IndexVector();
		
		//cout << "numChildren: " << numChildren << endl << "numLeaves" << numLeaves << endl;
		
		for( OctreeNodePtr child : children )
		{
			RandomSampleOctree::m_pointAppender->appendPoints( child, childrenPoints, numChildren, numLeaves );
		}
		
		if( numChildren == numLeaves && childrenPoints.size() <= RandomSampleOctree::m_maxPointsPerNode )
		{
			//cout << "Will merge children." << endl << endl;
			// All children are leaves, but they have less points than the threshold and must be merged.
			auto tempIt = firstChildIt;
			advance( firstChildIt, numChildren );
			
			RandomSampleOctree::m_hierarchy->erase( tempIt, currentChildIt );
			
			// Creates leaf to replace children.
			auto mergedNode = make_shared< LeafNode >();
			mergedNode->setContents( childrenPoints );
			
			( *RandomSampleOctree::m_hierarchy )[ parentCode ] = mergedNode;
		}
		else
		{
			//cout << "Just LOD." << endl << endl;
			// No merge or absorption is needed. Just does LOD.
			advance( firstChildIt, numChildren );
			
			( *RandomSampleOctree::m_hierarchy )[ parentCode ] = buildInnerNode( childrenPoints );
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline OctreeNodePtr< MortonPrecision, Float, Vec3 > IndexedOctree< MortonPrecision, Float, Vec3, Point >
		::buildInnerNode( const IndexVector& childrenPoints ) const
	{
		unsigned int numChildrenPoints = childrenPoints.size();
		
		auto node = make_shared< InnerNode< MortonPrecision, Float, Vec3, IndexVector > >();
		int numSamplePoints = std::max( 1., numChildrenPoints * 0.125 );
		IndexVector selectedPoints( numSamplePoints );
		
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
	inline void IndexedOctree< MortonPrecision, Float, Vec3, Point >::setupInnerNodeRendering( OctreeNodePtr innerNode,
																							   MortonCodePtr code,
																							RenderingState& renderingState )
	{
		assert( !innerNode->isLeaf() && "innerNode cannot be leaf." );
		
		setupNodeRendering( innerNode, renderingState );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void IndexedOctree< MortonPrecision, Float, Vec3, Point >::setupLeafNodeRendering( OctreeNodePtr leafNode, 
																							  MortonCodePtr code,
																						   RenderingState& renderingState )
	{
		assert( leafNode->isLeaf() && "leafNode cannot be inner." );
		
		setupNodeRendering( leafNode, renderingState );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void IndexedOctree< MortonPrecision, Float, Vec3, Point >::setupNodeRendering( OctreeNodePtr node,
																						  RenderingState& renderingState )
	{
		IndexVectorPtr points = node-> template getContents< IndexVector >();
		renderingState.handleNodeRendering( points );
	}
	
	// ====================== Type Sugar ================================ /
	
	template< typename Float, typename Vec3, typename Point >
	using ShallowIndexedOctree = RandomSampleOctree< unsigned int, Float, Vec3, Point >;
}

#endif