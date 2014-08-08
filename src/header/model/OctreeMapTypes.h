#ifndef OCTREE_MAP_TYPES_H
#define OCTREE_MAP_TYPES_H

#include <memory>
#include <map>

namespace model
{	
	/** Internal map type used to actualy store the octree. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using OctreeMap = map< MortonCodePtr<MortonPrecision>, OctreeNodePtr< MortonPrecision, Float, Vec3 >,
							MortonComparator<MortonPrecision> >;

	/** Smart pointer for the internal octree map. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using OctreeMapPtr = shared_ptr< OctreeMap< MortonPrecision, Float, Vec3 > >;

	/** 32-bit morton code octree map. */
	template <typename Float, typename Vec3>
	using ShallowOctreeMap = OctreeMap<unsigned int, Float, Vec3>;

	/** 32-bit morton code octree map pointer. */
	template <typename Float, typename Vec3>
	using ShallowOctreeMapPtr = shared_ptr< ShallowOctreeMap< Float, Vec3 > >;

	/** 64-bit morton code octree map. */
	template <typename Float, typename Vec3>
	using MediumOctreeMap = OctreeMap<unsigned long, Float, Vec3>; 

	/** 64-bit morton code octree map pointer. */
	template <typename Float, typename Vec3>
	using MediumOctreeMapPtr = shared_ptr< MediumOctreeMap< Float, Vec3 > >;

	/** 128-bit morton code octree map. */
	template <typename Float, typename Vec3>
	using DeepOctreeMap = OctreeMap<unsigned long long, Float, Vec3>; 

	/** 128-bit morton code octree map pointer. */
	template <typename Float, typename Vec3>
	using DeepOctreeMapPtr =  shared_ptr< DeepOctreeMap< Float, Vec3 > >;
}

#endif