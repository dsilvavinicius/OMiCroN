#ifndef OCTREE_H
#define OCTREE_H

#include <map>
#include "MortonCode.h"
#include "OctreeNode.h"

using namespace std;

namespace model
{	
	/** Octree implemented has a hash-map using morton code as here:
	 * http://www.sbgames.org/papers/sbgames09/computing/short/cts19_09.pdf.
	 * MortonPrecision is specified in the types bellow this class and VecType
	 * should be one of glm's vec3 types. */
	template <typename MortonPrecision, typename NumType>
	class Octree
	{
	public:
		Octree();
		
		/** Builds the octree for a given point cloud. */
		virtual void build(vector<shared_ptr<Point>> points);
		
		/** Traverses the octree, rendering all necessary points. */
		virtual void traverse();
	private:
		/** Calculates octree's boundaries. */
		buildBoundaries(vector<shared_ptr<Point>> points);
		
		/** The hierarchy itself. */
		shared_ptr< map< MortonCode<MortonPrecision>, OctreeNodePtr<MortonPrecision> > > hierarchy;
		
		/** Octree origin. Can be used to calculate node positions. */
		shared_ptr<tvec3<NumType>> m_origin;
		
		/** Spatial size of the octree. */
		shared_ptr<NumType> m_size;
	};
	
	//TODO: Specify also all possibilities for NumType (OMG!).
	
	/** 32-bit morton code octree (10 levels max). */
	template <typename NumType>
	using ShallowOctree = Octree<int, NumType>;
	
	/** 64-bit morton code octree (21 levels max). */
	template <typename NumType>
	using DeepOctree = Octree<long, NumType>; 
	
	/** 128-bit morton code octree (42 levels max). */
	template <typename NumType>
	using DeeperOctree = Octree<long long, NumType>; 
	
	/** Octree smart pointer. */
	template <typename T>
	using OctreePtr = shared_ptr<Octree<typename MortonCode<T>::type>>;
}

#endif