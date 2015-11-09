#ifndef OUT_OF_CORE_FAST_OCTREE_H
#define OUT_OF_CORE_FAST_OCTREE_H

#include "OutOfCoreOctree.h"

namespace model
{
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	class OutOfCoreFastOctree
	: public OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	{
	public:
		using ParentOctree = model::OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >;
		
		OutOfCoreFastOctree( const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename,
							 const MemorySetup& memSetup = OutOfCoreOctree::MemorySetup( 0.1f, 0.2f, 200, 100, 1000 ) );
		
		// ==================
		// RandomSampleOctree
		// ==================
		
		/** Traverses the octree, rendering all necessary points.
		 * @returns the number of rendered points. */
		virtual OctreeStats traverse( RenderingState& renderingState, const Float& projThresh );
		
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
		
		// ===========
		// FrontOctree
		// ===========
		
		/** Tracks the hierarchy front, by prunning or branching nodes ( one level only ). This method should be called
		 * after ParentOctree::traverse( RenderingState& renderer, const Float& projThresh ),
		 * so the front can be init in a traversal from root. */
		FrontOctreeStats trackFront( RenderingState& renderer, const Float& projThresh );
	
		/** Tracks one node of the front.
		 * @returns true if the node represented by code should be deleted or false otherwise. */
		bool trackNode( MortonCodePtr& code, RenderingState& renderingState, const Float& projThresh );
		
		// ===============
		// OutOfCoreOctree
		// ===============
		
		/** Builds octree using the database. */
		virtual void build();
		
		virtual void buildFromFile( const string& plyFileName, const typename PlyPointReader::Precision& precision,
									const Attributes& attribs ) override;
		
		SQLiteManager& getSQLiteManager() { return m_sqLite; }
		
		/** DEPRECATED: use build() instead. */
		virtual void build( PointVector& points ) override;
		
	protected:
		
		// ==================
		// RandomSampleOctree
		// ==================
		
		/** Builds the inner node given all child nodes, inserting it into the hierarchy. */
		virtual void buildInnerNode( typename OctreeMap::iterator& firstChildIt,
									 const typename OctreeMap::iterator& currentChildIt,
									 const MortonCodePtr& parentCode, const vector< OctreeNodePtr >& children );
		
		/** Traversal recursion. */
		virtual void traverse( MortonCodePtr nodeCode, RenderingState& renderingState, const Float& projThresh );
		
		// ===========
		// FrontOctree
		// ===========
		
		/** Checks if the node and their siblings should be pruned from the front, giving place to their parent. */
		bool checkPrune( RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh ) const;
		
		/** Creates the deletion and insertion entries related with the prunning of the node and their siblings and
		 * puts parent points into renderingState if needed. */
		void prune( const MortonCodePtr& code, RenderingState& renderingState );
		
		/** Check if the node should be branched, giving place to its children.
		 * @param isCullable is an output that indicates if the node was culled by frustrum. */
		bool checkBranch( RenderingState& renderingState, const MortonCodePtr& code, const Float& projThresh,
			bool& out_isCullable ) const;
		
		/** Creates deletion ansd insertion entries related with the branching of the node and puts children's points
		 * into renderer if necessary.
		 * @returns true if code identifies an inner node (and must be deleted to make place for its children), false
		 * otherwise. */
		bool branch( const MortonCodePtr& code, RenderingState& renderingState );
		
		/** Overriden to add rendered node into front addition list. */
		void setupLeafNodeRendering( OctreeNodePtr leafNode, MortonCodePtr code, RenderingState& renderingState );
		
		/** Overriden to add rendered node into front addition list. */
		void setupInnerNodeRendering( OctreeNodePtr innerNode, MortonCodePtr code, RenderingState& renderingState );
		
		/** Overriden to add culled node into front addition list. */
		void handleCulledNode( MortonCodePtr code );
		
		/** Rendering setup method for both leaf and inner node cases. Inserts the node into the front. */
		virtual void setupNodeRendering( OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState );
		
		/** Rendering setup method for both leaf and inner node cases. Doesn't insert the node into the front, assuming it
		 * is inserted. */
		virtual void setupNodeRenderingNoFront( OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState );
		
		// ===============
		// OutOfCoreOctree
		// ===============
		
		virtual void insertPointInLeaf( const PointPtr& point ) override;
		
		/** Acquires a node with given morton code. Searches in-memory hierarchy first and database if not found.
		 * DOES NOT TRACK PERSISTENCE, since the traversal and creation algorithms expect the atomic persistence
		 * operation to have a per-sibling basis. 
		 * @return A smart-pointer to the node or nullptr if not found. */
		OctreeNodePtr getNode( const MortonCodePtr& code );
		
		/** Acquires all child nodes of a given parent node code. Searches in-memory hierarchy first and database if
		 * not found. It assumes that the parameter is a code for an inner node. Using this method for a leaf node code
		 * will result in unnecessary accesses to database. TRACKS PERSISTENCE.
		 * @return A vector with all found nodes. */
		vector< OctreeNodePtr > getChildren( const MortonCodePtr& parent );
		
		/** Get a query for a range of nodes in closed interval [ a, b ]. */
		SQLiteQuery getRangeInDB( const MortonCodePtr& a, const MortonCodePtr& b );
		
		void buildInners() override;
		
		void eraseNodes( const typename OctreeMap::iterator& first, const typename OctreeMap::iterator& last ) override;
		
		/** This implementation makes a node load request if the iterator indicates that the node was not found in
		 * memory. */
		void onPrunningItAcquired( const typename OctreeMap::iterator& it, const MortonCodePtr& code ) override;
		
		/** This implementation makes a node load request if the iterator indicates that the node was not found in
		 * memory. */
		void onBranchingItAcquired( const typename OctreeMap::iterator& it, const MortonCodePtr& code ) override;
		
		/** Override to also check for completed node requests. */
		void onTraversalEnd() override;
		
		/** DEPRECATED. */
		virtual void buildBoundaries( const PointVector& points ) override;
		
		/** DEPRECATED. */
		virtual void buildNodes( PointVector& points ) override;
		
		/** DEPRECATED. */
		virtual void buildLeaves( const PointVector& points ) override;
		
	private:
		// ==================
		// RandomSampleOctree
		// ==================
		
		void setMaxLvl( const int& maxLevel, const ShallowMortonCode& );
		void setMaxLvl( const int& maxLevel, const MediumMortonCode& );
		
		/** Builds the inner node given all child points. */
		OctreeNodePtr buildInnerNode( const PointVector& childrenPoints ) const;
		
		// ===============
		// OutOfCoreOctree
		// ===============
		
		/** Releases nodes in the in-memory hierarchy at hierarchy creation time in order to ease memory pressure.
		 * Persists all released "dirty" nodes in the database. TRACKS PERSISTENCE.
		 * @returns the last released node code or nullptr in case of no release. */
		MortonCodePtr releaseNodesAtCreation();
		
		/** Releases nodes while tracking front in order to ease memory pressure. All not-front node are eligible to
		 * release and it is done until the memory pressure threshold is achieved. */
		void releaseNodesAtFrontTracking();
		
		/** Persists and release leaf nodes at hierarchy creation in order to ease memory pressure. Since leaf nodes
		 * are acessed in a random pattern and a loaded node is assured to be modified in the near future, this method
		 * just assumes all in-memory nodes dirty, sending them to the database whenever released. This trait also imposes
		 * NO PERSISTENCE TRACKING. */
		void persistAndReleaseLeaves();
		
		/** Persists all leaf nodes in order to start bottom-up inner nodes creation. Also load a few nodes to start
		 working. DOESN'T TRACK PERSISTENCE. */
		void persistAllLeaves();
		
		/** Persists all dirty nodes currently in memory while in inner nodes creation. TRACKS PERSISTENCE. */
		void persistAllDirty();
		
		/** Checks if a node is dirty and needs to be persisted before released. */
		bool isDirty( const MortonCodePtr& code ) const;
		
		/** Load nodes from database at hierarchy creation and revalidates the iterator. */
		void loadNodesAndValidateIter( const MortonCodePtr& nextFirstChildCode, const MortonCodePtr& lvlBoundary,
									   typename OctreeMap::iterator& firstChildIt );
		
		/** Load sibling groups in a query.
		 * @param query is the query with nodes to load.
		 * @returns The first loaded MortonCode or nullptr if the query returns no node. */
		shared_ptr< MortonCode > loadSiblingGroups( SQLiteQuery& query );
	};
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	OutOfCoreFastOctree< OctreeParams, Front, FrontInsertionContainer >
	::OutOfCoreFastOctree( const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename,
						   const MemorySetup& memSetup )
	: ParentOctree( maxPointsPerNode, maxLevel, dbFilename, memSetup ) {}
}

#endif