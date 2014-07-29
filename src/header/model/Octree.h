#ifndef OCTREE_H
#define OCTREE_H

#include <map>
#include "MortonCode.h"
#include "OctreeNode.h"

using namespace std;

namespace model
{	
	/** Octree implemented as a hash-map using morton code as explained here:
	 * http://www.sbgames.org/papers/sbgames09/computing/short/cts19_09.pdf.
	 * 
	 * MortonPrecision is the precision of the morton code for nodes.
	 * Float is the glm type specifying the floating point precision.
	 * Vec3 is the glm type specifying the precision of the vector with 3 coordinates. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	class Octree
	{
	public:
		Octree();
		
		/** Builds the octree for a given point cloud. */
		virtual void build(vector< PointPtr< Vec3 > > points);
		
		/** Traverses the octree, rendering all necessary points. */
		virtual void traverse();
	private:
		/** Calculates octree's boundaries. */
		void buildBoundaries(vector< PointPtr< Vec3 >> points);
		
		/** Creates all nodes bottom-up. */
		void buildNodes(vector< PointPtr< Vec3 >> points);
		
		/** The hierarchy itself. */
		shared_ptr< map< MortonCode<MortonPrecision>,
			OctreeNodePtr< MortonPrecision, Float, Vec3 > > > hierarchy;
		
		/** Octree origin. Can be used to calculate node positions. */
		shared_ptr<Vec3> m_origin;
		
		/** Spatial size of the octree. */
		shared_ptr<Vec3> m_size;
	};
	
	/** 32-bit morton code octree (10 levels max). */
	template <typename Float, typename Vec3>
	using ShallowOctree = Octree<int, Float, Vec3>;
	
	/** 64-bit morton code octree (21 levels max). */
	template <typename Float, typename Vec3>
	using DeepOctree = Octree<long, Float, Vec3>; 
	
	/** 128-bit morton code octree (42 levels max). */
	template <typename Float, typename Vec3>
	using DeeperOctree = Octree<long long, Float, Vec3>; 
	
	/** Octree smart pointer. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using OctreePtr = shared_ptr< Octree < MortonPrecision, Float, Vec3 >>;
}

#endif