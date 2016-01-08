#ifndef O1OCTREE
#define O1OCTREE

#include "PointSorter.h"
#include "O1OctreeNode.h"
#include "HierarchyCreator.h"

namespace model
{
	/** Out-of-core fast parallel octree. */
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
		
		FastParallelOctree()
		: m_root( nullptr )
		{}
		
		~FastParallelOctree()
		{
			if( m_root )
			{
				delete m_root;
			}
		}
		
		/**
		 * Builds from a .ply file, generates a sorted file in the process which can be used with buildFromSortedFile()
		 * later on to skip sorting, increasing creation performance.
		 * @param maxLvl is the level from which the octree will be constructed bottom-up. Lesser values incur in
		 * less created nodes, but also less possibilities for LOD ( level of detail ). In practice, the more points the
		 * model has, the deeper the hierachy needs to be for good visualization. */
		void buildFromFile( const string& plyFilename, const int& maxLvl, ulong loadPerThread = 1024,
							size_t memoryLimit = 1024 * 1024 * 8 );
		
		/** Builds from a .ply file. The file is assumed to be sorted in z-order. Also, the octree dimensions are expected
		 * to be known. */
		void buildFromSortedFile( const string& plyFilename, const Dim& dim, ulong loadPerThread = 1024,
								  size_t memoryLimit = 1024 * 1024 * 8 );
	
		/** Gets dimensional info of this octree. */
		const Dim& dim() const { return m_dim; }
		
		Node& root() { return *m_root; }
		
		template< typename M, typename Pt >
		friend ostream& operator<<( ostream& out, const FastParallelOctree< M, Pt >& octree );
		
	private:
		void setupNodeRendering( Node node, RenderingState& renderingState );
		
		string toString( const Node& node, const Dim& nodeLvlDim ) const;
		
		/** Dimensional info of this octree. */
		Dim m_dim;
		
		/** Root node of the hierarchy. */
		Node* m_root;
		
		/** Number of threads used in octree construction and front tracking. The database thread is not part of the
		 * group. */
		static constexpr int M_N_THREADS = 8;
	};
	
	template< typename Morton, typename Point >
	void FastParallelOctree< Morton, Point >
	::buildFromFile( const string& plyFilename, const int& maxLvl, ulong loadPerThread, size_t memoryLimit )
	{
		assert( maxLvl <= Morton::maxLvl() );
		
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
		
		cout << sortedFilename << endl << endl;
		
		sorter.sort( sortedFilename );
		m_dim = sorter.comp();
		
		HierarchyCreator creator( sortedFilename, m_dim, loadPerThread, memoryLimit );
		m_root = creator.create();
	}
	
	template< typename Morton, typename Point >
	void FastParallelOctree< Morton, Point >
	::buildFromSortedFile( const string& plyFilename, const Dim& dim, ulong loadPerThread, size_t memoryLimit )
	{
		m_dim = dim;
		HierarchyCreator creator( plyFilename, m_dim, loadPerThread, memoryLimit );
		m_root = creator.create();
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
		NodeArray child = node.child();
		for( int i = 0; i < child.size(); ++i )
		{
			ss << toString( child[ i ], childLvlDim );
		}
		
		return ss.str();
	}
	
	template< typename M, typename P >
	ostream& operator<<( ostream& out, const FastParallelOctree< M, P >& octree )
	{
		using Dim = typename FastParallelOctree< M, P >::Dim;
		out << octree.toString( *octree.m_root, Dim( octree.dim(), 0 ) );
		return out;
	}
}

#endif