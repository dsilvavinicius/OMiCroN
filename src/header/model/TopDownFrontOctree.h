#ifndef TOP_DOWN_FRONT_OCTREE_H
#define TOP_DOWN_FRONT_OCTREE_H

#include <jsoncpp/json/json.h>
#include "Front.h"
#include "OctreeDimensions.h"
#include "RuntimeSetup.h"
#include "PointSorter.h"

namespace model
{
	/** A simple octree created top-down. The heuristic is to subdivide everytime a node contains K points.. */
	template< typename Morton >
	class TopDownFrontOctree
	{
	public:
		using Dim = OctreeDimensions< Morton >;
		using Front = model::Front< Morton >;
		using Node = typename Front::Node;
		using NodeLoader = typename Front::NodeLoader;
		using Renderer = SplatRenderer;
		
		/** @param octreeJson is a Json with the octree dimensions and a binary octree file entry.
		 * @param nodeLoader is the GPU loader used by the front to render the octree. */
		TopDownFrontOctree( const Json::Value& octreeJson, NodeLoader& nodeLoader, const RuntimeSetup& );
		
		/** Unsuported. Just here to fit FastParallelOctree interface. */
		TopDownFrontOctree( const string&, const int, NodeLoader&, const RuntimeSetup& );
		
		OctreeStats trackFront( Renderer& renderer, const Float projThresh );
	
		/** Just here to fit FastParallelOctree interface. */
		void waitCreation(){}
		
		bool isCreationFinished() { return true; }
		
		/** Traverses the hierarchy to calculate its number of nodes and node contents.
		 * @return If the hierarchy is already finished, a pair with first value equals to the number of nodes in the
		 * hierarchy and second values equals to the the number of contents in all nodes. */
		pair< uint, uint > nodeStatistics() const;
		
		/** Just here to fit FastParallelOctree interface. */
		int hierarchyCreationDuration() const { return m_hierarchyConstructionTime; }
		
		const Dim& dim() const { return m_dim; }
		
		Node& root() { return *m_root; }
		
		uint substitutedPlaceholders() const{ return 0u; }
		
		uint readerInTime() { return m_inTime; }
		uint readerInitTime() { return 0; }
		uint readerReadTime() { return 0; }
		
	private:
		void insert( const Point& p, Node& node, const Dim& currentLvlDim );
		
		/** Finds the child of node where p should be inserted. If the node does not exist yet, it is created. */
		Node& findInsertionChild( const Point& p, Node& node, const Dim& childLvlDim );
		
		/** Post-process the octree in order to eliminate empty nodes and shrink buffers. */
		void postProcess( Node& node );
		
		static float& getOffsetReference( typename Node::ContentsArray& contents ) { return ( contents.end() - 1 )->c[ 0 ]; }
		static float& getOffsetReference( Node& node ) { return getOffsetReference( node.getContents() ); }
		
		unique_ptr< Front > m_front;
		unique_ptr< Node > m_root;
		Dim m_dim;
		uint m_inTime;
		uint m_hierarchyConstructionTime;
	};
	
	template< typename Morton >
	inline TopDownFrontOctree< Morton >::TopDownFrontOctree( const Json::Value& octreeJson, NodeLoader& nodeLoader, const RuntimeSetup& )
	{
		throw logic_error( "TopDownFrontOctree creation from octree file is unsuported." );
	}
	
	template< typename Morton >
	inline TopDownFrontOctree< Morton >::TopDownFrontOctree( const string& plyFilename, const int maxLvl, NodeLoader& loader, const RuntimeSetup& setup )
	{
		PointSorter< Morton > sorter( plyFilename, maxLvl );
		PointSet< Morton > pointSet = sorter.points();
		m_dim = pointSet.m_dim;
		
		m_inTime = sorter.inputTime();
		
		auto now = Profiler::now( "Hierarchy construction" );
		
		typename Node::ContentsArray surfels( TOP_DOWN_OCTREE_K );
		
		m_root = unique_ptr< Node >( new Node( surfels, true ) );
		getOffsetReference( *m_root ) = 0.f;
		
		for( const Point& p : *pointSet.m_points )
		{
			insert( p, *m_root, Dim( m_dim, 0 ) );
		}
		
		m_hierarchyConstructionTime = Profiler::elapsedTime( now, "Hierarchy construction" );
		
		m_front = unique_ptr< Front >( new Front( "", m_dim, 1, loader, 8ul * 1024ul * 1024ul * 1024ul ) );
		m_front->insertRoot( *m_root );
		
		postProcess( *m_root );
	}
	
	template< typename Morton >
	inline typename TopDownFrontOctree<Morton>::Node& TopDownFrontOctree< Morton >::findInsertionChild( const Point& p, Node& node, const Dim& childLvlDim )
	{
		Node* insertionChild = nullptr;
		
		for( Node& child : node.child() )
		{
			// Debug
// 			{
// 				cout << "Child: " << childLvlDim.calcMorton( child ).getPathToRoot() << " size: " << getOffsetReference( child ) << endl << endl ;
// 			}
			//
			
			if( childLvlDim.calcMorton( p ) == childLvlDim.calcMorton( child ) )
			{
				insertionChild = &child;
			}
			break;
		}
		
		if( insertionChild == nullptr )
		{
			// Debug
// 			{
// 				cout << "Creating new node" << endl << endl;
// 			}
			
			typename Node::NodeArray newChildArray( node.child().size() + 1 );
			for( int i = 0; i < node.child().size(); ++i )
			{
				newChildArray[ i ] = std::move( node.child()[ i ] );
			}
			newChildArray[ newChildArray.size() - 1 ] = Node( typename Node::ContentsArray( TOP_DOWN_OCTREE_K ), &node );
			getOffsetReference( newChildArray[ newChildArray.size() - 1 ] ) = 0;
			
			node.setChildren( std::move( newChildArray ) );
			
			insertionChild = node.child().end() - 1;
		}
		
		return *insertionChild;
	}

	
	template< typename Morton >
	inline void TopDownFrontOctree< Morton >::insert( const Point& p, Node& node, const Dim& currentLvlDim )
	{
		if( node.isLeaf() )
		{
			// Debug 
// 			{
// 				if( getOffsetReference( node ) == 0 )
// 				{
// 					cout << "Inserting at empty node" << endl << endl;
// 				}
// 				else
// 				{
// 					cout << "Inserting at " << currentLvlDim.calcMorton( node ).getPathToRoot() << endl << endl;
// 				}
// 			}
			//
			
			int offset = getOffsetReference( node )++;
			
			// Debug
// 			{
// 				cout << "offset: " << offset << endl << endl;
// 			}
			
			Surfel surfel( p );
			
// 			Vector2f multipliers = ReconstructionParams::calcAcummulatedMultipliers( currentLvlDim.level(), m_dim.level() - 1 );
// 			surfel.multiplyTangents( multipliers );
			
			node.getContents()[ offset ] = surfel;
			
			if( offset == TOP_DOWN_OCTREE_K - 1 )
			{
				// Subdivision.
				
				// Debug
// 				{
// 					cout << "Subdiv" << endl << endl;
// 				}
				//
				
				Dim nextLvlDim = currentLvlDim.levelBellow();
				
				int movedToInner = 0;
				
				typename Node::ContentsArray prevContents( std::move( node.getContents() ) );
				
				node = Node( typename Node::ContentsArray( TOP_DOWN_OCTREE_K / 8 ), node.parent(), typename Node::NodeArray() );
				
				// Calc the multipliers needed to reverse the tangent multiplication done for the current lvl.
// 				Vector2f reverseMultipliers = ReconstructionParams::calcMultipliers( currentLvlDim.level() );
// 				reverseMultipliers.x() = 1.f / reverseMultipliers.x();
// 				reverseMultipliers.y() = 1.f / reverseMultipliers.y();
				
				for( const Surfel& surfel : prevContents )
				{
					if( movedToInner < node.getContents().size() )
					{
						node.getContents()[ movedToInner++ ] = surfel;
					}
					
					// Fix multipliers for the next level.
					Surfel nextLvlSurfel = surfel;
// 					nextLvlSurfel.multiplyTangents( reverseMultipliers );
					
					Node& child = findInsertionChild( p, node, nextLvlDim );
					insert( p, child, nextLvlDim );
				}
			}
		}
		else
		{
			Dim nextLvlDim = currentLvlDim.levelBellow();
			
			Node& child = findInsertionChild( p, node, nextLvlDim );
			
			// Debug 
// 			{
// 				cout << "Traversing " << currentLvlDim.calcMorton( node ).getPathToRoot() << endl << endl;
// 			}
			//
			
			insert( p, child, nextLvlDim );
		}
	}
	
	template< typename Morton >
	void TopDownFrontOctree< Morton >::postProcess( Node& node )
	{
		if( node.isLeaf() )
		{
			int contentsSize = getOffsetReference( node );
			if( contentsSize < TOP_DOWN_OCTREE_K )
			{
				typename Node::ContentsArray newSurfelArray( contentsSize );
				auto iter = newSurfelArray.begin();
				for( const Surfel& surfel : node.getContents() )
				{
					*iter = surfel;
					iter++;
				}
				node.setContents( std::move( newSurfelArray ) );
			}
		}
		else
		{
			for( Node& child : node.child() )
			{
				postProcess( child );
			}
		}
	}

	
	template< typename Morton >
	inline OctreeStats TopDownFrontOctree< Morton >::trackFront( Renderer& renderer, const Float projThresh )
	{
		m_front->trackFront( renderer, projThresh );
	}
	
	template< typename Morton >
	pair< uint, uint > TopDownFrontOctree< Morton >::nodeStatistics() const
	{
		return m_root->subtreeStatistics();
	}
}

#endif