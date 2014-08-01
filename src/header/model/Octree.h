#ifndef OCTREE_H
#define OCTREE_H

#include <map>
#include "MortonCode.h"
#include "OctreeNode.h"
#include "LeafNode.h"

using namespace std;

namespace model
{	
	/** Octree implemented as a hash-map using morton code as explained here:
	 * http://www.sbgames.org/papers/sbgames09/computing/short/cts19_09.pdf.
	 * 
	 * MortonPrecision is the precision of the morton code for nodes.
	 * Float is the glm type specifying the floating point type / precision.
	 * Vec3 is the glm type specifying the type / precision of the vector with 3 coordinates. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	class OctreeBase
	{
	public:
		/** Constructs this octree, giving the desired max number of nodes per node. */
		OctreeBase(const int& maxPointsPerNode);
		
		/** Builds the octree for a given point cloud. */
		virtual void build(vector< PointPtr< Vec3 > > points);
		
		/** Traverses the octree, rendering all necessary points. */
		virtual void traverse();
		
	protected:
		/** Calculates octree's boundaries. */
		void buildBoundaries(vector< PointPtr< Vec3 >> points);
		
		/** Creates all nodes bottom-up. */
		void buildNodes(vector< PointPtr< Vec3 >> points);
		
		/** The hierarchy itself. */
		shared_ptr< map< MortonCodePtr<MortonPrecision>, OctreeNodePtr< MortonPrecision, Float, Vec3 > > > m_hierarchy;
		
		/** Octree origin. Can be used to calculate node positions. */
		shared_ptr<Vec3> m_origin;
		
		/** Spatial size of the octree. */
		shared_ptr<Vec3> m_size;
		
		/** Maximum number of points per node. */
		int m_maxPointsPerNode;
		
		/** Maximum level of this octree. */
		unsigned int m_maxLevel;
	};
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	OctreeBase<MortonPrecision, Float, Vec3>::OctreeBase(const int& maxPointsPerNode)
	{
		m_size = make_shared<Vec3>();
		m_origin = make_shared<Vec3>();
		m_maxPointsPerNode = maxPointsPerNode;
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::build(vector< PointPtr<Vec3> > points)
	{
		buildBoundaries(points);
		buildNodes(points);
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::buildBoundaries(
		vector< PointPtr<Vec3> > points)
	{
		Float negInf = numeric_limits<Float>::min();
		Float posInf = numeric_limits<Float>::max();
		Vec3 minCoords(posInf, posInf, posInf);
		Vec3 maxCoords(negInf, negInf, negInf);
		
		for (PointPtr<Vec3> point : points)
		{
			shared_ptr<Vec3> pos = point->getPos();
			
			for (int i = 0; i < 3; ++i)
			{
				minCoords[i] = glm::min(minCoords[i], (*pos)[i]);
				maxCoords[i] = glm::max(maxCoords[i], (*pos)[i]);
			}
		}
		
		*m_origin = minCoords;
		*m_size = maxCoords - minCoords;
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase< MortonPrecision, Float, Vec3 >::buildNodes(
		vector< PointPtr< Vec3 > > points)
	{
		// Creates leaf nodes.
		for (unsigned int x = 0; x < m_maxLevel; ++x)
		{
			for (unsigned int y = 0; y < m_maxLevel; ++y)
			{
				for (unsigned int z = 0; z < m_maxLevel; ++z)
				{
					PointsLeafNodePtr< MortonPrecision, Float, Vec3 >
						leafNode = make_shared< PointsLeafNode< MortonPrecision, Float, Vec3 > >();
						
					leafNode->setContents(make_shared< vector< PointPtr< Vec3 > > >());
					OctreeNodePtr< MortonPrecision, Float, Vec3 > octreeNode = leafNode;
					
					MortonCodePtr< MortonPrecision > code = make_shared< MortonCode< MortonPrecision > >();
					code->build(x, y, z, m_maxLevel);
					
					m_hierarchy[code] = octreeNode;
				}
			}
		}
		
		// Puts points inside leaf nodes
		for (PointPtr< Vec3 > point : points)
		{
			
		}
		
		// Creates inner nodes.
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::traverse()
	{
		
	}
	
	//=====================================================================
	// Specializations.
	//=====================================================================
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	class Octree {};
	
	template<typename Float, typename Vec3>
	class Octree <unsigned int, Float, Vec3> : public OctreeBase<unsigned int, Float, Vec3>
	{
	public:
		Octree(const int& maxPointsPerNode);
	};
	
	template<typename Float, typename Vec3>
	class Octree <unsigned long, Float, Vec3> : public OctreeBase<unsigned long, Float, Vec3>
	{
	public:
		Octree(const int& maxPointsPerNode);
	};
	
	template<typename Float, typename Vec3>
	class Octree <unsigned long long, Float, Vec3> : public OctreeBase<unsigned long long, Float, Vec3>
	{
	public:
		Octree(const int& maxPointsPerNode);
	};
	
	template<typename Float, typename Vec3>
	Octree< unsigned int, Float, Vec3 >::Octree(const int& maxPointsPerNode)
	: OctreeBase< unsigned int, Float, Vec3 >::OctreeBase(maxPointsPerNode)
	{
		OctreeBase<unsigned int, Float, Vec3>::m_maxLevel = 10;
	}
	
	template<typename Float, typename Vec3>
	Octree<unsigned long, Float, Vec3>::Octree(const int& maxPointsPerNode)
	: OctreeBase< unsigned long, Float, Vec3 >::OctreeBase(maxPointsPerNode)
	{
		OctreeBase<unsigned long, Float, Vec3>::m_maxLevel = 21;
	}
	
	template<typename Float, typename Vec3>
	Octree<unsigned long long, Float, Vec3>::Octree(const int& maxPointsPerNode)
	: OctreeBase< unsigned long long, Float, Vec3 >::OctreeBase(maxPointsPerNode)
	{
		OctreeBase<unsigned long long, Float, Vec3>::m_maxLevel = 42;
	}
	
	//=====================================================================
	// Type sugars.
	//=====================================================================
	
	/** 32-bit morton code octree (10 levels max). */
	template <typename Float, typename Vec3>
	using ShallowOctree = Octree<unsigned int, Float, Vec3>;
	
	/** 64-bit morton code octree (21 levels max). */
	template <typename Float, typename Vec3>
	using DeepOctree = Octree<unsigned long, Float, Vec3>; 
	
	/** 128-bit morton code octree (42 levels max). WARNING: The compiler may complain that unsigned long long
	 * cannot contain constants used to calculate 128-bit morton codes. That occurs because, by C++ specification, long
	 * longs can have less than 128 bits. Don't use this type in this case. */
	template <typename Float, typename Vec3>
	using DeeperOctree = Octree<unsigned long long, Float, Vec3>; 
	
	/** Octree smart pointer. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using OctreePtr = shared_ptr< Octree < MortonPrecision, Float, Vec3 >>;
}

#endif