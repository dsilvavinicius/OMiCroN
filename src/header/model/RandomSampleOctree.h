#ifndef RANDOM_SAMPLE__OCTREE_H
#define RANDOM_SAMPLE__OCTREE_H

#include "Octree.h"
#include <time.h>

namespace model
{
	/** Octree which inner nodes have points randomly sampled from child nodes. Provides a smooth transition
	 * between level of detail (LOD), but has the cost of more points being rendered per frame. */
	template< typename MortonCode, typename Point >
	class RandomSampleOctree
	: public Octree< MortonCode, Point >
	{
		using MortonCodePtr = shared_ptr< MortonCode >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		using OctreeNodePtr = model::OctreeNodePtr< MortonCode >;
		using LeafNode = model::LeafNode< MortonCode, PointVector >;
		using OctreeMap = model::OctreeMap< MortonCode >;
		using Octree = model::Octree< MortonCode, Point >;
		using RandomPointAppender = model::RandomPointAppender< MortonCode, Point >;
		
	public:
		RandomSampleOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		template <typename M, typename P >
		friend ostream& operator<<( ostream& out, const RandomSampleOctree< M, P >& octree );
		
	protected:
		virtual void buildInnerNode( typename OctreeMap::iterator& firstChildIt,
									 const typename OctreeMap::iterator& currentChildIt, const MortonCodePtr& parentCode,
							   const vector< OctreeNodePtr >& children ) override;
		
		/** Put all points of the inner nodes inside the rendering lists. */
		virtual void setupInnerNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState )
		override;
		
		virtual void setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState );
		
		virtual void eraseNodes( const typename OctreeMap::iterator& first, const typename OctreeMap::iterator& last );
		
	private:
		/** Creates a new inner node by randomly sampling the points of the child nodes. */
		OctreeNodePtr buildInnerNode( const PointVector& childrenPoints ) const;
	};
	
	template< typename MortonCode, typename Point >
	RandomSampleOctree< MortonCode, Point >::RandomSampleOctree( const int& maxPointsPerNode, const int& maxLevel )
	: Octree::Octree( maxPointsPerNode, maxLevel )
	{
		delete Octree::m_pointAppender;
		Octree::m_pointAppender = new RandomPointAppender();
		srand( 1 );
	}
	
	template< typename MortonCode, typename Point >
	inline void RandomSampleOctree< MortonCode, Point >::buildInnerNode(
		typename OctreeMap::iterator& firstChildIt, const typename OctreeMap::iterator& currentChildIt,
		const MortonCodePtr& parentCode, const vector< OctreeNodePtr >& children )
	{
		// These counters are used to check if the accumulated number of child node points is less than a threshold.
		// In this case, the children are deleted and their points are merged into the parent.
		int numChildren = 0;
		int numLeaves = 0;
		
		// Points to be accumulated for LOD or to be merged into the parent.
		auto childrenPoints = PointVector();
		
		for( OctreeNodePtr child : children )
		{
			Octree::m_pointAppender->appendPoints( child, childrenPoints, numChildren, numLeaves );
		}

		if( numChildren == numLeaves && childrenPoints.size() <= Octree::m_maxPointsPerNode )
		{
			// All children are leaves, but they have less points than the threshold and must be merged.
			auto tempIt = firstChildIt;
			advance( firstChildIt, numChildren );
			
			eraseNodes( tempIt, currentChildIt );
			
			// Creates leaf to replace children.
			shared_ptr< LeafNode > mergedNode( new LeafNode() );
			mergedNode->setContents( childrenPoints );
			
			( *Octree::m_hierarchy )[ parentCode ] = mergedNode;
		}
		else
		{
			// No merge or absorption is needed. Just does LOD.
			advance( firstChildIt, numChildren );
			
			( *Octree::m_hierarchy )[ parentCode ] = buildInnerNode( childrenPoints );
		}
	}
	
	template< typename MortonCode, typename Point >
	inline OctreeNodePtr< MortonCode > RandomSampleOctree< MortonCode, Point >
	::buildInnerNode( const PointVector& childrenPoints ) const
	{
		unsigned int numChildrenPoints = childrenPoints.size();
		
		InnerNodePtr< MortonCode, PointVector > node( new InnerNode< MortonCode, PointVector > () );
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
	
	template< typename MortonCode, typename Point >
	inline void RandomSampleOctree< MortonCode, Point >::setupInnerNodeRendering( OctreeNodePtr innerNode,
																				  MortonCodePtr code,
																			   RenderingState& renderingState )
	{
		assert( !innerNode->isLeaf() && "innerNode cannot be leaf." );
		
		setupNodeRendering( innerNode, renderingState );
	}
	
	template< typename MortonCode, typename Point >
	inline void RandomSampleOctree< MortonCode, Point >::setupNodeRendering( OctreeNodePtr node,
																			 RenderingState& renderingState )
	{
		PointVector points = node-> template getContents< PointVector >();
		renderingState.handleNodeRendering( points );
	}
	
	template< typename MortonCode, typename Point >
	inline void RandomSampleOctree< MortonCode, Point >::eraseNodes( const typename OctreeMap::iterator& first,
																	 const typename OctreeMap::iterator& last )
	{
		Octree::eraseNodes( first, last );
	}
	
	template< typename MortonCode, typename Point >
	ostream& operator<<( ostream& out, const RandomSampleOctree< MortonCode, Point >& octree )
	{
		using PointVector = vector< shared_ptr< Point > >;
		using MortonCodePtr = shared_ptr< MortonCode >;
		using OctreeMapPtr = model::OctreeMapPtr< MortonCode >;
		using OctreeNodePtr = model::OctreeNodePtr< MortonCode >;
		
		out << endl << "=========== Begin Octree ============" << endl << endl
			<< "origin: " << glm::to_string(*octree.m_origin) << endl
			<< "size: " << glm::to_string(*octree.m_size) << endl
			<< "leaf size: " << glm::to_string(*octree.m_leafSize) << endl
			<< "max points per node: " << octree.m_maxPointsPerNode << endl << endl;
		
		/** Maximum level of this octree. */
		unsigned int m_maxLevel;
		OctreeMapPtr hierarchy = octree.getHierarchy();
		for( auto nodeIt = hierarchy->begin(); nodeIt != hierarchy->end(); ++nodeIt )
		{
			MortonCodePtr code = nodeIt->first;
			OctreeNodePtr genericNode = nodeIt->second;
			
			out << "Node: {" << endl << *code << "," << endl;
			operator<< < MortonCode, PointVector >( out, *genericNode );
			out << endl << "}" << endl << endl;
		}
		out << "=========== End Octree ============" << endl << endl;
		return out;
	}
	
	// ====================== Type Sugar ================================ /
	using ShallowRandomSampleOctree = RandomSampleOctree< ShallowMortonCode, Point >;
	using ShallowRandomSampleOctreePtr = shared_ptr< ShallowRandomSampleOctree >;
	
	using MediumRandomSampleOctree = RandomSampleOctree< MediumMortonCode, Point >;
	using MediumRandomSampleOctreePtr = shared_ptr< MediumRandomSampleOctree >;
	
	using ShallowExtRandomSampleOctree = RandomSampleOctree< ShallowMortonCode, ExtendedPoint >;
	using ShallowExtRandomSampleOctreePtr = shared_ptr< ShallowExtRandomSampleOctree >;
	
	using MediumExtRandomSampleOctree = RandomSampleOctree< MediumMortonCode, ExtendedPoint >;
	using MediumExtRandomSampleOctreePtr = shared_ptr< MediumExtRandomSampleOctree >;
}

#endif