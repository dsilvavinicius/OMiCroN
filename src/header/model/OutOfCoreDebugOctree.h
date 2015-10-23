#ifndef OUT_OF_CORE_DEBUG_OCTREE_H
#define OUT_OF_CORE_DEBUG_OCTREE_H

#include "OutOfCoreOctree.h"

namespace model
{
	/** OutOfCoreOctree that also provides debug info when rendering. */
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	class OutOfCoreDebugOctree
	: public OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >
	{
		using MortonCode = typename OctreeParams::Morton;
		using MortonCodePtr = shared_ptr< MortonCode >;
		
		using Point = typename OctreeParams::Point;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		
		using OctreeNode = typename OctreeParams::Node;
		using OctreeNodePtr = shared_ptr< OctreeNode >;
		
		using ParentOctree = OutOfCoreOctree< OctreeParams, Front, FrontInsertionContainer >;
		
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
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	OutOfCoreDebugOctree< OctreeParams, Front, FrontInsertionContainer >::OutOfCoreDebugOctree(
		const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename,
		const typename ParentOctree::MemorySetup& memSetup )
	: ParentOctree( maxPointsPerNode, maxLevel, dbFilename, memSetup ),
	m_isDebugOn( false )
	{}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	OutOfCoreDebugOctree< OctreeParams, Front, FrontInsertionContainer >::OutOfCoreDebugOctree(
		const int& maxPointsPerNode, const int& maxLevel, const string& dbFilename )
	: ParentOctree( maxPointsPerNode, maxLevel, dbFilename ),
	m_isDebugOn( false )
	{}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	void OutOfCoreDebugOctree< OctreeParams, Front, FrontInsertionContainer >::toggleDebug( const bool& flag )
	{
		m_isDebugOn = flag;
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreDebugOctree< OctreeParams, Front, FrontInsertionContainer >::setupNodeRendering(
		OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState )
	{
		if( m_isDebugOn )
		{
			stringstream ss;
			ss << "0x" << hex << code->getBits();
			PointPtr p = node->getContents()[ 0 ];
			renderingState.renderText( p->getPos(), ss.str() );
		}
		
		ParentOctree::setupNodeRendering( node, code, renderingState );
	}
	
	template< typename OctreeParams, typename Front, typename FrontInsertionContainer >
	inline void OutOfCoreDebugOctree< OctreeParams, Front, FrontInsertionContainer >::setupNodeRenderingNoFront(
		OctreeNodePtr node, MortonCodePtr code, RenderingState& renderingState )
	{
		if( m_isDebugOn )
		{
			stringstream ss;
			ss << "0x" << hex << code->getBits();
			PointPtr p = node->getContents()[ 0 ];
			renderingState.renderText( p->getPos(), ss.str() );
		}
		
		ParentOctree::setupNodeRenderingNoFront( node, code, renderingState );
	}
	
	// ====================== Type Sugar ================================ /
	template< typename OctreeParams >
	using DefaultOutOfCoreDebugOctree = OutOfCoreDebugOctree< 	OctreeParams,
																unordered_set< typename OctreeParams::Morton >,
																vector< typename OctreeParams::Morton > >;
	
	DECLARE_OCTREE_TYPE(SPOpS,OutOfCoreDebugOctree,DefaultOutOfCoreDebugOctree,ShallowMortonCode,Point,OctreeNode< PointVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(MPOpS,OutOfCoreDebugOctree,DefaultOutOfCoreDebugOctree,MediumMortonCode,Point,OctreeNode< PointVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(SEOpS,OutOfCoreDebugOctree,DefaultOutOfCoreDebugOctree,ShallowMortonCode,ExtendedPoint,OctreeNode< ExtendedPointVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(MEOpS,OutOfCoreDebugOctree,DefaultOutOfCoreDebugOctree,MediumMortonCode,ExtendedPoint,OctreeNode< ExtendedPointVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(SPOiS,OutOfCoreDebugOctree,DefaultOutOfCoreDebugOctree,ShallowMortonCode,Point,OctreeNode< IndexVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(MPOiS,OutOfCoreDebugOctree,DefaultOutOfCoreDebugOctree,MediumMortonCode,Point,OctreeNode< IndexVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(SEOiS,OutOfCoreDebugOctree,DefaultOutOfCoreDebugOctree,ShallowMortonCode,ExtendedPoint,OctreeNode< IndexVector >,OctreeMap)
	
	DECLARE_OCTREE_TYPE(MEOiS,OutOfCoreDebugOctree,DefaultOutOfCoreDebugOctree,MediumMortonCode,ExtendedPoint,OctreeNode< IndexVector >,OctreeMap)
}

#endif