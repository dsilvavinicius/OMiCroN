#ifndef OCTREE_H
#define OCTREE_H

#include <map>
#include "MortonCode.h"
#include "OctreeNode.h"

using namespace std;

namespace model
{	
	/** Octree implemented has a hash-map using morton code as here:
	 * http://www.sbgames.org/papers/sbgames09/computing/short/cts19_09.pdf */
	template <typename MortonPrecision>
	class Octree
	{
	public:
		/** Builds the octree for a given point cloud. */
		virtual void build(vector<shared_ptr<Point>> points);
		
		/** Traverses the octree, rendering all necessary points. */
		virtual void traverse();
	private:
		/** The hierarchy itself. */
		shared_ptr< map< MortonCode<MortonPrecision>, OctreeNode<MortonPrecision> > > hierarchy;
		
		/** Octree origin. Can be used to calculate node position. */
		shared_ptr<vec3> origin;
	};
	
	// TODO: Verify if it is necessary to create smart pointer types for the octree types.
	
	/** 32-bit morton code octree (10 levels max). */
	using ShallowOctree = Octree<int>;
	
	/** 64-bit morton code octree (21 levels max). */
	using DeepOctree = Octree<long>; 
	
	/** 128-bit morton code octree (42 levels max). */
	using DeeperOctree = Octree<long long>; 
	
	/** Generic octree smart pointer. */
	template <typename T>
	using OctreePtr = shared_ptr<Octree<typename MortonCode<T>::type>>;
}

#endif