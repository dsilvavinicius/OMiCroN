#ifndef POINT_APPENDER_H
#define POINT_APPENDER_H

#include "OctreeNode.h"
#include "ExtendedPoint.h"

namespace model
{
	/** Class with utilitary methods for appending points into vectors. Used by octree code. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	class PointAppender{};
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	class PointAppender< MortonPrecision, Float, Vec3, Point< Float, Vec3 > >
	{
		using Point = model::Point< Float, Vec3 >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		
	public:
		virtual void appendPoints( OctreeNodePtr< MortonPrecision, Float, Vec3 > node, PointVector& vec, int& numChildren,
						   int& numLeaves ) const
		{
			++numChildren;
			if (node->isLeaf())
			{
				PointVectorPtr childPoints = node-> template getContents< PointVector >();
				vec.insert(vec.end(), childPoints->begin(), childPoints->end());
				++numLeaves;
			}
			else
			{
				PointPtr LODPoint = node-> template getContents< Point >();
				vec.push_back(LODPoint);
			}
		}
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	class PointAppender< MortonPrecision, Float, Vec3, ExtendedPoint< Float, Vec3 > >
	{
		using Point = model::ExtendedPoint< Float, Vec3 >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		
	public:
		virtual void appendPoints( OctreeNodePtr< MortonPrecision, Float, Vec3 > node, PointVector& vec, int& numChildren,
						   int& numLeaves ) const
		{
			++numChildren;
			if (node->isLeaf())
			{
				PointVectorPtr childPoints = node-> template getContents< PointVector >();
				vec.insert(vec.end(), childPoints->begin(), childPoints->end());
				++numLeaves;
			}
			else
			{
				PointPtr LODPoint = node-> template getContents< Point >();
				vec.push_back(LODPoint);
			}
		}
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Contents >
	class RandomPointAppender {};
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	class RandomPointAppender< MortonPrecision, Float, Vec3, Point< Float, Vec3 > >
	: public PointAppender< MortonPrecision, Float, Vec3, Point< Float, Vec3 > >
	{
		using Point = model::Point< Float, Vec3 >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		
	public:
		virtual void appendPoints( OctreeNodePtr< MortonPrecision, Float, Vec3 > node, PointVector& vec, int& numChildren,
						   int& numLeaves ) const
		{
			++numChildren;
			if( node->isLeaf() )
			{
				++numLeaves;
			}
			
			PointVectorPtr childPoints = node-> template getContents< PointVector >();
			
			vec.insert( vec.end(), childPoints->begin(), childPoints->end() );
		}
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3 >
	class RandomPointAppender< MortonPrecision, Float, Vec3, ExtendedPoint< Float, Vec3 > >
	: public PointAppender< MortonPrecision, Float, Vec3, ExtendedPoint< Float, Vec3 > >
	{
		using Point = model::ExtendedPoint< Float, Vec3 >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		
	public:
		virtual void appendPoints( OctreeNodePtr< MortonPrecision, Float, Vec3 > node, PointVector& vec, int& numChildren,
						   int& numLeaves ) const
		{
			++numChildren;
			if( node->isLeaf() )
			{
				++numLeaves;
			}
			
			PointVectorPtr childPoints = node-> template getContents< PointVector >();
			
			vec.insert( vec.end(), childPoints->begin(), childPoints->end() );
		}
	};
}

#endif