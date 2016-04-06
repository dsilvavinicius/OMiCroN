#ifndef O1OCTREE
#define O1OCTREE

#include <forward_list>
#include "PointSorter.h"
#include "O1OctreeNode.h"
#include "HierarchyCreator.h"
#include "Front.h"

namespace model
{
	/** Out-of-core fast parallel octree. Provides visualization while async constructing the hierarchy bottom-up. */
	template< typename MortonCode, typename P >
	class FastParallelOctree
	{
	public:
		using Morton = MortonCode;
		using Point = P;
		using PointPtr = shared_ptr< Point >;
		using HierarchyCreator = model::HierarchyCreator< Morton, Point >;
		using Node = typename HierarchyCreator::Node;
		using NodeArray = typename HierarchyCreator::NodeArray;
		using Dim = typename HierarchyCreator::OctreeDim;
		using Front = model::Front< MortonCode, Point >;
		using Renderer = RenderingState;
		
		/**
		 * Ctor. Creates the octree from a .ply file, generating a sorted file in the process which can be used with
		 * the other constructor in order to increase creation performance.
		 * @param maxLvl is the level from which the octree will be constructed bottom-up. Lesser values incur in
		 * less created nodes, but also less possibilities for LOD ( level of detail ). In practice, the more points the
		 * model has, the deeper the hierachy needs to be for good visualization. */
		FastParallelOctree( const string& plyFilename, const int& maxLvl, ulong loadPerThread = 1024,
							ulong memoryLimit = 1024 * 1024 * 8, int nThreads = 8, bool async = false );
		
		/** Ctor. Creates the octree from a sorted .ply file. */
		FastParallelOctree( const string& plyFilename, const Dim& dim, ulong loadPerThread = 1024,
							ulong memoryLimit = 1024 * 1024 * 8, int nThreads = 8, bool async = false );
		
		~FastParallelOctree();
		
		/** Tracks the rendering front of the octree. */
		FrontOctreeStats trackFront( Renderer& renderer, const Float projThresh );
		
		/** Checks if the async creation is finished. */
		bool isCreationFinished();
		
		/** Returns only after async hierarchy creation has finished. */
		void waitCreation();
		
		/** Gets dimensional info of this octree. */
		const Dim& dim() const { return m_dim; }
		
		Node& root() { return *m_root; }
		
		#ifdef HIERARCHY_STATS
			atomic_ulong m_processedNodes;
		#endif
		
		template< typename M, typename Pt >
		friend ostream& operator<<( ostream& out, const FastParallelOctree< M, Pt >& octree );
		
	private:
		/** Builds from a .ply file. The file is assumed to be sorted in z-order. Also, the octree dimensions are expected
		 * to be known. */
		void buildFromSortedFile( const string& plyFilename, const Dim& dim, ulong loadPerThread, ulong memoryLimit,
								  int nThreads, bool async );
		
		string toString( const Node& node, const Dim& nodeLvlDim ) const;
		
		/** Octree front used for rendering. */
		Front* m_front;
		
		/** Manages the octree creation. */
		HierarchyCreator* m_hierarchyCreator;
		
		/** Future with the async creation result */
		future< Node* > m_creationFuture;
		
		/** Octree construction thread. */
		thread m_octreeThread;
		
		/** Dimensional info of this octree. */
		Dim m_dim;
		
		/** Root node of the hierarchy. */
		Node* m_root;
	};
	
	template< typename Morton, typename Point >
	FastParallelOctree< Morton, Point >
	::FastParallelOctree( const string& plyFilename, const int& maxLvl, ulong loadPerThread, ulong memoryLimit,
						  int nThreads, bool async )
	: m_hierarchyCreator( nullptr ),
	m_front( nullptr ),
	m_root( nullptr )
	{
		assert( maxLvl <= Morton::maxLvl() );
		
		omp_set_num_threads( nThreads );
		
		PointSorter< Morton, Point > sorter( plyFilename, maxLvl );
		
		int slashIdx = plyFilename.find_last_of( "/" );
		string sortedFilename = plyFilename;
		if( slashIdx != plyFilename.npos )
		{
			sortedFilename.insert( slashIdx + 1, "sorted_" );
		}
		else
		{
			sortedFilename.insert( 0, "sorted_" );
		}
		
		sorter.sort( sortedFilename );
		
		buildFromSortedFile( sortedFilename, sorter.comp(), loadPerThread, memoryLimit, nThreads, async );
	}
	
	template< typename Morton, typename Point >
	FastParallelOctree< Morton, Point >
	::FastParallelOctree( const string& plyFilename, const Dim& dim, ulong loadPerThread, ulong memoryLimit, int nThreads,
						  bool async )
	: m_hierarchyCreator( nullptr ),
	m_front( nullptr ),
	m_root( nullptr )
	{
		buildFromSortedFile( plyFilename, dim, loadPerThread, memoryLimit, nThreads, async );
	}
	
	template< typename Morton, typename Point >
	FastParallelOctree< Morton, Point >::~FastParallelOctree()
	{
		waitCreation();
			
		delete m_hierarchyCreator;
		m_hierarchyCreator = nullptr;
		
		delete m_root;
		m_root = nullptr;
		
		delete m_front;
		m_front = nullptr;
	}
	
	template< typename Morton, typename Point >
	void FastParallelOctree< Morton, Point >
	::buildFromSortedFile( const string& plyFilename, const Dim& dim, ulong loadPerThread, ulong memoryLimit,
						   int nThreads, bool async )
	{
		#ifdef HIERARCHY_STATS
			m_processedNodes = 0;
		#endif
		
		omp_set_num_threads( nThreads );
		
		m_dim = dim;
		
		string dbFilename = plyFilename.substr( 0, plyFilename.find_last_of( "." ) ).append( ".db" );
		m_front = new Front( dbFilename, m_dim, nThreads, memoryLimit );
		
		m_hierarchyCreator = new HierarchyCreator( plyFilename, m_dim, *m_front, loadPerThread, memoryLimit, nThreads );
		
		m_creationFuture = m_hierarchyCreator->createAsync();
	}
	
	template< typename Morton, typename Point >
	FrontOctreeStats FastParallelOctree< Morton, Point >::trackFront( Renderer& renderer, const Float projThresh )
	{
		return m_front->trackFront( renderer, projThresh );
	}
	
	template< typename Morton, typename Point >
	bool FastParallelOctree< Morton, Point >::isCreationFinished()
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
	
	template< typename Morton, typename Point >
	void FastParallelOctree< Morton, Point >::waitCreation()
	{
		if( m_creationFuture.valid() )
		{
			cout << "Waiting for async octree creation finish. It can take several minutes or hours depending on model size..."
						<< endl << endl;
			
			m_root = m_creationFuture.get();
			
			cout << "Hierarchy creation finished." << endl << endl;
			
			#ifdef HIERARCHY_STATS
				m_processedNodes = m_hierarchyCreator.m_processedNodes;
			#endif
		}
	}
	
	template< typename Morton, typename Point >
	string FastParallelOctree< Morton, Point >::toString( const Node& node, const Dim& nodeLvlDim ) const
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
	
	template< typename M, typename P >
	ostream& operator<<( ostream& out, const FastParallelOctree< M, P >& octree )
	{
		typename FastParallelOctree< M, P >::Node* root;
		{
			lock_guard< mutex > lock( mutex() );
			root = octree.m_root;
		}
		if( root )
		{
			using Dim = typename FastParallelOctree< M, P >::Dim;
			out << octree.toString( *root, Dim( octree.dim(), 0 ) );
			return out;
		}
	}
}

#endif