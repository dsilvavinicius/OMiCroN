#ifndef OUT_OF_CORE_DEBUG_OCTREE_H
#define OUT_OF_CORE_DEBUG_OCTREE_H

#include "OutOfCoreOctree.h"

namespace model
{
	/** OutOfCoreOctree that also provides debug info when rendering. */
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	class OutOfCoreDebugOctree
	: public OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >
	{
		using MortonCodePtr = shared_ptr< MortonCode >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		using ParentOctree = OutOfCoreOctree< MortonCode, Point, Front, FrontInsertionContainer >;
		
	public:
		OutOfCoreDebugOctree( const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename,
							  const typename ParentOctree::MemorySetup& memSetup );
		
		OutOfCoreDebugOctree( const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename );
		
		/** Indicates if debug info is needed in next rendering. */
		void toggleDebug( const bool& flag );
		
	protected:
		/** Rendering setup method that also generates a debug string. */
		void setupNodeRendering( OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState ) override;
		
		/** Rendering setup method that also generates a debug string. Don't insert the node into the front (assumes that
		 * it is already inserted). */
		void setupNodeRenderingNoFront( OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState ) override;
		
		bool m_isDebugOn;
	};
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	OutOfCoreDebugOctree< MortonCode, Point, Front, FrontInsertionContainer >::OutOfCoreDebugOctree(
		const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename,
		const typename ParentOctree::MemorySetup& memSetup )
	: ParentOctree( maxPointsPerNode, maxLevel, dbFilename, memSetup ),
	m_isDebugOn( false )
	{}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	OutOfCoreDebugOctree< MortonCode, Point, Front, FrontInsertionContainer >::OutOfCoreDebugOctree(
		const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename )
	: ParentOctree( maxPointsPerNode, maxLevel, dbFilename ),
	m_isDebugOn( false )
	{}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	void OutOfCoreDebugOctree< MortonCode, Point, Front, FrontInsertionContainer >::toggleDebug( const bool& flag )
	{
		m_isDebugOn = flag;
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreDebugOctree< MortonCode, Point, Front, FrontInsertionContainer >::setupNodeRendering(
		OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState )
	{
		if( m_isDebugOn )
		{
			stringstream ss;
			ss << "0x" << hex << code->getBits();
			PointPtr p = node->template getContents< PointVector >()[ 0 ];
			renderingState.renderText( p->getPos(), ss.str() );
		}
		
		ParentOctree::setupNodeRendering( node, code, renderingState );
	}
	
	template< typename MortonCode, typename Point, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreDebugOctree< MortonCode, Point, Front, FrontInsertionContainer >::setupNodeRenderingNoFront(
		OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState )
	{
		if( m_isDebugOn )
		{
			stringstream ss;
			ss << "0x" << hex << code->getBits();
			PointPtr p = node->template getContents< PointVector >()[ 0 ];
			renderingState.renderText( p->getPos(), ss.str() );
		}
		
		ParentOctree::setupNodeRenderingNoFront( node, code, renderingState );
	}
	
	// ====================== Type Sugar ================================ /
	template< typename MortonCode, typename Point >
	using DefaultOutOfCoreDebugOctree = OutOfCoreDebugOctree< 	MortonCode, Point, unordered_set< MortonCode >,
																vector< MortonCode > >;
	
	using ShallowOutOfCoreDebugOctree = DefaultOutOfCoreDebugOctree< ShallowMortonCode, Point >;
	using ShallowOutOfCoreDebugOctreePtr = shared_ptr< ShallowOutOfCoreDebugOctree >;
	
	using MediumOutOfCoreDebugOctree = DefaultOutOfCoreDebugOctree< MediumMortonCode, Point >;
	using MediumOutOfCoreDebugOctreePtr = shared_ptr< MediumOutOfCoreDebugOctree >;
	
	using ShallowExtOutOfCoreDebugOctree = DefaultOutOfCoreDebugOctree< ShallowMortonCode, ExtendedPoint >;
	using ShallowExtOutOfCoreDebugOctreePtr = shared_ptr< ShallowExtOutOfCoreDebugOctree >;
	
	using MediumExtOutOfCoreDebugOctree = DefaultOutOfCoreDebugOctree< MediumMortonCode, ExtendedPoint >;
	using MediumExtOutOfCoreDebugOctreePtr = shared_ptr< MediumExtOutOfCoreDebugOctree >;
}

#endif