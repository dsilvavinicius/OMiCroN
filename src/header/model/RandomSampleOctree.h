#ifndef RANDOM_SAMPLE__OCTREE_H
#define RANDOM_SAMPLE__OCTREE_H

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
#include "OctreeParameterTypes.h"
#include <time.h>

using namespace std;
using namespace util;

namespace model
{
	/** Octree which inner nodes have points randomly sampled from child nodes. Provides a smooth transition
	 * between level of detail (LOD), but has the cost of more points being rendered per frame.
	 * @param OctreeParamaters is the struct defining the morton code, point, node and hierarchy types used by the
	 * octree. */
	template< typename OctreeParameters >
	class RandomSampleOctree
	{
		using MortonCode = typename OctreeParameters::Morton;
		using MortonCodePtr = shared_ptr< MortonCode >;
		
		using Point = typename OctreeParameters::Point;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		using PointVectorPtr = shared_ptr< PointVector >;
		
		using OctreeNode = typename OctreeParameters::Node;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		using InnerNode = model::InnerNode< PointVector >;
		using InnerNodePtr = shared_ptr< InnerNode >;
		using LeafNode = model::LeafNode< PointVector >;
		using LeafNodePtr = shared_ptr< LeafNode >;
		
		using OctreeMap = typename OctreeParameters::Hierarchy;
		using OctreeMapPtr = shared_ptr< OctreeMap >;
		
		using PlyPointReader = util::PlyPointReader< Point >;
		using Precision = typename PlyPointReader::Precision;
		using PointAppender = model::PointAppender< MortonCode, Point >;
		using RandomPointAppender = model::RandomPointAppender< MortonCode, Point >;
		
	public:
		/** Initialize data for building the octree, giving the desired max number of nodes per node and the maximum
		 * level of the hierarchy. */
		RandomSampleOctree( const int& maxPointsPerNode, const int& maxLevel );
		
		~RandomSampleOctree();
		
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
		
		/** Calculates the MortonCode of a Point. */
		MortonCode calcMorton( const Point& point ) const;
		
		template< typename P >
		friend ostream& operator<<( ostream& out, const RandomSampleOctree< P >& octree );
		
	protected:
		/** Calculates octree's boundaries. */
		virtual void buildBoundaries( const PointVector& points );
		
		/** Creates all nodes bottom-up. In any moment of the building, points can be cleared in order to save memory.*/
		virtual void buildNodes( PointVector& points );
		
		/** Creates all leaf nodes and put points inside them. */
		virtual void buildLeaves( const PointVector& points );
		
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
		virtual void eraseNodes( const typename OctreeMap::iterator& first, const typename OctreeMap::iterator& last );
		
		/** Traversal recursion. */
		virtual void traverse( MortonCodePtr nodeCode, RenderingState& renderingState, const Float& projThresh );
		
		virtual void setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState );
		
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
		void setMaxLvl( const int& maxLevel, const ShallowMortonCode& );
		void setMaxLvl( const int& maxLevel, const MediumMortonCode& );
		
		/** Builds the inner node given all child points. */
		OctreeNodePtr buildInnerNode( const PointVector& childrenPoints ) const;
	};
	
	template< typename OctreeParameters >
	RandomSampleOctree< OctreeParameters >
	::RandomSampleOctree( const int& maxPointsPerNode, const int& maxLevel )
	: m_size( new Vec3() ),
	m_leafSize( new Vec3() ),
	m_origin( new Vec3() ),
	m_maxPointsPerNode( maxPointsPerNode ),
	m_hierarchy( new OctreeMap() )
	{
		setMaxLvl( maxLevel, MortonCode() );
		m_pointAppender = new RandomPointAppender();
		srand( 1 );
	}
	
	template< typename OctreeParameters >
	RandomSampleOctree< OctreeParameters >::~RandomSampleOctree()
	{
		delete m_pointAppender;
	}
	
	template< typename OctreeParameters >
	void RandomSampleOctree< OctreeParameters >::build( PointVector& points )
	{
		buildBoundaries( points );
		buildNodes( points );
	}
	
	template< typename OctreeParameters >
	void RandomSampleOctree< OctreeParameters >
	::buildFromFile( const string& plyFileName, const Precision& precision, const Attributes& attribs )
	{
		PointVector points;
		PlyPointReader *reader = new PlyPointReader(
			[ & ]( const Point& point )
			{
				points.push_back( makeManaged< Point >( point ) );
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
	
	template< typename OctreeParameters >
	void RandomSampleOctree< OctreeParameters >::buildBoundaries( const PointVector& points )
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
	
	template< typename OctreeParameters >
	void RandomSampleOctree< OctreeParameters >::buildNodes( PointVector& points )
	{
		cout << "Before leaf nodes build." << endl << endl;
		buildLeaves(points);
		cout << "After leaf nodes build." << endl << endl;
		
		// From now on the point vector is not necessary. Clear it to save memory.
		points.clear();
		
		buildInners();
		cout << "After inner nodes build." << endl << endl;
	}
	
	template< typename OctreeParameters >
	void RandomSampleOctree< OctreeParameters >::buildLeaves( const PointVector& points )
	{
		for( PointPtr point : points )
		{
			insertPointInLeaf( point );
		}
	}
	
	template< typename OctreeParameters >
	inline typename OctreeParameters::Morton RandomSampleOctree< OctreeParameters >
	::calcMorton( const Point& point ) const
	{
		const Vec3& pos = point.getPos();
		Vec3 index = ( pos - ( *m_origin ) ) / ( *m_leafSize );
		MortonCode code;
		code.build( index.x, index.y, index.z, m_maxLevel );
		
		return code;
	}
	
	template< typename OctreeParameters >
	inline void RandomSampleOctree< OctreeParameters >::insertPointInLeaf( const PointPtr& point )
	{
		MortonCodePtr code = makeManaged< MortonCode >( calcMorton( *point ) );
		typename OctreeMap::iterator genericLeafIt = m_hierarchy->find( code );
		
		if( genericLeafIt == m_hierarchy->end() )
		{
			// Creates leaf node.
			LeafNodePtr leafNode = makeManaged< LeafNode >();
			
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
	
	template< typename OctreeParameters >
	void RandomSampleOctree< OctreeParameters >::buildInners()
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

	template< typename OctreeParameters >
	inline void RandomSampleOctree< OctreeParameters >::buildInnerNode(
		typename OctreeMap::iterator& firstChildIt, const typename OctreeMap::iterator& currentChildIt,
		const MortonCodePtr& parentCode, const vector< OctreeNodePtr >& children )
	{
		// These counters are used to check if the accumulated number of child node points is less than a threshold.
		// In this case, the children are deleted and their points are merged into the parent.
		int numChildren = 0;
		int numLeaves = 0;
		
		// Points to be accumulated for LOD or to be merged into the parent.
		auto childrenPoints = PointVector();
		
		for( OctreeNodePtr child : children )
		{
			m_pointAppender->appendPoints( child, childrenPoints, numChildren, numLeaves );
		}

		if( numChildren == numLeaves && childrenPoints.size() <= m_maxPointsPerNode )
		{
			//cout << "Merging child into " << endl << parentCode->getPathToRoot( true ) << endl;
			
			// All children are leaves, but they have less points than the threshold and must be merged.
			auto tempIt = firstChildIt;
			advance( firstChildIt, numChildren );
			
			eraseNodes( tempIt, currentChildIt );
			
			// Creates leaf to replace children.
			shared_ptr< LeafNode > mergedNode = makeManaged< LeafNode >();
			mergedNode->setContents( childrenPoints );
			
			( *m_hierarchy )[ parentCode ] = mergedNode;
		}
		else
		{
			//cout << "Creating LOD" << endl << parentCode->getPathToRoot( true )  << endl;
			
			// No merge or absorption is needed. Just does LOD.
			advance( firstChildIt, numChildren );
			
			( *m_hierarchy )[ parentCode ] = buildInnerNode( childrenPoints );
		}
	}
	
	template< typename OctreeParameters >
	inline void RandomSampleOctree< OctreeParameters >::eraseNodes( const typename OctreeMap::iterator& first,
																		 const typename OctreeMap::iterator& last )
	{
		m_hierarchy->erase( first, last );
	}
	
	template< typename OctreeParameters >
	inline shared_ptr< typename OctreeParameters::Node > RandomSampleOctree< OctreeParameters >
	::buildInnerNode( const PointVector& childrenPoints ) const
	{
		unsigned int numChildrenPoints = childrenPoints.size();
		
		InnerNodePtr node = makeManaged< InnerNode >();
		int numSamplePoints = std::max( 1., numChildrenPoints * 0.125 );
		PointVector selectedPoints( numSamplePoints );
		
		// Gets random 1/8 of the number of points.
		for( int i = 0; i < numSamplePoints; ++i )
		{
			int choosenPoint = rand() % numChildrenPoints;
			//cout << "Iter " << i << ". Choosen point index: " << choosenPoint << endl;
			selectedPoints[ i ] = childrenPoints[ choosenPoint ];
		}
		
		node->setContents( selectedPoints );
		return node;
	}
	
	template< typename OctreeParameters >
	OctreeStats RandomSampleOctree< OctreeParameters >::traverse( RenderingState& renderingState,
																	   const Float& projThresh )
	{
		clock_t timing = clock();
		
		MortonCodePtr rootCode = makeManaged< MortonCode >();
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
	
	template< typename OctreeParameters >
	inline pair< Vec3, Vec3 > RandomSampleOctree< OctreeParameters >::getBoundaries( MortonCodePtr code ) const
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
	
	template< typename OctreeParameters >
	void RandomSampleOctree< OctreeParameters >::traverse( MortonCodePtr nodeCode, RenderingState& renderingState,
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
	
	template< typename OctreeParameters >
	inline void RandomSampleOctree< OctreeParameters >
	::setupNodeRendering( OctreeNodePtr node, RenderingState& renderingState )
	{
		PointVector points = node-> template getContents< PointVector >();
		
		renderingState.handleNodeRendering( points );
	}
	
	template< typename OctreeParameters >
	inline void RandomSampleOctree< OctreeParameters >
	::setupInnerNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState )
	{
		assert( !innerNode->isLeaf() && "innerNode cannot be leaf." );
		
		setupNodeRendering( innerNode, renderingState );
	}
	
	template< typename OctreeParameters >
	inline void RandomSampleOctree< OctreeParameters >
	::setupLeafNodeRendering( OctreeNodePtr leafNode, MortonCodePtr code, RenderingState& renderingState )
	{
		assert( leafNode->isLeaf() && "leafNode cannot be inner." );
		
		PointVector& points = leafNode-> template getContents< PointVector >();
		renderingState.handleNodeRendering( points );
	}
	
	template< typename OctreeParameters >
	inline shared_ptr< typename OctreeParameters::Hierarchy > RandomSampleOctree< OctreeParameters >
	::getHierarchy() const
	{
		return m_hierarchy;
	}
		
	/** Gets the origin, which is the point contained in octree with minimun coordinates for all axis. */
	template< typename OctreeParameters >
	inline shared_ptr< Vec3 > RandomSampleOctree< OctreeParameters >::getOrigin() const { return m_origin; }
		
	/** Gets the size of the octree, which is the extents in each axis from origin representing the space that the octree occupies. */
	template< typename OctreeParameters >
	inline shared_ptr< Vec3 > RandomSampleOctree< OctreeParameters >::getSize() const { return m_size; }
	
	template< typename OctreeParameters >
	inline shared_ptr< Vec3 > RandomSampleOctree< OctreeParameters >::getLeafSize() const { return m_leafSize; }
		
	/** Gets the maximum number of points that can be inside an octree node. */
	template< typename OctreeParameters >
	inline unsigned int RandomSampleOctree< OctreeParameters >::getMaxPointsPerNode() const
	{
		return m_maxPointsPerNode;
	}
		
	/** Gets the maximum level that this octree can reach. */
	template< typename OctreeParameters >
	inline unsigned int RandomSampleOctree< OctreeParameters >::getMaxLevel() const { return m_maxLevel; }
	
	template< typename OctreeParameters >
	ostream& operator<<( ostream& out, const RandomSampleOctree< OctreeParameters >& octree )
	{
		using PointPtr = shared_ptr< typename OctreeParameters::Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		using MortonCodePtr = shared_ptr< typename OctreeParameters::Morton >;
		using OctreeMapPtr = shared_ptr< typename OctreeParameters::Hierarchy >;
		
		out << "=========== Begin Octree ============" << endl << endl
			<< "origin: " << glm::to_string( *octree.m_origin ) << endl
			<< "size: " << glm::to_string( *octree.m_size ) << endl
			<< "leaf size: " << glm::to_string( *octree.m_leafSize ) << endl
			<< "max points per node: " << octree.m_maxPointsPerNode << endl << endl;
		
		/** Maximum level of this octree. */
		unsigned int m_maxLevel;
		OctreeMapPtr hierarchy = octree.getHierarchy();
		for( auto nodeIt = hierarchy->begin(); nodeIt != hierarchy->end(); ++nodeIt )
		{
			MortonCodePtr code = nodeIt->first;
			OctreeNodePtr genericNode = nodeIt->second;
			
			out << code->getPathToRoot( true ) << endl;
			//out << "Node: {" << endl << code->getPathToRoot( true ) << "," << endl;
			//genericNode-> template output< PointVector >( out );
			//out << endl << "}" << endl << endl;
		}
		out << "=========== End Octree ============" << endl;
		return out;
	}
	
	template< typename OctreeParameters >
	void RandomSampleOctree< OctreeParameters >::setMaxLvl( const int& maxLevel, const ShallowMortonCode& )
	{
		m_maxLevel = maxLevel;
		m_maxMortonLevel = 10; // 0 to 10.
		
		assert( m_maxLevel <= m_maxMortonLevel && "Octree level cannot exceed maximum." );
	}
	
	template< typename OctreeParameters >
	void RandomSampleOctree< OctreeParameters >::setMaxLvl( const int& maxLevel, const MediumMortonCode& )
	{
		m_maxLevel = maxLevel;
		m_maxMortonLevel = 20; // 0 to 20.
		
		assert( m_maxLevel <= m_maxMortonLevel && "Octree level cannot exceed maximum." );
	}
	
	// ====================== Type Sugar ================================ /
	using ShallowRandomSampleOctree = 	RandomSampleOctree<
											OctreeParameters<
												ShallowMortonCode, Point, OctreeNode,
												OctreeMap< ShallowMortonCode, OctreeNode >
											>
										>;
	using ShallowRandomSampleOctreePtr = shared_ptr< ShallowRandomSampleOctree >;
	
	using MediumRandomSampleOctree = 	RandomSampleOctree<
											OctreeParameters<
												MediumMortonCode, Point, OctreeNode,
												OctreeMap< MediumMortonCode, OctreeNode >
											>
										>;
	using MediumRandomSampleOctreePtr = shared_ptr< MediumRandomSampleOctree >;
	
	using ShallowExtRandomSampleOctree = 	RandomSampleOctree<
												OctreeParameters<
													ShallowMortonCode, ExtendedPoint, OctreeNode,
													OctreeMap< ShallowMortonCode, OctreeNode >
												>
											>;
	using ShallowExtRandomSampleOctreePtr = shared_ptr< ShallowExtRandomSampleOctree >;
	
	using MediumExtRandomSampleOctree = 	RandomSampleOctree<
												OctreeParameters<
													MediumMortonCode, ExtendedPoint, OctreeNode,
													OctreeMap< MediumMortonCode, OctreeNode >
												>
											>;
	using MediumExtRandomSampleOctreePtr = shared_ptr< MediumExtRandomSampleOctree >;
}

#endif