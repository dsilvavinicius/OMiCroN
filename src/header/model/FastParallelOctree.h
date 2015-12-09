#ifndef O1OCTREE
#define O1OCTREE

#include "PointSorter.h"
#include "O1OctreeNode.h"
#include "HierarchyCreator.h"

namespace model
{
	/** Out-of-core fast parallel octree. */
	template< typename MortonCode, typename Point >
	class FastParallelOctree
	{
	public:
		using Morton = MortonCode;
		using PointPtr = shared_ptr< Point >;
		using Node = O1OctreeNode< PointPtr >;
		using Dim = OctreeDimensions< Morton, Point >;
		using HierarchyCreator = model::HierarchyCreator< Morton, Point >;
		
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
		void buildFromFile( const string& plyFilename, const int& maxLvl );
		
		/** Builds from a .ply file. The file is assumed to be sorted in z-order. Use buildFromFile() to generate the
		 * sorted .ply file. */
		void buildFromSortedFile( const string& plyFilename );
	
		/** Gets dimensional info of this octree. */
		const Dim& dim() const { return m_dim; }
		
		Node& root() { return *m_root; }
		
		template< typename M, typename P >
		friend ostream& operator<<( ostream& out, const FastParallelOctree< M, P >& octree );
		
	private:
		void setupNodeRendering( Node node, RenderingState& renderingState );
		
		/** Dimensional info of this octree. */
		Dim m_dim;
		
		/** Root node of the hierarchy. */
		Node* m_root;
		
		/** Number of threads used in octree construction and front tracking. The database thread is not part of the
		 * group. */
		static constexpr int M_N_THREADS = 8;
	};
	
	template< typename Morton, typename Point >
	void FastParallelOctree< Morton, Point >::buildFromFile( const string& plyFilename, const int& maxLvl )
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
		
		buildFromSortedFile( sortedFilename );
	}
	
	template< typename Morton, typename Point >
	void FastParallelOctree< Morton, Point >::buildFromSortedFile( const string& plyFilename )
	{
		size_t memoryLimit = 1024 * 1024 * 8;
		HierarchyCreator creator( plyFilename, m_dim, 1024, memoryLimit );
		m_root = creator.create();
	}
}

#endif