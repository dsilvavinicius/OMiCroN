#ifndef OCTREE_H
#define OCTREE_H

#include "Stream.h"

#include <cassert>
#include <map>
#include <ctime>
#include <glm/ext.hpp>

#include "MortonCode.h"
#include "OctreeNode.h"
#include "LeafNode.h"
#include "InnerNode.h"
#include "OctreeMapTypes.h"
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
	 * @param MortonCode is the morton code type.
	 * @param Point is the type used to represent renderable points.
	 */
	template< typename MortonCode, typename Point >
	class OctreeBase
	{
		using MortonCodePtr = shared_ptr< MortonCode >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using PointVectorPtr = shared_ptr< PointVector >;
		using OctreeMap = model::OctreeMap< MortonCode >;
		using OctreeMapPtr = shared_ptr< OctreeMap >;
		using OctreeNode = model::OctreeNode< MortonCode >;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		using InnerNode = model::InnerNode< MortonCode, PointPtr >;
		using InnerNodePtr = shared_ptr< InnerNode >;
		using LeafNode = model::LeafNode< MortonCode, PointVector >;
		using LeafNodePtr = shared_ptr< LeafNode >;
		using Precision = typename PlyPointReader< Point >::Precision;
		using PointAppender = model::PointAppender< MortonCode, Point >;
		using PlyPointReader = util::PlyPointReader< Point >;
		
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
		
		template <typename M, typename P >
		friend ostream& operator<<( ostream& out, const OctreeBase< M, P >& octree );
		
	protected:
		/** Calculates octree's boundaries. */
		virtual void buildBoundaries( const PointVector& points );
		
		/** Creates all nodes bottom-up. In any moment of the building, points can be cleared in order to save memory.*/
		virtual void buildNodes( PointVector& points );
		
		/** Creates all leaf nodes and put points inside them. */
		virtual void buildLeaves( const PointVector& points );
		
		/** Calculates the MortonCode of a Point. */
		MortonCode calcMorton( const Point& point ) const;
		
		/** Inserts a point into the octree leaf it belongs. Creates the node in the process if necessary. */
		virtual void insertPointInLeaf( const PointPtr& point );
		
		/** Creates all inner nodes, with LOD. Bottom-up. If a node has only leaf chilren and the accumulated number of
		 * children points is less than a threshold, the children are merged into parent. */
		virtual void buildInners();
		
		/** Builds the inner node given all child nodes, inserting it into the hierarchy. */
		virtual void buildInnerNode( typename OctreeMap::iterator& firstChildIt,
									 const typename OctreeMap::iterator& currentChildIt,
									 const MortonCodePtr& parentCode, const vector< OctreeNodePtr >& children );
		
		/** Erase a range of nodes, represented by iterator for first (inclusive) and last (not inclusive). */
		void eraseNodes( const typename OctreeMap::iterator& first, const typename OctreeMap::iterator& last );
		
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
	
	template< typename MortonCode, typename Point >
	OctreeBase< MortonCode, Point >::OctreeBase( const int& maxPointsPerNode, const int& maxLevel )
	: m_maxLevel( maxLevel ),
	m_size( new Vec3() ),
	m_leafSize( new Vec3() ),
	m_origin( new Vec3() ),
	m_maxPointsPerNode( maxPointsPerNode ),
	m_hierarchy( new OctreeMap() )
	{
		m_pointAppender = new PointAppender();
	}
	
	template< typename MortonCode, typename Point >
	OctreeBase< MortonCode, Point >::~OctreeBase()
	{
		delete m_pointAppender;
	}
	
	template< typename MortonCode, typename Point >
	void OctreeBase< MortonCode, Point >::build( PointVector& points )
	{
		buildBoundaries( points );
		buildNodes( points );
	}
	
	template< typename MortonCode, typename Point >
	void OctreeBase< MortonCode, Point >::buildFromFile( const string& plyFileName, const Precision& precision,
														 const Attributes& attribs )
	{
		PointVector points;
		PlyPointReader *reader = new PlyPointReader(
			[ & ]( const Point& point )
			{
				points.push_back( PointPtr( new Point( point ) ) );
			}
		);
		reader->read( plyFileName, precision, attribs );
		
		cout << "After reading points" << endl << endl;
		
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
	
	template < typename MortonCode, typename Point >
	void OctreeBase< MortonCode, Point >::buildBoundaries( const PointVector& points )
	{
		Float negInf = -numeric_limits< Float >::max();
		Float posInf = numeric_limits< Float >::max();
		Vec3 minCoords( posInf, posInf, posInf );
		Vec3 maxCoords( negInf, negInf, negInf );
		
		for( PointPtr point : points )
		{
			const Vec3& pos = point->getPos();
			
			for( int i = 0; i < 3; ++i )
			{
				minCoords[ i ] = glm::min( minCoords[ i ], pos[ i ] );
				maxCoords[ i ] = glm::max( maxCoords[ i ], pos[ i ] );
			}
		}
		
		*m_origin = minCoords;
		*m_size = maxCoords - minCoords;
		*m_leafSize = *m_size * ( ( Float )1 / ( ( unsigned long long )1 << m_maxLevel ) );
	}
	
	template< typename MortonCode, typename Point >
	void OctreeBase< MortonCode, Point >::buildNodes( PointVector& points )
	{
		cout << "Before leaf nodes build." << endl << endl;
		buildLeaves(points);
		cout << "After leaf nodes build." << endl << endl;
		
		// From now on the point vector is not necessary. Clear it to save memory.
		points.clear();
		
		buildInners();
		cout << "After inner nodes build." << endl << endl;
	}
	
	template< typename MortonCode, typename Point >
	void OctreeBase< MortonCode, Point >::buildLeaves( const PointVector& points )
	{
		for( PointPtr point : points )
		{
			insertPointInLeaf( point );
		}
	}
	
	template< typename MortonCode, typename Point >
	inline MortonCode OctreeBase< MortonCode, Point >::calcMorton( const Point& point ) const
	{
		const Vec3& pos = point.getPos();
		Vec3 index = ( pos - ( *m_origin ) ) / ( *m_leafSize );
		MortonCode code;
		code.build( index.x, index.y, index.z, m_maxLevel );
		
		return code;
	}
	
	template< typename MortonCode, typename Point >
	inline void OctreeBase< MortonCode, Point >::insertPointInLeaf( const PointPtr& point )
	{
		MortonCodePtr code( new MortonCode( calcMorton( *point ) ) );
		typename OctreeMap::iterator genericLeafIt = m_hierarchy->find( code );
		
		if( genericLeafIt == m_hierarchy->end() )
		{
			// Creates leaf node.
			LeafNodePtr leafNode( new LeafNode() );
			
			PointVector points;
			points.push_back( point );
			leafNode->setContents( points );
			( *m_hierarchy )[ code ] = leafNode;
		}
		else
		{
			// Node already exists. Appends the point there.
			OctreeNodePtr leafNode = genericLeafIt->second;
			PointVector& nodePoints = leafNode-> template getContents< PointVector >();
			nodePoints.push_back( point );
		}
	}
	
	template< typename MortonCode, typename Point >
	void OctreeBase< MortonCode, Point >::buildInners()
	{
		// Do a bottom-up per-level construction of inner nodes.
		for( int level = m_maxLevel - 1; level > -1; --level )
		{
			cout << "========== Octree construction, level " << level << " ==========" << endl << endl;
			// The idea behind this boundary is to get the minimum morton code that is from lower levels than
			// the current. This is the same of the morton code filled with just one 1 bit from the level immediately
			// below the current one. 
			unsigned long long mortonLvlBoundary = ( unsigned long long )( 1 ) << ( 3 * ( level + 1 ) + 1 );
			
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

	template< typename MortonCode, typename Point >
	inline void OctreeBase< MortonCode, Point >::buildInnerNode( typename OctreeMap::iterator& firstChildIt,
																 const typename OctreeMap::iterator& currentChildIt,
															  const MortonCodePtr& parentCode,
															  const vector< OctreeNodePtr >& children )
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
			
			eraseNodes( tempIt, currentChildIt );
			
			// Creates leaf to replace children.
			LeafNodePtr mergedNode( new LeafNode() );
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
	
	template< typename MortonCode, typename Point >
	inline void OctreeBase< MortonCode, Point >::eraseNodes( const typename OctreeMap::iterator& first,
															 const typename OctreeMap::iterator& last )
	{
		cout << "Octree::eraseNodes :" << first->first->getPathToRoot( true ) << " to ";
		if( last != m_hierarchy->end() )
		{
			cout << last->first->getPathToRoot( true ) << endl;
		}
		else
		{
			cout << "end" << endl;
		}
		
		m_hierarchy->erase( first, last );
	}
	
	template< typename MortonCode, typename Point >
	inline OctreeNodePtr< MortonCode > OctreeBase< MortonCode, Point >::buildInnerNode( const PointVector& childrenPoints )
	const
	{	
		// Accumulate points for LOD.
		Point accumulated;
		for( PointPtr point : childrenPoints )
		{
			accumulated = accumulated + ( *point );
		}
		accumulated = accumulated.multiply( 1 / ( Float )childrenPoints.size());
		
		InnerNodePtr LODNode( new InnerNode() );
		LODNode->setContents( PointPtr( new Point( accumulated ) ) );
		
		return LODNode;
	}
	
	template< typename MortonCode, typename Point >
	OctreeStats OctreeBase< MortonCode, Point >::traverse( RenderingState& renderingState, const Float& projThresh )
	{
		clock_t timing = clock();
		
		MortonCodePtr rootCode( new MortonCode() );
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
	
	template< typename MortonCode, typename Point >
	inline pair< Vec3, Vec3 > OctreeBase< MortonCode, Point >::getBoundaries( MortonCodePtr code ) const
	{
		unsigned int level = code->getLevel();
		auto nodeCoordsVec = code->decode(level);
		Vec3 nodeCoords( nodeCoordsVec[ 0 ], nodeCoordsVec[ 1 ], nodeCoordsVec[ 2 ] );
		Float nodeSizeFactor = Float( 1 ) / Float( 1 << level );
		Vec3 levelNodeSize = ( *m_size ) * nodeSizeFactor;
		
		Vec3 minBoxVert = ( *m_origin ) + nodeCoords * levelNodeSize;
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
	
	template< typename MortonCode, typename Point >
	void OctreeBase< MortonCode, Point >::traverse( MortonCodePtr nodeCode, RenderingState& renderingState,
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
	
	template< typename MortonCode, typename Point >
	inline void OctreeBase< MortonCode, Point >::setupInnerNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code,
																		  RenderingState& renderingState )
	{
		assert( !innerNode->isLeaf() && "innerNode cannot be leaf." );
		
		PointPtr& point = innerNode-> template getContents< PointPtr >();
		renderingState.handleNodeRendering( point );
	}
	
	template< typename MortonCode, typename Point >
	inline void OctreeBase< MortonCode, Point >::setupLeafNodeRendering( OctreeNodePtr leafNode, MortonCodePtr code,
																		 RenderingState& renderingState )
	{
		assert( leafNode->isLeaf() && "leafNode cannot be inner." );
		
		PointVector& points = leafNode-> template getContents< PointVector >();
		renderingState.handleNodeRendering( points );
	}
	
	template< typename MortonCode, typename Point >
	inline OctreeMapPtr< MortonCode > OctreeBase< MortonCode, Point >::getHierarchy()
	const
	{
		return m_hierarchy;
	}
		
	/** Gets the origin, which is the point contained in octree with minimun coordinates for all axis. */
	template< typename MortonCode, typename Point >
	inline shared_ptr< Vec3 > OctreeBase< MortonCode, Point >::getOrigin() const { return m_origin; }
		
	/** Gets the size of the octree, which is the extents in each axis from origin representing the space that the octree occupies. */
	template< typename MortonCode, typename Point>
	inline shared_ptr< Vec3 > OctreeBase< MortonCode, Point >::getSize() const { return m_size; }
	
	template< typename MortonCode, typename Point>
	inline shared_ptr< Vec3 > OctreeBase< MortonCode, Point >::getLeafSize() const { return m_leafSize; }
		
	/** Gets the maximum number of points that can be inside an octree node. */
	template< typename MortonCode, typename Point>
	inline unsigned int OctreeBase< MortonCode, Point >::getMaxPointsPerNode() const
	{
		return m_maxPointsPerNode;
	}
		
	/** Gets the maximum level that this octree can reach. */
	template< typename MortonCode, typename Point >
	inline unsigned int OctreeBase< MortonCode, Point >::getMaxLevel() const { return m_maxLevel; }
	
	template< typename MortonCode, typename Point >
	ostream& operator<<( ostream& out, const OctreeBase< MortonCode, Point >& octree )
	{
		using PointVector = model::PointVector;
		using MortonCodePtr = model::MortonCodePtr< MortonCode >;
		using OctreeMapPtr = model::OctreeMapPtr< MortonCode >;
		using OctreeNodePtr = model::OctreeNodePtr< MortonCode >;
		
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
				genericNode-> template output< PointVector >( out );
				out << endl << "}" << endl << endl;
			}
			else
			{
				out << "Node: {" << endl << *code << "," << endl;
				genericNode-> template output< shared_ptr< Point > >( out );
				out << endl << "}" << endl;
			}
			
		}
		out << "=========== End Octree ============" << endl << endl;
		return out;
	}
	
	//=====================================================================
	// Specializations.
	//=====================================================================
	
	template< typename MortonCode, typename Point >
	class Octree {};
	
	template< typename Point >
	class Octree< ShallowMortonCode, Point > : public OctreeBase< ShallowMortonCode, Point >
	{
	public:
		Octree( const int& maxPointsPerNode, const int& maxLevel );
	};
	
	template< typename Point >
	class Octree< MediumMortonCode, Point > : public OctreeBase< MediumMortonCode, Point >
	{
	public:
		Octree( const int& maxPointsPerNode, const int& maxLevel );
	};
	
	//template< typename Point >
	//class Octree< DeepMortonCode, Point > : public OctreeBase< DeepMortonCode, Point >
	//{
	//public:
	//	Octree( const int& maxPointsPerNode, const int& maxLevel );
	//};
	
	template< typename Point >
	Octree< ShallowMortonCode, Point >::Octree( const int& maxPointsPerNode, const int& maxLevel )
	: OctreeBase< ShallowMortonCode, Point >::OctreeBase( maxPointsPerNode, maxLevel )
	{
		OctreeBase< ShallowMortonCode, Point >::m_maxMortonLevel = 10; // 0 to 10.
		assert( ( OctreeBase< ShallowMortonCode, Point >::m_maxLevel <=
			OctreeBase< ShallowMortonCode, Point >::m_maxMortonLevel ) && "Octree level cannot exceed maximum." );
	}
	
	template< typename Point >
	Octree< MediumMortonCode, Point >::Octree( const int& maxPointsPerNode, const int& maxLevel )
	: OctreeBase< MediumMortonCode, Point >::OctreeBase( maxPointsPerNode, maxLevel )
	{
		OctreeBase< MediumMortonCode, Point >::m_maxMortonLevel = 20; // 0 to 20.
		assert( ( OctreeBase< MediumMortonCode, Point >::m_maxLevel <=
			OctreeBase< MediumMortonCode, Point >::m_maxMortonLevel ) && "Octree level cannot exceed maximum." );
	}
	
	//template< typename Point >
	//Octree< DeepMortonCode, Point >::Octree( const int& maxPointsPerNode, const int& maxLevel )
	//: OctreeBase< DeepMortonCode, Point >::OctreeBase( maxPointsPerNode, maxLevel )
	//{
	//	OctreeBase< DeepMortonCode, Point >::m_maxMortonLevel = 42; // 0 to 42
	//	assert( ( OctreeBase< DeepMortonCode, Point >::m_maxLevel <=
	//		OctreeBase< DeepMortonCode, Point >::m_maxMortonLevel ) && "Octree level cannot exceed maximum." );
	//}
	
	// ====================== Type Sugar ================================ /
	using ShallowOctree = Octree< ShallowMortonCode, Point >;
	using ShallowOctreePtr = shared_ptr< ShallowOctree >;
	
	using MediumOctree = Octree< MediumMortonCode, Point >;
	using MediumOctreePtr = shared_ptr< MediumOctree >;
	
	using ShallowExtOctree = Octree< ShallowMortonCode, ExtendedPoint >;
	using ShallowExtOctreePtr = shared_ptr< ShallowExtOctree >;
	
	using MediumExtOctree = Octree< MediumMortonCode, ExtendedPoint >;
	using MediumExtOctreePtr = shared_ptr< MediumExtOctree >;
}

#endif