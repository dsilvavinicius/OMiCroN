#ifndef OCTREE_H
#define OCTREE_H

#include <map>
#include <glm/ext.hpp>
#include <Qt3D/QGLPainter>
#include "MortonCode.h"
#include "OctreeNode.h"
#include "LeafNode.h"
#include "MortonComparator.h"
#include "InnerNode.h"
#include "OctreeTypes.h"
#include "OctreeMapTypes.h"
#include "Stream.h"

using namespace std;

namespace model
{	
	/** Base Octree implemented as a hash-map using morton code as explained here:
	 * http://www.sbgames.org/papers/sbgames09/computing/short/cts19_09.pdf. All parts of construction and traversal are free
	 * for reimplementation of derived classes.
	 * 
	 * MortonPrecision is the precision of the morton code for nodes.
	 * Float is the glm type specifying the floating point type / precision.
	 * Vec3 is the glm type specifying the type / precision of the vector with 3 coordinates. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	class OctreeBase
	{
	public:
		/** Initialize data for building the octree, giving the desired max number of nodes per node
		 * and the Camera used to cull points in traversal. */
		OctreeBase(const int& maxPointsPerNode);
		
		/** Builds the octree for a given point cloud. */
		virtual void build(const PointVector< Float, Vec3 >& points);
		
		/** Traverses the octree, rendering all necessary points. */
		virtual void traverse(QGLPainter *painter);
		
		virtual OctreeMapPtr< MortonPrecision, Float, Vec3 > getHierarchy() const;
		
		/** Gets the origin, which is the point contained in octree with minimun coordinates for all axis. */
		virtual shared_ptr< Vec3 > getOrigin() const;
		
		/** Gets the size of the octree, which is the extents in each axis from origin representing the space that the
		 * octree occupies. */
		virtual shared_ptr< Vec3 > getSize() const;
		
		/** Gets the size of the leaf nodes. */
		virtual shared_ptr< Vec3 > getLeafSize() const;
		
		/** Gets the maximum number of points that can be inside an octree node. */
		virtual unsigned int getMaxPointsPerNode() const;
		
		/** Gets the maximum level that this octree can reach. */
		virtual unsigned int getMaxLevel() const;
		
		template <typename M, typename F, typename V>
		friend ostream& operator<<(ostream& out, const OctreeBase< M, F, V >& octree);
		
	protected:
		/** Calculates octree's boundaries. */
		virtual void buildBoundaries(const PointVector< Float, Vec3 >& points);
		
		/** Creates all nodes bottom-up. */
		virtual void buildNodes(const PointVector< Float, Vec3 >& points);
		
		/** Creates all leaf nodes and put points inside them. */
		virtual void buildLeaves(const PointVector< Float, Vec3 >& points);
		
		/** Creates all inner nodes, with LOD. Bottom-up. If a node has only leaf chilren and the accumulated number of
		 * children points is less than a threshold, the children is merged into parent. */
		virtual void buildInners();
		
		/** Computes the boundaries of the node indicated by the given morton code. */
		pair< Vec3, Vec3 > getBoundaries(MortonCodePtr< MortonPrecision >) const;
		
		/** Utility: appends the points of a child node into a vector, incrementing the number of parent's children and
		 * leaves. */
		static void appendPoints(OctreeNodePtr< MortonPrecision, Float, Vec3 > node,
								 PointVector< Float, Vec3 >& vector, int& numChildren, int& numLeaves);
		
		/** Traversal recursion. */
		virtual void traverse(MortonCodePtr< MortonPrecision > nodeCode, QGLPainter* painter,
							  vector< Vec3 >& pointsToDraw, vector< Vec3 >& colorsToDraw);
		
		/** The hierarchy itself. */
		OctreeMapPtr< MortonPrecision, Float, Vec3 > m_hierarchy;
		
		/** Octree origin, which is the point contained in octree with minimum coordinates for all axis. */
		shared_ptr<Vec3> m_origin;
		
		/** Spatial size of the octree. */
		shared_ptr<Vec3> m_size;
		
		/** Spatial size of the leaf nodes. */
		shared_ptr<Vec3> m_leafSize;
		
		/** Maximum number of points per node. */
		unsigned int m_maxPointsPerNode;
		
		/** Maximum level of this octree. */
		unsigned int m_maxLevel;
	};
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	OctreeBase<MortonPrecision, Float, Vec3>::OctreeBase(const int& maxPointsPerNode)
	{
		m_size = make_shared<Vec3>();
		m_leafSize = make_shared<Vec3>();
		m_origin = make_shared<Vec3>();
		m_maxPointsPerNode = maxPointsPerNode;
		m_hierarchy = make_shared< OctreeMap< MortonPrecision, Float, Vec3 > >();
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::build(const PointVector< Float, Vec3 >& points)
	{
		buildBoundaries(points);
		buildNodes(points);
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::buildBoundaries(
		const PointVector< Float, Vec3 >& points)
	{
		Float negInf = -numeric_limits<Float>::max();
		Float posInf = numeric_limits<Float>::max();
		Vec3 minCoords(posInf, posInf, posInf);
		Vec3 maxCoords(negInf, negInf, negInf);
		
		for (PointPtr< Float, Vec3 > point : points)
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
		*m_leafSize = *m_size * ((Float)1 / ((MortonPrecision)1 << m_maxLevel));
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase< MortonPrecision, Float, Vec3 >::buildNodes(
		const PointVector< Float, Vec3 >& points)
	{	
		buildLeaves(points);
		buildInners();
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::buildLeaves(const PointVector< Float, Vec3 >& points)
	{
		for (PointPtr< Float, Vec3 > point : points)
		{
			shared_ptr<Vec3> pos = point->getPos();
			Vec3 index = ((*pos) - (*m_origin)) / (*m_leafSize);
			MortonCodePtr< MortonPrecision > code = make_shared< MortonCode< MortonPrecision > >();
			code->build((MortonPrecision)index.x, (MortonPrecision)index.y, (MortonPrecision)index.z, m_maxLevel);
			
			typename OctreeMap< MortonPrecision, Float, Vec3 >::iterator genericLeafIt = m_hierarchy->find(code);
			if (genericLeafIt == m_hierarchy->end())
			{
				// Creates leaf node.
				PointsLeafNodePtr< MortonPrecision, Float, Vec3 >
						leafNode = make_shared< PointsLeafNode< MortonPrecision, Float, Vec3 > >();
						
				leafNode->setContents(PointVector< Float, Vec3 >());
				(*m_hierarchy)[code] = leafNode;
				leafNode->getContents()->push_back(point);
			}
			else
			{
				// Node already exists. Appends the point there.
				OctreeNodePtr< MortonPrecision, Float, Vec3 > leafNode = genericLeafIt->second;
				shared_ptr< PointVector< Float, Vec3 > > nodePoints =
					leafNode-> template getContents< PointVector< Float, Vec3 > >();
				nodePoints->push_back(point);
			}
		}
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::buildInners()
	{
		// Do a bottom-up per-level construction of inner nodes.
		for (int level = m_maxLevel - 1; level > -1; --level)
		{
			// The idea behind this boundary is to get the minimum morton code that is from lower levels than
			// the current. This is the same of the morton code filled with just one 1 bit from the level immediately
			// below the current one. 
			MortonPrecision mortonLvlBoundary = 1 << (3 * (level + 1) + 1);
			
			typename OctreeMap< MortonPrecision, Float, Vec3 >::iterator firstChildIt = m_hierarchy->begin(); 
			
			while (true) // Loops per siblings in a level.
			{
				// Stop conditions.
				if (firstChildIt == m_hierarchy->end()) { break; }
				if (firstChildIt->first->getBits() >= mortonLvlBoundary) { break; }
				
				MortonCodePtr< MortonPrecision > parentCode = firstChildIt->first->traverseUp();
				
				// These counters are used to check if the accumulated number of child node points is less than a threshold.
				// In this case, the children are deleted and their points are merged into the parent.
				int numChildren = 0;
				int numLeaves = 0;
				
				// Points to be accumulated for LOD or to be merged into the parent.
				auto childrenPoints = PointVector< Float, Vec3 >(); 
				
				// Appends first child node.
				OctreeNodePtr< MortonPrecision, Float, Vec3 > firstChild = firstChildIt->second;
				appendPoints(firstChild, childrenPoints, numChildren, numLeaves);
				
				// Adds points of remaining child nodes.
				typename OctreeMap< MortonPrecision, Float, Vec3 >::iterator currentChildIt = firstChildIt;
				while ((++currentChildIt) != m_hierarchy->end())
				{
					if (*currentChildIt->first->traverseUp() != *parentCode) { break; }
					
					OctreeNodePtr< MortonPrecision, Float, Vec3 > currentChild = currentChildIt->second;
					appendPoints(currentChild, childrenPoints, numChildren, numLeaves);
				}
				
				// TODO: Verify what to do with the cases of chains in hierarchy where the deeper chain node
				// is not leaf.
				if (numChildren == numLeaves && childrenPoints.size() <= m_maxPointsPerNode)
				{
					// All children are leaves, but they have less points than the threshold and must be merged.
					auto tempIt = firstChildIt;
					advance(firstChildIt, numChildren);
					
					m_hierarchy->erase(tempIt, currentChildIt);
					
					// Creates leaf to replace children.
					PointsLeafNodePtr< MortonPrecision, Float, Vec3 > mergedNode =
						make_shared< PointsLeafNode< MortonPrecision, Float, Vec3 > >();
					mergedNode->setContents(childrenPoints);
					
					(*m_hierarchy)[parentCode] = mergedNode;
				}
				/* 
				// The condition below is to build a sparse representation of the octree.
				else if (numChildren == 1 && numLeaves == 0)
				{
					// Inner node child, which forms a chain with the parent and represents the same info.
					// The parent should just absorb the child node.
					auto tempIt = firstChildIt;
					advance(firstChildIt, numChildren);
					
					m_hierarchy->erase(tempIt, currentChildIt);
					
					// Creates leaf to replace children.
					LODInnerNodePtr< MortonPrecision, Float, Vec3 > newNode =
						make_shared< LODInnerNode< MortonPrecision, Float, Vec3 > >();
					newNode->setContents(*childrenPoints[0]);
					
					(*m_hierarchy)[parentCode] = newNode;
				}*/
				else
				{
					// No merge or absorption is needed. Just does LOD.
					advance(firstChildIt, numChildren);
					
					// Accumulate points for LOD.
					Point< Float, Vec3 > accumulated(Vec3(0, 0, 0), Vec3(0, 0, 0));
					for (PointPtr< Float, Vec3 > point : childrenPoints)
					{
						accumulated = accumulated + (*point);
					}
					accumulated = accumulated.multiply(1 / (Float)childrenPoints.size());
					
					// Creates leaf to replace children.
					LODInnerNodePtr< MortonPrecision, Float, Vec3 > LODNode =
						make_shared< LODInnerNode< MortonPrecision, Float, Vec3 > >();
					LODNode->setContents(accumulated);
					
					(*m_hierarchy)[parentCode] = LODNode;
				}
			}
		}
	}

	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::appendPoints(OctreeNodePtr< MortonPrecision, Float, Vec3 > node,
							 PointVector< Float, Vec3 >& vec, int& numChildren, int& numLeaves)
	{
		++numChildren;
		if (node->isLeaf())
		{
			PointVectorPtr< Float, Vec3 > childPoints = node-> template getContents< PointVector< Float, Vec3 > >();
			vec.insert(vec.end(), childPoints->begin(), childPoints->end());
			++numLeaves;
		}
		else
		{
			PointPtr< Float, Vec3 > LODPoint = node-> template getContents< Point< Float, Vec3 > >();
			vec.push_back(LODPoint);
		}
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::traverse(QGLPainter* painter)
	{
		painter->clearAttributes();
		painter->setStandardEffect(QGL::FlatPerVertexColor);
		
		MortonCodePtr< MortonPrecision > rootCode = make_shared< MortonCode< MortonPrecision > >();
		rootCode->build(0x1);
		vector<Vec3> pointsToDraw;
		vector<Vec3> colorsToDraw;
		
		traverse(rootCode, painter, pointsToDraw, colorsToDraw);
		
		// TODO: Find a way to specify the precision properly here,
		QGLAttributeValue pointValues(3, GL_FLOAT, 0, &pointsToDraw[0]);
		QGLAttributeValue colorValues(3, GL_FLOAT, 0, &colorsToDraw[0]);
		painter->setVertexAttribute(QGL::Position, pointValues);
		painter->setVertexAttribute(QGL::Color, colorValues);
		
		painter->draw(QGL::Points, pointsToDraw.size());
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	void OctreeBase<MortonPrecision, Float, Vec3>::traverse(MortonCodePtr< MortonPrecision > nodeCode,
															QGLPainter* painter, vector< Vec3 >& pointsToDraw,
															vector< Vec3 >& colorsToDraw)
	{
		auto nodeIt = m_hierarchy->find(nodeCode);
		if (nodeIt != m_hierarchy->end())
		{
			OctreeNodePtr< MortonPrecision, Float, Vec3 > node = nodeIt->second;
			pair< Vec3, Vec3 > boxVerts = getBoundaries(nodeCode);
			Vec3 v0 = boxVerts.first;
			Vec3 v1 = boxVerts.second;
			QBox3D box(QVector3D(v0.x, v0.y, v0.z), QVector3D(v1.x, v1.y, v1.z));
			
			if (!painter->isCullable(box))
			{
				QMatrix4x4 modelViewProjection = painter->combinedMatrix();
				QVector4D projMin = modelViewProjection.map(QVector4D(box.minimum(), 1));
				projMin = projMin / projMin.w();
				QVector4D projMax = modelViewProjection.map(QVector4D(box.maximum(), 1));
				projMax = projMax / projMax.w();
				
				if ((projMax - projMin).lengthSquared() < 1) // One pixel threshold.
				{
					if (node->isLeaf())
					{
						PointVectorPtr< Float, Vec3 > points = node-> template getContents< PointVector< Float, Vec3 > >();
						for(PointPtr<Float, Vec3> point : *points)
						{
							pointsToDraw.push_back(*point->getPos());
							colorsToDraw.push_back(*point->getColor());
						}
					}
					else
					{
						PointPtr< Float, Vec3 > point = node-> template getContents< Point< Float, Vec3 > >();
						pointsToDraw.push_back(*point->getPos());
						colorsToDraw.push_back(*point->getColor());
					}
				}
				else
				{
					vector< MortonCodePtr< MortonPrecision > > childrenCodes = nodeCode->traverseDown();
			
					for (MortonCodePtr< MortonPrecision > childCode : childrenCodes)
					{
						traverse(childCode, painter, pointsToDraw, colorsToDraw);
					}
				}
			}
		}
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	pair<Vec3, Vec3> OctreeBase< MortonPrecision, Float, Vec3 >::getBoundaries(MortonCodePtr< MortonPrecision > code)
	const
	{
		unsigned int level = code->getLevel();
		vector< MortonPrecision > nodeCoords = code->decode(level);
		Vec3 levelNodeSize = (*m_size) * Float(1 / (2 << level));
		
		Vec3 minBoxVert = (*m_origin) * levelNodeSize;
		Vec3 maxBoxVert = minBoxVert + levelNodeSize;
		
		cout << "Boundaries calc: level = " << level << endl
			 << "node coordinates = " << nodeCoords << endl
			 << "level node size = " << "{" << levelNodeSize[0] << ", " << levelNodeSize[1] << ", " << levelNodeSize[2] << "}" << endl
			 << "min coords = " << glm::to_string(minBoxVert) << endl
			 << "max coords = " << glm::to_string(maxBoxVert) << endl;
		
		return pair< Vec3, Vec3 >(minBoxVert, maxBoxVert);
	}
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	OctreeMapPtr< MortonPrecision, Float, Vec3 > OctreeBase< MortonPrecision, Float, Vec3 >::getHierarchy() const
	{
		return m_hierarchy;
	}
		
	/** Gets the origin, which is the point contained in octree with minimun coordinates for all axis. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	shared_ptr< Vec3 > OctreeBase< MortonPrecision, Float, Vec3 >::getOrigin() const { return m_origin; }
		
	/** Gets the size of the octree, which is the extents in each axis from origin representing the space that the octree occupies. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	shared_ptr< Vec3 > OctreeBase< MortonPrecision, Float, Vec3 >::getSize() const { return m_size; }
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	shared_ptr< Vec3 > OctreeBase< MortonPrecision, Float, Vec3 >::getLeafSize() const { return m_leafSize; }
		
	/** Gets the maximum number of points that can be inside an octree node. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	unsigned int OctreeBase< MortonPrecision, Float, Vec3 >::getMaxPointsPerNode() const { return m_maxPointsPerNode; }
		
	/** Gets the maximum level that this octree can reach. */
	template <typename MortonPrecision, typename Float, typename Vec3>
	unsigned int OctreeBase< MortonPrecision, Float, Vec3 >::getMaxLevel() const { return m_maxLevel; }
	
	template <typename MortonPrecision, typename Float, typename Vec3>
	ostream& operator<<(ostream& out, const OctreeBase< MortonPrecision, Float, Vec3 >& octree)
	{
		out << endl << "=========== Begin Octree ============" << endl << endl
			<< "origin: " << glm::to_string(*octree.m_origin) << endl
			<< "size: " << glm::to_string(*octree.m_size) << endl
			<< "leaf size: " << glm::to_string(*octree.m_leafSize) << endl
			<< "max points per node: " << octree.m_maxPointsPerNode << endl << endl;
		
		/** Maximum level of this octree. */
		unsigned int m_maxLevel;
		OctreeMapPtr< MortonPrecision, Float, Vec3 > hierarchy = octree.getHierarchy();
		for (auto nodeIt = hierarchy->begin(); nodeIt != hierarchy->end(); ++nodeIt)
		{
			MortonCodePtr< MortonPrecision > code = nodeIt->first;
			OctreeNodePtr< MortonPrecision, Float, Vec3 > genericNode = nodeIt->second;
			if (genericNode->isLeaf())
			{
				auto node = dynamic_pointer_cast< PointsLeafNode< MortonPrecision, Float, Vec3 > >(genericNode);
				out << "Node: {" << endl << *code << "," << endl << *node << "}" << endl;
			}
			else
			{
				auto node = dynamic_pointer_cast< LODInnerNode< MortonPrecision, Float, Vec3 > >(genericNode);
				out << "Node: {" << endl << *code << "," << endl << *node << "}" << endl;
			}
			
		}
		out << "=========== End Octree ============" << endl << endl;
		return out;
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
		OctreeBase<unsigned int, Float, Vec3>::m_maxLevel = 10; // 0 to 10.
	}
	
	template<typename Float, typename Vec3>
	Octree<unsigned long, Float, Vec3>::Octree(const int& maxPointsPerNode)
	: OctreeBase< unsigned long, Float, Vec3 >::OctreeBase(maxPointsPerNode)
	{
		OctreeBase<unsigned long, Float, Vec3>::m_maxLevel = 21; // 0 to 21.
	}
	
	template<typename Float, typename Vec3>
	Octree<unsigned long long, Float, Vec3>::Octree(const int& maxPointsPerNode)
	: OctreeBase< unsigned long long, Float, Vec3 >::OctreeBase(maxPointsPerNode)
	{
		OctreeBase<unsigned long long, Float, Vec3>::m_maxLevel = 42; // 0 to 42
	}
}

#endif