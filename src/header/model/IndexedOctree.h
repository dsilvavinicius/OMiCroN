#ifndef INDEXED_OCTREE_H
#define INDEXED_OCTREE_H

#include "ExtendedPoint.h"
#include "RandomSampleOctree.h"

namespace model
{
	/** An octree that uses indices to a vector of points in the hierarchy. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class IndexedOctree
	: RandomSampleOctree< MortonPrecision, Float, Vec3, Point >
	{
		using IndexVector = vector< unsigned int >;
		
	public:
		vector< Point >& getPoints(){ return points; } 
		
	protected:
		virtual void buildLeaves( const PointVector& points );
		
		virtual void buildInners();
		
		virtual OctreeNodePtr< MortonPrecision, Float, Vec3 > buildInnerNode(
			const PointVector& childrenPoints ) const;
		
		virtual void appendPoints( OctreeNodePtr< MortonPrecision, Float, Vec3 > node, PointVector& vector,
								   int& numChildren, int& numLeaves ) const;
		
	private:
		/** Point vector which will be indexed in the actual hierarchy. */
		vector< PointPtr > points;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	IndexedOctree< MortonPrecision, Float, Vec3, Point >::buildLeaves()
	{
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void IndexedOctree< MortonPrecision, Float, Vec3, Point >::appendPoints(
		OctreeNodePtr< MortonPrecision, Float, Vec3 > node, IndexVector& vector, int& numChildren, int& numLeaves ) const
	{
		
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void IndexedOctree< MortonPrecision, Float, Vec3, Point >::appendPoints(
		OctreeNodePtr< MortonPrecision, Float, Vec3 > node, PointVector& vector, int& numChildren, int& numLeaves ) const
	{}
	
	
}

#endif