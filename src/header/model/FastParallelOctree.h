#ifndef O1OCTREE
#define O1OCTREE

#include <forward_list>
#include "PointSorter.h"
#include "O1OctreeNode.h"
#include "HierarchyCreator.h"
#include "Front.h"
#include "global_malloc.h"
#include "RuntimeSetup.h"

namespace model
{
	/** Fast parallel octree. Provides visualization while async constructing the hierarchy bottom-up. */
	template< typename MortonCode >
	class FastParallelOctree
	{
	public:
		using Morton = MortonCode;
		using HierarchyCreator = model::HierarchyCreator< Morton >;
		using Node = typename HierarchyCreator::Node;
		using NodeArray = typename HierarchyCreator::NodeArray;
		using Dim = typename HierarchyCreator::OctreeDim;
		using Front = model::Front< MortonCode >;
		using NodeLoader = typename Front::NodeLoader;
		using Renderer = SplatRenderer;
		
		/**
		 * Ctor. Creates the octree from a .ply file, generating a sorted file in the process which can be used with
		 * the other constructor in order to increase creation performance.
		 * @param maxLvl is the level from which the octree will be constructed bottom-up. Smaller values incur in
		 * less created nodes, but also less possibilities for LOD ( level of detail ). In practice, the more points the
		 * model has, the deeper the hierachy needs to be for good visualization. */
		FastParallelOctree( const string& plyFilename, const int maxLvl, NodeLoader& nodeLoader,
							const RuntimeSetup& runtime = RuntimeSetup() );
		
		/** Ctor. Creates the octree from a octree file. */
		FastParallelOctree( const Json::Value& octreeJson, NodeLoader& nodeLoader,
							const RuntimeSetup& runtime = RuntimeSetup() );
		
		~FastParallelOctree();
		
		/** Tracks the rendering front of the octree. */
		OctreeStats trackFront( Renderer& renderer, const Float projThresh );
		
		/** Checks if the async creation is finished. */
		bool isCreationFinished();
		
		/** Returns only after async hierarchy creation has finished. */
		void waitCreation();
		
		/** Gets dimensional info of this octree. */
		const Dim& dim() const { return m_dim; }
		
		Node& root() { return *m_root; }
		
		/** Get the time needed to create the hierarchy in ms. If the hierarchy is not created yet, it returns 0. */
		int hierarchyCreationDuration() { return m_hierarchyCreationDuration; }
		
		/** Traverses the hierarchy to calculate its number of nodes and node contents. Hierarchy creation must be finished
		 * beforehand. 
		 * @return If the hierarchy is already finished, a pair with first value equals to the number of nodes in the
		 * hierarchy and second values equals to the the number of contents in all nodes. Returns 0 otherwise. */
		pair< uint, uint > nodeStatistics() const;
		
		uint substitutedPlaceholders() const;
		
		template< typename M >
		friend ostream& operator<<( ostream& out, const FastParallelOctree< M >& octree );
		
	private:
		/** Builds from a sorted point set in memory. */
		void buildFromSortedPoints( const SortedPointSet< Morton >& points, NodeLoader& loader, const RuntimeSetup& runtime );
		
		/** Builds from a octree file json. */
		void buildFromSortedFile( const Json::Value& octreeJson, NodeLoader& nodeLoader, const RuntimeSetup& runtime );
		
		string toString( const Node& node, const Dim& nodeLvlDim ) const;
		
		/** Octree front used for rendering. */
		Front* m_front;
		
		/** Manages the octree creation. */
		HierarchyCreator* m_hierarchyCreator;
		
		/** Future with the async creation result */
		future< pair< Node*, int > > m_creationFuture;
		
		/** Octree construction thread. */
		thread m_octreeThread;
		
		/** Dimensional info of this octree. */
		Dim m_dim;
		
		/** Root node of the hierarchy. */
		Node* m_root;
		
		/** Duration of the hierarchy creation procedure. */
		int m_hierarchyCreationDuration;
	};
	
	template< typename Morton >
	FastParallelOctree< Morton >
	::FastParallelOctree( const string& plyFilename, const int maxLvl, NodeLoader& loader, const RuntimeSetup& runtime )
	: m_hierarchyCreator( nullptr ),
	m_front( nullptr ),
	m_root( nullptr ),
	m_hierarchyCreationDuration( 0 )
	{
		assert( maxLvl <= Morton::maxLvl() );
		
		omp_set_num_threads( 8 );
		
		SortedPointSet< Morton > sortedPoints;
		{
			PointSorter< Morton > sorter( plyFilename, maxLvl );
			sortedPoints = sorter.sort();
		}
		
		buildFromSortedPoints( sortedPoints, loader, runtime );
	}
	
	template< typename Morton >
	FastParallelOctree< Morton >
	::FastParallelOctree( const Json::Value& octreeJson, NodeLoader& loader, const RuntimeSetup& runtime )
	: m_hierarchyCreator( nullptr ),
	m_front( nullptr ),
	m_root( nullptr )
	{
		buildFromSortedFile( octreeJson, loader, runtime );
	}
	
	template< typename Morton >
	FastParallelOctree< Morton >::~FastParallelOctree()
	{
		waitCreation();
			
		delete m_hierarchyCreator;
		m_hierarchyCreator = nullptr;
		
		delete m_root;
		m_root = nullptr;
		
		delete m_front;
		m_front = nullptr;
	}
	
	template< typename Morton >
	void FastParallelOctree< Morton >
	::buildFromSortedPoints( const SortedPointSet< Morton >& points, NodeLoader& loader, const RuntimeSetup& runtime )
	{
		omp_set_num_threads( 8 );
		
		m_dim = points.m_dim;
		
		// Debug
		{
			cout << "Dim from sorted set: " << points.m_dim << endl;
		}
		
		m_front = new Front( "", m_dim, runtime.m_nThreads, loader, runtime.m_memoryQuota );
		
		m_hierarchyCreator = new HierarchyCreator( 	points,
													#ifdef HIERARCHY_CREATION_RENDERING
														*m_front,
													#endif
													runtime.m_loadPerThread, runtime.m_memoryQuota, runtime.m_nThreads );
		
		m_creationFuture = m_hierarchyCreator->createAsync();
	}
	
	template< typename Morton >
	void FastParallelOctree< Morton >
	::buildFromSortedFile( const Json::Value& octreeJson, NodeLoader& loader, const RuntimeSetup& runtime )
	{
		cout << "Octree json: " << endl << octreeJson << endl;
			
		omp_set_num_threads( 8 );
		
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
		
		m_front = new Front( octreeJson[ "database" ].asString(), m_dim, runtime.m_nThreads, loader,
							 runtime.m_memoryQuota );
		
		m_hierarchyCreator = new HierarchyCreator( octreeJson[ "points" ].asString(), m_dim,
													#ifdef HIERARCHY_CREATION_RENDERING
														*m_front,
													#endif
												   runtime.m_loadPerThread, runtime.m_memoryQuota, runtime.m_nThreads );
		
		m_creationFuture = m_hierarchyCreator->createAsync();
	}
	
	template< typename Morton >
	OctreeStats FastParallelOctree< Morton >
	::trackFront( Renderer& renderer, const Float projThresh )
	{
		return m_front->trackFront( renderer, projThresh );
	}
	
	template< typename Morton >
	bool FastParallelOctree< Morton >::isCreationFinished()
	{
		if( m_creationFuture.wait_for( chrono::seconds( 0 ) ) == future_status::ready )
		{
			waitCreation();
			return true;
		}
		else
		{
			return false;
		}
	}
	
	template< typename Morton >
	void FastParallelOctree< Morton >::waitCreation()
	{
		if( m_creationFuture.valid() )
		{
			cout << "Waiting for async octree creation finish. It can take several minutes or hours depending on model size..."
						<< endl << endl;
			
			pair< Node*, int > creationResult = m_creationFuture.get();
			
			m_root = creationResult.first;			
			m_hierarchyCreationDuration = creationResult.second;
			
			cout << "Hierarchy creation finished. Duration: " << m_hierarchyCreationDuration << endl << endl;
		}
	}
	
	template< typename Morton >
	pair< uint, uint > FastParallelOctree< Morton >::nodeStatistics() const
	{
		return m_root->subtreeStatistics();
	}
	
	template< typename Morton >
	uint FastParallelOctree< Morton >::substitutedPlaceholders() const
	{
		return m_front->substitutedPlaceholders();
	}
	
	template< typename Morton >
	string FastParallelOctree< Morton >::toString( const Node& node, const Dim& nodeLvlDim ) const
	{
		stringstream ss;
		for( int i = 0; i < int( nodeLvlDim.m_nodeLvl ) - 1; ++i )
		{
			ss << "	";
		}
		
		ss << nodeLvlDim.calcMorton( node ).getPathToRoot( true );
		
		Dim childLvlDim( nodeLvlDim, nodeLvlDim.m_nodeLvl + 1 );
		const NodeArray& child = node.child();
		for( int i = 0; i < child.size(); ++i )
		{
			ss << toString( child[ i ], childLvlDim );
		}
		
		return ss.str();
	}
	
	template< typename M >
	ostream& operator<<( ostream& out, const FastParallelOctree< M >& octree )
	{
		typename FastParallelOctree< M >::Node* root;
		{
			lock_guard< mutex > lock( mutex() );
			root = octree.m_root;
		}
		if( root )
		{
			using Dim = typename FastParallelOctree< M >::Dim;
			out << octree.toString( *root, Dim( octree.dim(), 0 ) );
			return out;
		}
	}
}

#endif