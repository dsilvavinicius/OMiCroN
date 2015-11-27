#ifndef O1OCTREE
#define O1OCTREE

#include "PointSorter.h"
#include "O1OctreeNode.h"

namespace model
{
	/** Out-of-core fast parallel octree. */
	template< typename Morton, typename Point >
	class FastParallelOctree
	{
	public:
		using PointPtr = shared_ptr< Point >;
		using Node = O1OctreeNode< PointPtr >;
	
		/** Ctor.
		 * @param maxLvl is the level from which the octree will be constructed bottom-up. Lesser values incur in
		 * less created nodes, but also less possibilities for LOD ( level of detail ). In practice, the more points the
		 * model has, the deeper the hierachy need to be for a good visualization. */
		FastParallelOctree( const int& maxLvl );
		
		/** Builds from a .ply file, generates a sorted file in the process which can be used with buildFromSortedFile()
		 * later on to skip sorting, increasing creation performance. */
		void buildFromFile( const string& plyFileName );
		
		/** Builds from a .ply file. The file is assumed to be sorted in z-order. Use buildFromFile() to generate the
		 * sorted .ply file. */
		void buildFromSortedFile( const string& plyFileName );
	
		/** Gets the origin, which is the point contained in octree with minimun coordinates for all axis. */
		Vec3& getOrigin() const;
		
		/** Gets the size of the octree, which is the extents in each axis from origin representing the space that the
		 * octree occupies. */
		Vec3& getSize() const;
		
		/** Gets the size of the leaf nodes. */
		Vec3& getLeafSize() const;
		
		/** Gets the maximum number of points that can be inside an octree node. */
		uint getMaxPointsPerNode() const;
		
		/** Gets the maximum level that this octree can reach. */
		uint getMaxLevel() const;
		
		template< typename M, typename P >
		friend ostream& operator<<( ostream& out, const FastParallelOctree< M, P >& octree );
		
	private:
		/** Creates all nodes bottom-up. In any moment of the building, nodes can be released and loaded from database
		 * in order to correctly manage memory.*/
		void buildNodes();
		
		void setupNodeRendering( Node node, RenderingState& renderingState );
		
		/** Maximum level of this octree. */
		uint m_maxLevel;
		
		/** Structure to calculate and compare morton codes from points. */
		OctreeDimensions m_pointComp;
	};
}

#endif