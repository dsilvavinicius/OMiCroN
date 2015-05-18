#ifndef OCTREE_H
#define OCTREE_H

#include <cassert>
#include <map>
#include <ctime>
#include <glm/ext.hpp>

#include "MortonCode.h"
#include "OctreeNode.h"
#include "LeafNode.h"
#include "MortonComparator.h"
#include "InnerNode.h"
#include "OctreeTypes.h"
#include "OctreeMapTypes.h"
#include "Stream.h"
#include "TransientRenderingState.h"
#include "OctreeStats.h"
#include "PlyPointReader.h"
#include "PointAppender.h"

using namespace std;
using namespace util;

namespace model
{	
	/** Base Octree implemented as a hash-map using morton code as explained here:
	 * http://www.sbgames.org/papers/sbgames09/computing/short/cts19_09.pdf. All parts of construction and
	 * traversal are free for reimplementation on derived classes.
	 * 
	 * @param MortonPrecision is the precision of the morton code for nodes.
	 * @param Float is the glm type specifying the floating point type / precision.
	 * @param Vec3 is the type specifying of the vector with 3 coordinates.
	 * @param Point is the type used to represent renderable points.
	 */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class OctreeBase
	{
		using MortonCode = model::MortonCode< MortonPrecision >;
		using MortonCodePtr = shared_ptr< MortonCode >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		using OctreeMap = model::OctreeMap< MortonPrecision, Float, Vec3 >;
		using OctreeMapPtr = shared_ptr< OctreeMap >;
		using OctreeNode = model::OctreeNode< MortonPrecision, Float, Vec3 >;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		using InnerNode = model::InnerNode< MortonPrecision, Float, Vec3, Point >;
		using InnerNodePtr = shared_ptr< InnerNode >;
		using LeafNode = model::LeafNode< MortonPrecision, Float, Vec3, PointVector >;
		using LeafNodePtr = shared_ptr< LeafNode >;
		using RenderingState = model::RenderingState< Vec3, Float >;
		using TransientRenderingState = model::TransientRenderingState< Vec3, Float >;
		using Precision = typename PlyPointReader< Float, Vec3, Point >::Precision;
		using PointAppender = model::PointAppender< MortonPrecision, Float, Vec3, Point >;
		using PlyPointReader = util::PlyPointReader< Float, Vec3, Point >;
		
	public:
		/** Initialize data for building the octree, giving the desired max number of nodes per node and the maximum level
		 * of the hierarchy. */
		OctreeBase( const int& maxPointsPerNode, const int& maxLevel );
		
		~OctreeBase();
		
		/** Builds the octree for a given point cloud file. The points are expected to be in world coordinates.
		 * @param precision tells the floating precision in which coordinates will be interpreted.
		 * @param attribs tells which point attributes will be loaded from the file. */
		virtual void buildFromFile( const string& plyFileName, const Precision& precision, const Attributes& attribs );
		
		/** Builds the octree for a given point cloud. The points are expected to be in world coordinates. In any moment
		 * of the building, points can be cleared in order to save memory. */
		virtual void build( PointVector& points );
		
		/** Traverses the octree, rendering all necessary points.
		 * @returns the number of rendered points. */
		virtual OctreeStats traverse( RenderingState& renderingState, const Float& projThresh );
		
		// TODO: Make tests for this function.
		/** Computes the boundaries of the node indicated by the given morton code.
		 * @returns a pair of vertices: the first is the box's minimum vertex an the last is the box's maximum
		 * vertex. */
		pair< Vec3, Vec3 > getBoundaries( MortonCodePtr ) const;
		
		virtual OctreeMapPtr getHierarchy() const;
		
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
		
		template <typename M, typename F, typename V, typename P >
		friend ostream& operator<<( ostream& out, const OctreeBase< M, F, V, P >& octree );
		
	protected:
		/** Calculates octree's boundaries. */
		virtual void buildBoundaries( const PointVector& points );
		
		/** Creates all nodes bottom-up. In any moment of the building, points can be cleared in order to save memory.*/
		virtual void buildNodes( PointVector& points );
		
		/** Creates all leaf nodes and put points inside them. */
		virtual void buildLeaves( const PointVector& points );
		
		/** Inserts a point into an octree leaf node identified by the given morton code. Creates the node in the process if
		 *	necessary. */
		virtual void insertPointInLeaf( const PointPtr& point, const MortonCodePtr& code );
		
		/** Creates all inner nodes, with LOD. Bottom-up. If a node has only leaf chilren and the accumulated number of
		 * children points is less than a threshold, the children are merged into parent. */
		virtual void buildInners();
		
		/** Builds the inner node given all child nodes, inserting it into the hierarchy. */
		virtual void buildInnerNode( typename OctreeMap::iterator& firstChildIt,
									 const typename OctreeMap::iterator& currentChildIt,
									 const MortonCodePtr& parentCode, const vector< OctreeNodePtr >& children );
		
		/** Traversal recursion. */
		virtual void traverse( MortonCodePtr nodeCode, RenderingState& renderingState, const Float& projThresh );
		
		/** Setups the rendering of an inner node, putting all necessary points into the rendering list. */
		virtual void setupInnerNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState );
		
		/** Setups the rendering of an leaf node, putting all necessary points into the rendering list. */
		virtual void setupLeafNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState );
		
		/** Handler called whenever a node is culled on traversal. Default implementation does nothing. */
		virtual void handleCulledNode( const MortonCodePtr code ) {};
		
		/** Event called on traversal ending, before rendering. Default implementation does nothing. */
		virtual void onTraversalEnd() {};
		
		/** The hierarchy itself. */
		OctreeMapPtr m_hierarchy;
		
		/** Octree origin, which is the point contained in octree with minimum coordinates for all axis. */
		shared_ptr< Vec3 > m_origin;
		
		/** Spatial size of the octree. */
		shared_ptr< Vec3 > m_size;
		
		/** Spatial size of the leaf nodes. */
		shared_ptr< Vec3 > m_leafSize;
		
		/** Maximum number of points per node. */
		unsigned int m_maxPointsPerNode;
		
		/** Maximum level of this octree. */
		unsigned int m_maxLevel;
		
		/** Maximum level that the type used as morton code of this octree can represent. */
		unsigned int m_maxMortonLevel;
		
		/** Utils to append points before building points. */
		PointAppender* m_pointAppender;
		
	private:
		/** Builds the inner node given all child points. */
		OctreeNodePtr buildInnerNode( const PointVector& childrenPoints ) const;
	};
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	OctreeBase< MortonPrecision, Float, Vec3, Point >::OctreeBase( const int& maxPointsPerNode, const int& maxLevel )
	{
		m_size = make_shared< Vec3 >();
		m_leafSize = make_shared< Vec3 >();
		m_origin = make_shared< Vec3 >();
		m_maxPointsPerNode = maxPointsPerNode;
		m_hierarchy = make_shared< OctreeMap >();
		m_maxLevel = maxLevel;
		m_pointAppender = new PointAppender();
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	OctreeBase< MortonPrecision, Float, Vec3, Point >::~OctreeBase()
	{
		delete m_pointAppender;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void OctreeBase< MortonPrecision, Float, Vec3, Point >::build( PointVector& points )
	{
		buildBoundaries( points );
		buildNodes( points );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void OctreeBase< MortonPrecision, Float, Vec3, Point >::buildFromFile( const string& plyFileName,
																		   const Precision& precision,
																		const Attributes& attribs )
	{
		PlyPointReader *reader = new PlyPointReader();
		reader->read( plyFileName, precision, attribs );
		
		cout << "After reading points" << endl << endl;
		
		PointVector points = reader->getPoints();
		
		//
		/*cout << "Read points" << endl << endl;
		for( int i = 0; i < 10; ++i )
		{
			cout << *points[ i ] << endl << endl;
		}*/
		//
		
		cout << "Attributes:" << reader->getAttributes() << endl << endl;
		
		//cout << "Read points: " << endl << points << endl; 
		
		// From now on the reader is not necessary. Delete it in order to save memory.
		delete reader;
		
		build( points );
	}
	
	template < typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void OctreeBase< MortonPrecision, Float, Vec3, Point >::buildBoundaries( const PointVector& points )
	{
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		Vec3 minCoords( posInf, posInf, posInf );
		Vec3 maxCoords( negInf, negInf, negInf );
		
		for( PointPtr point : points )
		{
			shared_ptr< Vec3 > pos = point->getPos();
			
			for( int i = 0; i < 3; ++i )
			{
				minCoords[ i ] = glm::min( minCoords[ i ], ( *pos )[ i ] );
				maxCoords[ i ] = glm::max( maxCoords[ i ], ( *pos )[ i ] );
			}
		}
		
		*m_origin = minCoords;
		*m_size = maxCoords - minCoords;
		*m_leafSize = *m_size * ( ( Float )1 / ( ( MortonPrecision )1 << m_maxLevel ) );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void OctreeBase< MortonPrecision, Float, Vec3, Point >::buildNodes( PointVector& points )
	{
		cout << "Before leaf nodes build." << endl << endl;
		buildLeaves(points);
		cout << "After leaf nodes build." << endl << endl;
		
		// From now on the point vector is not necessary. Clear it to save memory.
		points.clear();
		
		buildInners();
		cout << "After inner nodes build." << endl << endl;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void OctreeBase< MortonPrecision, Float, Vec3, Point >::buildLeaves( const PointVector& points )
	{
		for( PointPtr point : points )
		{
			shared_ptr< Vec3 > pos = point->getPos();
			Vec3 index = ( ( *pos ) - ( *m_origin ) ) / ( *m_leafSize );
			MortonCodePtr code = make_shared< MortonCode >();
			code->build( ( MortonPrecision )index.x, ( MortonPrecision )index.y, ( MortonPrecision )index.z, m_maxLevel );
			
			insertPointInLeaf( point, code );
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void OctreeBase< MortonPrecision, Float, Vec3, Point >::insertPointInLeaf( const PointPtr& point,
																					  const MortonCodePtr& code )
	{
		typename OctreeMap::iterator genericLeafIt = m_hierarchy->find( code );
		
		if( genericLeafIt == m_hierarchy->end() )
		{
			// Creates leaf node.
			auto leafNode = make_shared< LeafNode >();
			
			PointVector points;
			points.push_back( point );
			leafNode->setContents( points );
			( *m_hierarchy )[ code ] = leafNode;
		}
		else
		{
			// Node already exists. Appends the point there.
			OctreeNodePtr leafNode = genericLeafIt->second;
			shared_ptr< PointVector > nodePoints = leafNode-> template getContents< PointVector >();
			nodePoints->push_back( point );
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void OctreeBase< MortonPrecision, Float, Vec3, Point >::buildInners()
	{
		// Do a bottom-up per-level construction of inner nodes.
		for( int level = m_maxLevel - 1; level > -1; --level )
		{
			cout << "========== Octree construction, level " << level << " ==========" << endl << endl;
			// The idea behind this boundary is to get the minimum morton code that is from lower levels than
			// the current. This is the same of the morton code filled with just one 1 bit from the level immediately
			// below the current one. 
			MortonPrecision mortonLvlBoundary = MortonPrecision( 1 ) << ( 3 * ( level + 1 ) + 1 );
			
			//cout << "Morton lvl boundary: 0x" << hex << mortonLvlBoundary << dec << endl;
			
			typename OctreeMap::iterator firstChildIt = m_hierarchy->begin(); 
			
			// Loops per siblings in a level.
			while( firstChildIt != m_hierarchy->end() && firstChildIt->first->getBits() < mortonLvlBoundary )
			{
				MortonCodePtr parentCode = firstChildIt->first->traverseUp();
				
				auto children = vector< OctreeNodePtr >();
				
				// Puts children into vector.
				children.push_back( firstChildIt->second );
				
				typename OctreeMap::iterator currentChildIt = firstChildIt;
				while( ( ++currentChildIt ) != m_hierarchy->end() && *currentChildIt->first->traverseUp() == *parentCode )
				{
					OctreeNodePtr currentChild = currentChildIt->second;
					children.push_back( currentChild );
				}
				
				buildInnerNode( firstChildIt, currentChildIt, parentCode, children );
			}
			
			cout << "========== End of level " << level << " ==========" << endl << endl;
		}
	}

	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void OctreeBase< MortonPrecision, Float, Vec3, Point >::buildInnerNode(
		typename OctreeMap::iterator& firstChildIt, const typename OctreeMap::iterator& currentChildIt,
		const MortonCodePtr& parentCode, const vector< OctreeNodePtr >& children )
	{
		// These counters are used to check if the accumulated number of child node points is less than a threshold.
		// In this case, the children are deleted and their points are merged into the parent.
		int numChildren = 0;
		int numLeaves = 0;
		
		// Points to be accumulated for LOD or to be merged into the parent.
		auto childrenPoints = PointVector();
		
		//cout << "numChildren: " << numChildren << endl << "numLeaves" << numLeaves << endl;
		
		for( OctreeNodePtr child : children )
		{
			m_pointAppender->appendPoints( child, childrenPoints, numChildren, numLeaves );
		}
		
		if( numChildren == numLeaves && childrenPoints.size() <= m_maxPointsPerNode )
		{
			//cout << "Will merge children." << endl << endl;
			// All children are leaves, but they have less points than the threshold and must be merged.
			auto tempIt = firstChildIt;
			advance( firstChildIt, numChildren );
			
			m_hierarchy->erase( tempIt, currentChildIt );
			
			// Creates leaf to replace children.
			auto mergedNode = make_shared< LeafNode >();
			mergedNode->setContents( childrenPoints );
			
			( *m_hierarchy )[ parentCode ] = mergedNode;
		}
		else
		{
			//cout << "Just LOD." << endl << endl;
			// No merge or absorption is needed. Just does LOD.
			advance( firstChildIt, numChildren );
			
			( *m_hierarchy )[ parentCode ] = buildInnerNode( childrenPoints );
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline OctreeNodePtr< MortonPrecision, Float, Vec3 > OctreeBase< MortonPrecision, Float, Vec3, Point >::
		buildInnerNode( const PointVector& childrenPoints ) const
	{	
		// Accumulate points for LOD.
		Point accumulated;
		for( PointPtr point : childrenPoints )
		{
			accumulated = accumulated + ( *point );
		}
		accumulated = accumulated.multiply( 1 / ( Float )childrenPoints.size());
		
		// Creates leaf to replace children.
		auto LODNode = make_shared< InnerNode >();
		LODNode->setContents( accumulated );
		
		return LODNode;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	OctreeStats OctreeBase< MortonPrecision, Float, Vec3, Point >::traverse( RenderingState& renderingState,
																	  const Float& projThresh )
	{
		clock_t timing = clock();
		
		MortonCodePtr rootCode = make_shared< MortonCode >();
		rootCode->build( 0x1 );
		
		traverse( rootCode, renderingState, projThresh );
		
		onTraversalEnd();
		
		timing = clock() - timing;
		float traversalTime = float( timing ) / CLOCKS_PER_SEC * 1000;
		
		timing = clock();
		
		unsigned int numRenderedPoints = renderingState.render();
		timing = clock() - timing;
		
		float renderingTime = float( timing ) / CLOCKS_PER_SEC * 1000;
		
		return OctreeStats( traversalTime, renderingTime, numRenderedPoints );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline pair< Vec3, Vec3 > OctreeBase< MortonPrecision, Float, Vec3, Point >::getBoundaries(
		MortonCodePtr code ) const
	{
		unsigned int level = code->getLevel();
		vector< MortonPrecision > nodeCoordsVec = code->decode(level);
		Vec3 nodeCoords(nodeCoordsVec[0], nodeCoordsVec[1], nodeCoordsVec[2]);
		Float nodeSizeFactor = Float(1) / Float(1 << level);
		Vec3 levelNodeSize = (*m_size) * nodeSizeFactor;
		
		Vec3 minBoxVert = (*m_origin) + nodeCoords * levelNodeSize;
		Vec3 maxBoxVert = minBoxVert + levelNodeSize;
		
		/*cout << "Boundaries for node 0x" << hex << code->getBits() << dec << endl
			 << "level = " << level << endl
			 << "node coordinates = " << glm::to_string(nodeCoords) << endl
			 << "node size factor = " << nodeSizeFactor << endl
			 << "level node size = " << glm::to_string(levelNodeSize) << endl
			 << "min coords = " << glm::to_string(minBoxVert) << endl
			 << "max coords = " << glm::to_string(maxBoxVert) << endl;*/
		
		pair< Vec3, Vec3 > box( minBoxVert, maxBoxVert );
		
		return box;
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	void OctreeBase< MortonPrecision, Float, Vec3, Point >::traverse( MortonCodePtr nodeCode, RenderingState& renderingState,
																	  const Float& projThresh )
	{
		//cout << "TRAVERSING " << *nodeCode << endl << endl;
		auto nodeIt = m_hierarchy->find( nodeCode );
		if ( nodeIt != m_hierarchy->end() )
		{
			MortonCodePtr code = nodeIt->first;
			OctreeNodePtr node = nodeIt->second;
			pair< Vec3, Vec3 > box = getBoundaries( code );
			
			if( !renderingState.isCullable( box ) )
			{
				//cout << *nodeCode << "NOT CULLED!" << endl << endl;
				if( renderingState.isRenderable( box, projThresh ) )
				{
					//cout << *nodeCode << "RENDERED!" << endl << endl;
					if ( node->isLeaf() )
					{
						setupLeafNodeRendering( node, code, renderingState );
					}
					else
					{
						setupInnerNodeRendering( node, code, renderingState );
					}
				}
				else
				{
					if (node->isLeaf())
					{
						setupLeafNodeRendering( node, code, renderingState );
					}
					else
					{
						vector< MortonCodePtr > childrenCodes = nodeCode->traverseDown();
			
						for ( MortonCodePtr childCode : childrenCodes )
						{
							traverse( childCode, renderingState, projThresh );
						}
					}
				}
			}
			else
			{
				handleCulledNode( code );
			}
		}
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void OctreeBase< MortonPrecision, Float, Vec3, Point >::setupInnerNodeRendering(
		OctreeNodePtr innerNode, MortonCodePtr code,
		RenderingState& renderingState )
	{
		assert( !innerNode->isLeaf() && "innerNode cannot be leaf." );
		
		PointPtr point = innerNode-> template getContents< Point >();
		renderingState.handleNodeRendering( point );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline void OctreeBase< MortonPrecision, Float, Vec3, Point >::setupLeafNodeRendering( OctreeNodePtr leafNode, MortonCodePtr code,
		RenderingState& renderingState )
	{
		assert( leafNode->isLeaf() && "leafNode cannot be inner." );
		
		PointVectorPtr points = leafNode-> template getContents< PointVector >();
		renderingState.handleNodeRendering( points );
	}
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline OctreeMapPtr< MortonPrecision, Float, Vec3 > OctreeBase< MortonPrecision, Float, Vec3, Point >::getHierarchy()
	const
	{
		return m_hierarchy;
	}
		
	/** Gets the origin, which is the point contained in octree with minimun coordinates for all axis. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline shared_ptr< Vec3 > OctreeBase< MortonPrecision, Float, Vec3, Point >::getOrigin() const { return m_origin; }
		
	/** Gets the size of the octree, which is the extents in each axis from origin representing the space that the octree occupies. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point>
	inline shared_ptr< Vec3 > OctreeBase< MortonPrecision, Float, Vec3, Point >::getSize() const { return m_size; }
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point>
	inline shared_ptr< Vec3 > OctreeBase< MortonPrecision, Float, Vec3, Point >::getLeafSize() const { return m_leafSize; }
		
	/** Gets the maximum number of points that can be inside an octree node. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point>
	inline unsigned int OctreeBase< MortonPrecision, Float, Vec3, Point >::getMaxPointsPerNode() const
	{
		return m_maxPointsPerNode;
	}
		
	/** Gets the maximum level that this octree can reach. */
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	inline unsigned int OctreeBase< MortonPrecision, Float, Vec3, Point >::getMaxLevel() const { return m_maxLevel; }
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	ostream& operator<<( ostream& out, const OctreeBase< MortonPrecision, Float, Vec3, Point >& octree )
	{
		using PointVector = model::PointVector< Float, Vec3 >;
		using MortonCodePtr = model::MortonCodePtr< MortonPrecision >;
		using OctreeMapPtr = model::OctreeMapPtr< MortonPrecision, Float, Vec3 >;
		using OctreeNodePtr = model::OctreeNodePtr< MortonPrecision, Float, Vec3 >;
		
		out << endl << "=========== Begin Octree ============" << endl << endl
			<< "origin: " << glm::to_string(*octree.m_origin) << endl
			<< "size: " << glm::to_string(*octree.m_size) << endl
			<< "leaf size: " << glm::to_string(*octree.m_leafSize) << endl
			<< "max points per node: " << octree.m_maxPointsPerNode << endl << endl;
		
		/** Maximum level of this octree. */
		unsigned int m_maxLevel;
		OctreeMapPtr hierarchy = octree.getHierarchy();
		for( auto nodeIt = hierarchy->begin(); nodeIt != hierarchy->end(); ++nodeIt )
		{
			MortonCodePtr code = nodeIt->first;
			OctreeNodePtr genericNode = nodeIt->second;
			if ( genericNode->isLeaf() )
			{
				out << "Node: {" << endl << *code << "," << endl;
				operator<< < MortonPrecision, Float, Vec3, PointVector >( out, *genericNode );
				out << endl << "}" << endl << endl;
			}
			else
			{
				out << "Node: {" << endl << *code << "," << endl;
				operator<< < MortonPrecision, Float, Vec3, Point >( out, *genericNode );
				out << "}" << endl;
			}
			
		}
		out << "=========== End Octree ============" << endl << endl;
		return out;
	}
	
	//=====================================================================
	// Specializations.
	//=====================================================================
	
	template< typename MortonPrecision, typename Float, typename Vec3, typename Point >
	class Octree {};
	
	template< typename Float, typename Vec3, typename Point >
	class Octree< unsigned int, Float, Vec3, Point > : public OctreeBase< unsigned int, Float, Vec3, Point >
	{
	public:
		Octree( const int& maxPointsPerNode, const int& maxLevel );
	};
	
	template< typename Float, typename Vec3, typename Point >
	class Octree< unsigned long, Float, Vec3, Point > : public OctreeBase< unsigned long, Float, Vec3, Point >
	{
	public:
		Octree( const int& maxPointsPerNode, const int& maxLevel );
	};
	
	template< typename Float, typename Vec3, typename Point >
	class Octree< unsigned long long, Float, Vec3, Point > : public OctreeBase< unsigned long long, Float, Vec3, Point >
	{
	public:
		Octree( const int& maxPointsPerNode, const int& maxLevel );
	};
	
	template< typename Float, typename Vec3, typename Point >
	Octree< unsigned int, Float, Vec3, Point >::Octree( const int& maxPointsPerNode, const int& maxLevel )
	: OctreeBase< unsigned int, Float, Vec3, Point >::OctreeBase( maxPointsPerNode, maxLevel )
	{
		OctreeBase< unsigned int, Float, Vec3, Point >::m_maxMortonLevel = 10; // 0 to 10.
		assert( ( OctreeBase< unsigned int, Float, Vec3, Point >::m_maxLevel <=
			OctreeBase< unsigned int, Float, Vec3, Point >::m_maxMortonLevel ) && "Octree level cannot exceed maximum." );
	}
	
	template< typename Float, typename Vec3, typename Point >
	Octree< unsigned long, Float, Vec3, Point >::Octree( const int& maxPointsPerNode, const int& maxLevel )
	: OctreeBase< unsigned long, Float, Vec3, Point >::OctreeBase( maxPointsPerNode, maxLevel )
	{
		OctreeBase< unsigned long, Float, Vec3, Point >::m_maxMortonLevel = 20; // 0 to 20.
		assert( ( OctreeBase< unsigned long, Float, Vec3, Point >::m_maxLevel <=
			OctreeBase< unsigned long, Float, Vec3, Point >::m_maxMortonLevel ) && "Octree level cannot exceed maximum." );
	}
	
	template< typename Float, typename Vec3, typename Point >
	Octree< unsigned long long, Float, Vec3, Point >::Octree( const int& maxPointsPerNode, const int& maxLevel )
	: OctreeBase< unsigned long long, Float, Vec3, Point >::OctreeBase( maxPointsPerNode, maxLevel )
	{
		OctreeBase< unsigned long long, Float, Vec3, Point >::m_maxMortonLevel = 42; // 0 to 42
		assert( ( OctreeBase< unsigned long long, Float, Vec3, Point >::m_maxLevel <=
			OctreeBase< unsigned long long, Float, Vec3, Point >::m_maxMortonLevel ) && "Octree level cannot exceed maximum." );
	}
}

#endif