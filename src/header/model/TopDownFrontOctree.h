#ifndef TOP_DOWN_FRONT_OCTREE_H
#define TOP_DOWN_FRONT_OCTREE_H

#include <jsoncpp/json/json.h>
#include "Front.h"
#include "OctreeDimensions.h"
#include "RuntimeSetup.h"
#include "PointSorter.h"

namespace model
{
	/** A simple octree created top-down. The heuristic is to subdivide everytime a node contains K points.. */
	template< typename Morton >
	class TopDownFrontOctree
	{
	public:
		using Dim = OctreeDimensions< Morton >;
		using Front = model::Front< Morton >;
		using Node = typename Front::Node;
		using NodeLoader = typename Front::NodeLoader;
		using Renderer = SplatRenderer;
		
		/** @param octreeJson is a Json with the octree dimensions and a binary octree file entry.
		 * @param nodeLoader is the GPU loader used by the front to render the octree. */
		TopDownFrontOctree( const Json::Value& octreeJson, NodeLoader& nodeLoader, const RuntimeSetup& );
		
		/** Unsuported. Just here to fit FastParallelOctree interface. */
		TopDownFrontOctree( const string&, const int, NodeLoader&, const RuntimeSetup& );
		
		OctreeStats trackFront( Renderer& renderer, const Float projThresh );
	
		/** Just here to fit FastParallelOctree interface. */
		void waitCreation(){}
		
		bool isCreationFinished() { return true; }
		
		/** Traverses the hierarchy to calculate its number of nodes and node contents.
		 * @return If the hierarchy is already finished, a pair with first value equals to the number of nodes in the
		 * hierarchy and second values equals to the the number of contents in all nodes. */
		pair< uint, uint > nodeStatistics() const;
		
		/** Just here to fit FastParallelOctree interface. */
		int hierarchyCreationDuration() const { return 0; }
		
		const Dim& dim() const { return m_dim; }
		
		Node& root() { return *m_root; }
		
		uint substitutedPlaceholders() const{ return 0u; }
		
	private:
		void insert( const Point& p, Node& node, const Dim& currentLvlDim );
		
		/** Post-process the octree in order to eliminate empty nodes and shrink buffers. */
		void postProcess( Node& node );
		
		static float& getOffsetReference( const Node::ContentsArray& contents ) { return ( contents.end() - 1 )->c[ 0 ]; }
		static float& getOffsetReference( const Node& node ) { return getOffsetReference( node.getContents() ); }
		
		unique_ptr< Front > m_front;
		unique_ptr< Node > m_root;
		Dim m_dim;
	};
	
	template< typename Morton >FrontOctree
	inline TopDownFrontOctree< Morton >::FrontOctree( const Json::Value& octreeJson, NodeLoader& nodeLoader, const RuntimeSetup& )
	{
		throw logic_error( "TopDownFrontOctree creation from octree file is unsuported." );
	}
	
	template< typename Morton >
	inline TopDownFrontOctree< Morton >::FrontOctree( const string& plyFilename, const int maxLvl, NodeLoader& loader, const RuntimeSetup& setup )
	{
		auto now = Profiler::now( "Top-down octree creation" );
		
		PointSorter< Morton > sorter( plyFilename, maxLvl );
		PointSet< Morton > pointSet = sorter.points();
		m_dim = pointSet.m_dim;
		
		typename Node::ContentsArray surfels( TOP_DOWN_OCTREE_K );
		
		m_root = unique_ptr< Node >( new Node( surfels, true ) );
		getOffsetReference( *m_root ) = 0.f;
		
		for( const Point& p : *pointSet.m_points )
		{
			insert( p, *m_root, Dim( m_dim, 0 ) );
		}
		
		Profiler::elapsedTime( now, "Top-down octree creation" );
		
		m_front = unique_ptr< Front >( new Front( "", m_dim, 1, loader, 8ul * 1024ul * 1024ul * 1024ul ) );
		m_front->insertRoot( *m_root );
		
		postProcess( *m_root );
	}
	
	template< typename Morton >
	inline void TopDownFrontOctree::insert( const Point& p, Node& node, const Dim& currentLvlDim )
	{
		if( node.isLeaf() )
		{
			int offset = getOffsetReference( node )++;
			
			Surfel surfel( p );
			
			Vector2f multipliers = ReconstructionParams::calcAcummulatedMultipliers( currentLvlDim.level(), m_dim.level() - 1 );
			surfel.multiplyTangents( multipliers );
			
			node.getContents()[ offset ] = surfel;
			
			if( offset == TOP_DOWN_OCTREE_K )
			{
				// Subdivision.
				Dim nextLvlDim = currentLvlDim.levelBellow();
				
				int movedToInner = 0;
				
				Node newInner( Node::ContentsArray( TOP_DOWN_OCTREE_K / 8 ), node.parent(), Node::NodeArray( 8 ) );
				
				// Calc the multipliers needed to reverse the tangent multiplication done for the current lvl.
				Vector2f reverseMultipliers = ReconstructionParams::calcMultipliers( currentLvlDim.level() );
				reverseMultipliers.x() = 1.f / reverseMultipliers.x();
				reverseMultipliers.y() = 1.f / reverseMultipliers.y();
				
				for( const Surfel& surfel : node.getContents() )
				{
					if( movedToInner < newInner.getContents().size() )
					{
						newInner.getContents()[ movedToInner++ ] = surfel;
					}
					
					// Fix multipliers for the next level.
					Surfel nextLvlSurfel = surfel;
					nextLvlSurfel.multiplyTangents( reverseMultipliers );
					
					int childIdx = nextLvlDim.calcMorton( surfel.c ).getChildIdx();
					Node& child = newInner.child()[ childIdx ]; 
					
					if( child.isEmpty() )
					{
						Node::ContentsArray newChildSurfels( TOP_DOWN_OCTREE_K );
						newChildSurfels[ 0 ] = nextLvlSurfel;
						getOffsetReference( newChildSurfels ) = 1;
						
						Node newChild( std::move( newChildSurfels ), &node );
						
						newInner.child()[ childIdx ] = std::move( newChild );
					}
				}
				
				node = std::move( newInner );
			}
		}
		else
		{
			Dim nextLvlDim = currentLvlDim.levelBellow();
			Morton morton = nextLvlDim.calcMorton( p );
			
			insert( p, node.child()[ morton.getChildIdx() ], nextLvlDim );
		}
	}
	
	template< typename Morton >
	void TopDownFrontOctree< Morton >::postProcess( Node& node )
	{
		int contentsSize = getOffsetReference( node );
		if( contentsSize < TOP_DOWN_OCTREE_K / 8 )
		{
			// Shrink.
			if( !node.isLeaf() )
			{
				throw logic_error( "All inner nodes are expected to have TOP_DOWN_OCTREE_K / 8 surfels." );
			}
			
			Node::ContentsArray newSurfelArray( contentsSize );
			auto iter = newSurfelArray.begin();
			for( const Surfel& surfel : node.getContents() )
			{
				*iter = surfel;
				iter++;
			}
		}
		
		// Check for empty children.
		if( !node.isLeaf() )
		{
			int nonEmptyChildren = 0;
			for( const Node& child : node.child )
			{
				if( !child.empty() )
				{
					nonEmptyChildren++;
				}
			}
			
			if( nonEmptyChildren < 8 )
			{
				// Empty removal needed.
				Node::NodeArray newChildren( nonEmptyChildren );
				auto iter = newChildren.begin();
				for( Node& child : node.child )
				{
					if( !child.empty() )
					{
						*iter = std::move( child );
						iter++;
					}
				}
				
				node.setChildren( std::move( newChildren ) ); 
			}
		}
	}

	
	template< typename Morton >
	inline OctreeStats TopDownFrontOctree< Morton >::trackFront( Renderer& renderer, const Float projThresh )
	{
		m_front->trackFront( renderer, projThresh );
	}
	
	template< typename Morton >
	pair< uint, uint > TopDownFrontOctree< Morton >::nodeStatistics() const
	{
		return m_root->subtreeStatistics();
	}
}

#endif