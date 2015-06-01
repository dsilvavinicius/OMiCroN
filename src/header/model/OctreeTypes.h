#ifndef OCTREE_TYPES_H
#define OCTREE_TYPES_H

#include <memory>

namespace model
{
	// Forward declaration of Octree.
	template< typename MortonCode, typename Point>
	class Octree;
	
	/** Octree smart pointer. */
	template< typename MortonCode, typename Point >
	using OctreePtr = shared_ptr< Octree < MortonCode, Point > >;
	
	/** 32-bit morton code octree (10 levels max). */
	template< typename Point >
	using ShallowOctree = Octree< unsigned int, Point >;
	
	/** 32-bit morton code octree pointer. */
	template< typename Point >
	using ShallowOctreePtr = shared_ptr< ShallowOctree< Point > >;

	/** 64-bit morton code octree (21 levels max). */
	template< typename Point >
	using MediumOctree = Octree< unsigned long, Point >;
	
	/** 64-bit morton code octree pointer. */
	template< typename Point >
	using MediumOctreePtr = shared_ptr< MediumOctree< Point > >;

	/** 128-bit morton code octree (42 levels max). WARNING: The compiler may complain about unsigned long long
	 * size. That occurs because, by C++ specification, long longs can have less than 128 bits. Don't use this
	 * type in this case. */
	template< typename Point >
	using DeepOctree = Octree< unsigned long long, Point >;
	
	/** 128-bit morton code octree pointer. */
	template< typename Point >
	using DeepOctreePtr = shared_ptr< DeepOctree< Point > >; 
}

#endif