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
		FrontOctree( const Json::Value& octreeJson, NodeLoader& nodeLoader, const RuntimeSetup& );
		
		/** Unsuported. Just here to fit FastParallelOctree interface. */
		FrontOctree( const string&, const int, NodeLoader&, const RuntimeSetup& );
		
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
		unique_ptr< Front > m_front;
		Node* m_root;
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
		auto now = Profiler::now( "Binary octree file reading" );
		
		PointSorter< Morton > sorter( plyFilename, maxLvl );
		PointSet< Morton > pointSet = sorter.points();
		m_dim = pointSet.m_dim;
		
		// PUT LOGIC HERE!
		
		Profiler::elapsedTime( now, "Binary octree file reading" );
		
		m_front = unique_ptr< Front >( new Front( "", m_dim, 1, loader, 8ul * 1024ul * 1024ul * 1024ul ) );
		m_front->insertRoot( *m_root );
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