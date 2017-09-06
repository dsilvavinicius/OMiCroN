#ifndef SYNC_OCTREE_H
#define SYNC_OCTREE_H

#include <jsoncpp/json/json.h>
#include "Front.h"
#include "OctreeDimensions.h"
#include "OctreeFile.h"
#include "RuntimeSetup.h"

namespace model
{
	/** A simple octree with front tracking capability. Used to replace FastParallelOctree in tests. */
	template< typename Morton >
	class FrontOctree
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
		
	private:
		unique_ptr< Front > m_front;
		OctreeFile::NodePtr m_root;
		Dim m_dim;
	};
	
	template< typename Morton >
	inline FrontOctree< Morton >::FrontOctree( const Json::Value& octreeJson, NodeLoader& nodeLoader, const RuntimeSetup& )
	{
		Vec3 octreeSize( octreeJson[ "size" ][ "x" ].asFloat(),
						 octreeJson[ "size" ][ "y" ].asFloat(),
						 octreeJson[ "size" ][ "z" ].asFloat() );
		Vec3 octreeOrigin( octreeJson[ "origin" ][ "x" ].asFloat(),
						   octreeJson[ "origin" ][ "y" ].asFloat(),
						   octreeJson[ "origin" ][ "z" ].asFloat() );
		
		m_dim = Dim( octreeOrigin, octreeSize, octreeJson[ "depth" ].asUInt() );
		
		// Debug
		{
			cout << "Dim from Json: " << m_dim << endl;
		}
		
		auto now = Profiler::now( "Binary octree file reading" );
		
		m_root = OctreeFile::read( octreeJson[ "nodes" ].asString() );
		
		Profiler::elapsedTime( now, "Binary octree file reading" );
		
		m_front = unique_ptr< Front >( new Front( octreeJson[ "database" ].asString(), m_dim, 1, nodeLoader, 8ul * 1024ul * 1024ul * 1024ul ) );
		m_front->insertRoot( *m_root );
	}
	
	template< typename Morton >
	inline OctreeStats FrontOctree< Morton >::trackFront( Renderer& renderer, const Float projThresh )
	{
		m_front->trackFront( renderer, projThresh );
	}
	
	template< typename Morton >
	inline FrontOctree< Morton >::FrontOctree( const string&, const int, NodeLoader&, const RuntimeSetup& )
	{
		throw logic_error( "FrontOctree creation from point file is unsuported." );
	}
	
	template< typename Morton >
	pair< uint, uint > FrontOctree< Morton >::nodeStatistics() const
	{
		return m_root->subtreeStatistics();
	}
}

#endif