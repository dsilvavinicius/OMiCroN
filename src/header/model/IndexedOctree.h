#ifndef INDEXED_OCTREE_H
#define INDEXED_OCTREE_H

#include "ExtendedPoint.h"
#include "RandomSampleOctree.h"

namespace model
{
	/** An octree that uses indices to a vector of points in the hierarchy. */
	template< typename MortonCode, typename Point >
	class IndexedOctree
	: public RandomSampleOctree< MortonCode, Point >
	{
		using MortonCodePtr = shared_ptr< MortonCode >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using IndexVector = vector< unsigned int >;
		using IndexVectorPtr = shared_ptr< IndexVector >;
		using OctreeNode = model::OctreeNode< MortonCode >;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		using LeafNode = model::LeafNode< MortonCode, IndexVector >;
		using OctreeMap = model::OctreeMap< MortonCode >;
		using RandomSampleOctree = model::RandomSampleOctree< MortonCode, Point >;
		
	public:
		IndexedOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		virtual void build( PointVector& points ) override;
		
		virtual PointVector& getPoints(){ return m_points; } 
		
	protected:
		virtual void buildNodes( PointVector& points ) override;
		
		virtual void insertPointInLeaf( const PointPtr& point ) override;
		
		virtual void buildInnerNode( typename OctreeMap::iterator& firstChildIt,
									 const typename OctreeMap::iterator& currentChildIt, const MortonCodePtr& parentCode,
							   const vector< OctreeNodePtr >& children ) override;
		
		virtual void setupInnerNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState )
		override;
		
		virtual void setupLeafNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState )
		override;
		
		virtual void setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState ) override;
		
		/** Current number of points in octree. */
		unsigned long m_nPoints;
		
	private:
		OctreeNodePtr buildInnerNode( const IndexVector& childrenPoints ) const;
		
		/** Point vector which will be indexed in the actual hierarchy. */
		PointVector m_points;
	};
	
	template< typename MortonCode, typename Point >
	IndexedOctree< MortonCode, Point >::IndexedOctree( const int& maxPointsPerNode, const int& maxLevel )
	: RandomSampleOctree::RandomSampleOctree( maxPointsPerNode, maxLevel ),
	m_nPoints( 0 ) {}
	
	template< typename MortonCode, typename Point >
	void IndexedOctree< MortonCode, Point >::build( PointVector& points )
	{
		m_points = points;
		points.clear();
		
		RandomSampleOctree::buildBoundaries( m_points );
		buildNodes( m_points );
	}
	
	template< typename MortonCode, typename Point >
	void IndexedOctree< MortonCode, Point >::buildNodes( PointVector& points )
	{
		cout << "Before leaf nodes build." << endl << endl;
		RandomSampleOctree::buildLeaves(points);
		cout << "After leaf nodes build." << endl << endl;
		RandomSampleOctree::buildInners();
		cout << "After inner nodes build." << endl << endl;
	}
	
	template< typename MortonCode, typename Point >
	inline void IndexedOctree< MortonCode, Point >::insertPointInLeaf( const PointPtr& point )
	{
		MortonCodePtr code = make_shared< MortonCode >( RandomSampleOctree::calcMorton( *point ) );
		typename OctreeMap::iterator genericLeafIt = RandomSampleOctree::m_hierarchy->find( code );
		
		unsigned long index = m_nPoints++;
		
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
			IndexVector& indices = leafNode-> template getContents< IndexVector >();
			indices.push_back( index );
		}
	}
	
	template< typename MortonCode, typename Point >
	inline void IndexedOctree< MortonCode, Point >::buildInnerNode(
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
			
			RandomSampleOctree::eraseNodes( tempIt, currentChildIt );
			
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
	
	template< typename MortonCode, typename Point >
	inline OctreeNodePtr< MortonCode > IndexedOctree< MortonCode, Point >
		::buildInnerNode( const IndexVector& childrenPoints ) const
	{
		unsigned int numChildrenPoints = childrenPoints.size();
		
		auto node = make_shared< InnerNode< MortonCode, IndexVector > >();
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
	
	template< typename MortonCode, typename Point >
	inline void IndexedOctree< MortonCode, Point >::setupInnerNodeRendering( OctreeNodePtr innerNode,
																							   MortonCodePtr code,
																							RenderingState& renderingState )
	{
		assert( !innerNode->isLeaf() && "innerNode cannot be leaf." );
		
		setupNodeRendering( innerNode, renderingState );
	}
	
	template< typename MortonCode, typename Point >
	inline void IndexedOctree< MortonCode, Point >::setupLeafNodeRendering( OctreeNodePtr leafNode, 
																							  MortonCodePtr code,
																						   RenderingState& renderingState )
	{
		assert( leafNode->isLeaf() && "leafNode cannot be inner." );
		
		setupNodeRendering( leafNode, renderingState );
	}
	
	template< typename MortonCode, typename Point >
	inline void IndexedOctree< MortonCode, Point >::setupNodeRendering( OctreeNodePtr node,
																						  RenderingState& renderingState )
	{
		const IndexVector& points = node-> template getContents< IndexVector >();
		renderingState.handleNodeRendering( points );
	}
	
	// ====================== Type Sugar ================================ /
	using ShallowIndexedOctree = IndexedOctree< ShallowMortonCode, Point >;
	using ShallowIndexedOctreePtr = shared_ptr< ShallowIndexedOctree >;
	
	using MediumIndexedOctree = IndexedOctree< MediumMortonCode, Point >;
	using MediumIndexedOctreePtr = shared_ptr< MediumIndexedOctree >;
	
	using ShallowExtIndexedOctree = IndexedOctree< ShallowMortonCode, ExtendedPoint >;
	using ShallowExtIndexedOctreePtr = shared_ptr< ShallowExtIndexedOctree >;
	
	using MediumExtIndexedOctree = IndexedOctree< MediumMortonCode, ExtendedPoint >;
	using MediumExtIndexedOctreePtr = shared_ptr< MediumExtIndexedOctree >;
}

#endif