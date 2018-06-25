#ifndef TOP_DOWN_FRONT_OCTREE_H
#define TOP_DOWN_FRONT_OCTREE_H

#include <jsoncpp/json/json.h>
#include "Front.h"
#include "OctreeDimensions.h"
#include "RuntimeSetup.h"
#include "PointSorter.h"

namespace omicron::hierarchy
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
		TopDownFrontOctree( const string&, const int maxLvl, NodeLoader&, const RuntimeSetup& );
		
		OctreeStats trackFront( Renderer& renderer, const Float projThresh );
	
		/** Just here to fit FastParallelOctree interface. */
		void waitCreation(){}
		
		bool isCreationFinished() { return true; }
		
		/** Traverses the hierarchy to calculate its number of nodes and node contents.
		 * @return If the hierarchy is already finished, a pair with first value equals to the number of nodes in the
		 * hierarchy and second values equals to the the number of contents in all nodes. */
		pair< uint, uint > nodeStatistics() const;
		
		/** Just here to fit FastParallelOctree interface. */
		int hierarchyCreationDuration() const { return m_hierarchyConstructionTime; }
		
		const Dim& dim() const { return m_dim; }
		
		Node& root() { return *m_root; }
		
		uint substitutedPlaceholders() const{ return 0u; }
		
		uint readerInTime() { return m_inTime; }
		uint readerInitTime() { return 0; }
		uint readerReadTime() { return 0; }
		
	private:
		using SurfelVector = vector< Surfel, TbbAllocator< Surfel > >;
		using ChildVector = vector< Node, typename Node::NodeAlloc >;
		
		void subdivide( Node& node, const Dim& dim );
		
		unique_ptr< Front > m_front;
		unique_ptr< Node > m_root;
		Dim m_dim;
		uint m_inTime;
		uint m_hierarchyConstructionTime;
		int m_maxLvl;
	};
	
	template< typename Morton >
	inline TopDownFrontOctree< Morton >::TopDownFrontOctree( const Json::Value& octreeJson, NodeLoader& nodeLoader, const RuntimeSetup& )
	{
		throw logic_error( "TopDownFrontOctree creation from octree file is unsuported." );
	}
	
	template< typename Morton >
	inline TopDownFrontOctree< Morton >::TopDownFrontOctree( const string& plyFilename, const int maxLvl, NodeLoader& loader, const RuntimeSetup& setup )
	: m_maxLvl( maxLvl )
	{
		srand( 1 );
		
		typename Node::ContentsArray surfels;
		
		chrono::system_clock::time_point now;
		
		{
			PointSorter< Morton > sorter( plyFilename, maxLvl );
			PointSet< Morton > pointSet = sorter.points();
			m_dim = pointSet.m_dim;
			
			m_inTime = sorter.inputTime();
			
			now = Profiler::now( "Hierarchy construction" );
			
			surfels = typename Node::ContentsArray( pointSet.m_points->size() );
			auto iter = surfels.begin();
			while( !pointSet.m_points->empty() )
			{
				*iter = Surfel( pointSet.m_points->front() );
				iter++;
				pointSet.m_points->pop_front();
			}
		}
		
		m_root = unique_ptr< Node >( new Node( std::move( surfels ), true ) );
		
		subdivide( *m_root, Dim( m_dim, 0 ) );
		
		m_front = unique_ptr< Front >( new Front( "", m_dim, 1, loader, 8ul * 1024ul * 1024ul * 1024ul ) );
		m_front->insertRoot( *m_root );
		
		m_hierarchyConstructionTime = Profiler::elapsedTime( now, "Hierarchy construction" );
	}
	
	template< typename Morton >
	void TopDownFrontOctree< Morton >::subdivide( Node& node, const Dim& dim )
	{
		if( node.getContents().size() > TOP_DOWN_OCTREE_K && dim.level() < m_maxLvl )
		{
			Dim nextLvlDim = dim.levelBellow();
			typename Node::ContentsArray innerContents( TOP_DOWN_OCTREE_K );
			
			{
				ChildVector newChildren;
				SurfelVector surfelsPerChild[ 8 ];
				
				for( int i = 0; i < TOP_DOWN_OCTREE_K; ++i )
				{
					innerContents[ i ] = node.getContents()[ rand() % node.getContents().size() ];
				}
				
				for( const Surfel& surfel : node.getContents() )
				{
					surfelsPerChild[ nextLvlDim.calcMorton( surfel ).getChildIdx() ].push_back( surfel );
				}																																												
				
				for( SurfelVector surfelVector: surfelsPerChild )
				{
					if( !surfelVector.empty() )
					{
						newChildren.push_back( Node( typename Node::ContentsArray( std::move( surfelVector ) ), &node ) );
					}
				}
				
				node = Node( std::move( innerContents ), node.parent(), typename Node::NodeArray( std::move( newChildren ) ) );
			}
			
			for( Node& child : node.child() )
			{
				subdivide( child, nextLvlDim );
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
