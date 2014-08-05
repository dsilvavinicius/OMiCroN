#ifndef OCTREE_H
#define OCTREE_H

#include <map>
#include "MortonCode.h"
#include "OctreeNode.h"
#include "LeafNode.h"
#include "MortonComparator.h"
#include "InnerNode.h"

using namespace std;

namespace model
{	
	/** Internal map type used to actualy store the octree. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using OctreeMap = map< MortonCodePtr<MortonPrecision>, OctreeNodePtr< MortonPrecision, Float, Vec3 >,
							MortonComparator<MortonPrecision> >;
	
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
		shared_ptr< OctreeMap<MortonPrecision, Float, Vec3 > > m_hierarchy;
		
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
		m_hierarchy = make_shared< OctreeMap< MortonPrecision, Float, Vec3 > >();
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
		// Puts points inside leaf nodes.
		for (PointPtr< Vec3 > point : points)
		{
			shared_ptr<Vec3> pos = point->getPos();
			Vec3 index = ((*pos) - (*m_origin)) / (*m_size);
			MortonCodePtr< MortonPrecision > code = make_shared< MortonCode< MortonPrecision > >();
			code->build((MortonPrecision)index.x, (MortonPrecision)index.y, (MortonPrecision)index.z, m_maxLevel);
			
			typename OctreeMap< MortonPrecision, Float, Vec3 >::iterator genericLeaf = m_hierarchy->find(code);
			if (genericLeaf == m_hierarchy->end())
			{
				// Creates leaf node.
				PointsLeafNodePtr< MortonPrecision, Float, Vec3 >
						leafNode = make_shared< PointsLeafNode< MortonPrecision, Float, Vec3 > >();
						
				leafNode->setContents(vector< PointPtr< Vec3 > >());
				(*m_hierarchy)[code] = leafNode;
				leafNode->getContents()->push_back(point);
			}
			else
			{
				// Node already exists. Appends the point there.
				PointsLeafNodePtr< MortonPrecision, Float, Vec3 > leafNode =
					dynamic_pointer_cast< PointsLeafNode< MortonPrecision, Float, Vec3 > >(genericLeaf->second);
				leafNode->getContents()->push_back(point);
			}
		}
		
		// Do a bottom-up per-level construction of inner nodes.
		for (unsigned int level = m_maxLevel - 1; level > -1; --level)
		{
			// The idea behind this boundary is to get the minimum morton code that is from lower levels than
			// the current. This is the same of the morton code filled with just one 1 bit from the level immediately
			// below the current one. 
			MortonPrecision mortonLvlBoundary = 1 << 3 * (level + 1) + 1;
			
			for (typename OctreeMap< MortonPrecision, Float, Vec3 >::iterator it = m_hierarchy->begin();
				it->first->getBits() < mortonLvlBoundary && it != m_hierarchy->end(); it++)
			{
				MortonCodePtr< MortonPrecision > parentCode = it->first->traverseUp();
				
				typename OctreeMap< MortonPrecision, Float, Vec3 >::iterator parent =
					m_hierarchy->find(parentCode);
					
				if (parent == m_hierarchy->end())
				{
					PointPtr<Vec3> point = make_shared();
					// Creates inner node.
					LODInnerNodePtr<MortonPrecision, Float, Vec3> node =
						make_shared< LODInnerNode< MortonPrecision, Float, Vec3 > >();
					node->setContents();
				}
				else
				{
				}
			}
		}
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
		OctreeBase<unsigned int, Float, Vec3>::m_maxLevel = 9; // 0 to 9.
	}
	
	template<typename Float, typename Vec3>
	Octree<unsigned long, Float, Vec3>::Octree(const int& maxPointsPerNode)
	: OctreeBase< unsigned long, Float, Vec3 >::OctreeBase(maxPointsPerNode)
	{
		OctreeBase<unsigned long, Float, Vec3>::m_maxLevel = 20; // 0 to 21.
	}
	
	template<typename Float, typename Vec3>
	Octree<unsigned long long, Float, Vec3>::Octree(const int& maxPointsPerNode)
	: OctreeBase< unsigned long long, Float, Vec3 >::OctreeBase(maxPointsPerNode)
	{
		OctreeBase<unsigned long long, Float, Vec3>::m_maxLevel = 41; // 0 to 41
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
	
	/** 128-bit morton code octree (42 levels max). WARNING: The compiler may complain about unsigned long long
	 * size. That occurs because, by C++ specification, long longs can have less than 128 bits. Don't use this
	 * type in this case. */
	template <typename Float, typename Vec3>
	using DeeperOctree = Octree<unsigned long long, Float, Vec3>; 
	
	/** Octree smart pointer. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	using OctreePtr = shared_ptr< Octree < MortonPrecision, Float, Vec3 >>;
}

#endif