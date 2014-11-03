#ifndef OCTREE_TYPES_H
#define OCTREE_TYPES_H

#include <memory>

namespace model
{
	// Forward declaration of Octree.
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point>
	class Octree;
	
	/** Octree smart pointer. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	using OctreePtr = shared_ptr< Octree < MortonPrecision, Float, Vec3, Point > >;
	
	/** 32-bit morton code octree (10 levels max). */
	template< typename Float, typename Vec3, typename Point >
	using ShallowOctree = Octree< unsigned int, Float, Vec3, Point >;
	
	/** 32-bit morton code octree pointer. */
	template< typename Float, typename Vec3, typename Point >
	using ShallowOctreePtr = shared_ptr< ShallowOctree< Float, Vec3, Point > >;

	/** 64-bit morton code octree (21 levels max). */
	template< typename Float, typename Vec3, typename Point >
	using MediumOctree = Octree< unsigned long, Float, Vec3, Point >;
	
	/** 64-bit morton code octree pointer. */
	template< typename Float, typename Vec3, typename Point >
	using MediumOctreePtr = shared_ptr< MediumOctree< Float, Vec3, Point > >;

	/** 128-bit morton code octree (42 levels max). WARNING: The compiler may complain about unsigned long long
	 * size. That occurs because, by C++ specification, long longs can have less than 128 bits. Don't use this
	 * type in this case. */
	template< typename Float, typename Vec3, typename Point >
	using DeepOctree = Octree< unsigned long long, Float, Vec3, Point >;
	
	/** 128-bit morton code octree pointer. */
	template< typename Float, typename Vec3, typename Point >
	using DeepOctreePtr = shared_ptr< DeepOctree< Float, Vec3, Point > >; 
}

#endif