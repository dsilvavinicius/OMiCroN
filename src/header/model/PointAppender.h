#ifndef POINT_APPENDER_H
#define POINT_APPENDER_H

#include "OctreeNode.h"
#include "Point.h"

namespace model
{
	template< typename MortonCode, typename Point >
	class RandomPointAppender
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		using PointVectorPtr = shared_ptr< PointVector >;
		
	public:
		static void appendPoints( OctreeNodePtr< PointVector > node, PointVector& vec, int& numChildren, int& numLeaves )
		{
			++numChildren;
			if( node->isLeaf() )
			{
				++numLeaves;
			}
			
			PointVector& childPoints = node->getContents();
			vec.insert( vec.end(), childPoints.begin(), childPoints.end() );
		}
		
		static void appendPoints( OctreeNodePtr< IndexVector > node, IndexVector& vec, int& numChildren, int& numLeaves )
		{
			++numChildren;
			if( node->isLeaf() )
			{
				++numLeaves;
			}
			
			IndexVector& childIndices = node->getContents();
			vec.insert( vec.end(), childIndices.begin(), childIndices.end() );
		}
	};
}

#endif