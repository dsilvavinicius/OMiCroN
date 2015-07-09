#ifndef RENDERING_STATE_H
#define RENDERING_STATE_H

#include <stdexcept>
#include <vector>
#include "ExtendedPoint.h"
#include "Stream.h"

using namespace std;

namespace model
{
	enum Attributes
	{
		COLORS = 0x1,
		NORMALS = 0x2,
		COLORS_AND_NORMALS = 0x3
	};
	
	/** Renders related data used while traversing octree. USAGE: call handleNodeRendering() to indicate that the
	 * contents of octree nodes should be rendered. After all nodes are issued for rendering, call render() to
	 * render them all.
	 * @param Vec3 is the type for 3-dimensional vector.
	 * @param Float is the type for floating point numbers. */
	
	class RenderingState
	{
	public:
		RenderingState( const Attributes& attribs );
		
		/** Renders the current state. Should be called at the end of the traversal, when all rendered nodes have
		 * been already handled.
		 * @returns the number of rendered points. */
		virtual unsigned int render() = 0;
		
		/** Checks if the axis-aligned box is culled by camera frustum.
		 * @returns true if the box should be culled and false otherwise. */
		virtual bool isCullable( const pair< Vec3, Vec3 >& box ) const = 0;
		
		/** Checks if the axis-aligned box is renderable with the current projection threshold.
		 * @returns true if the box has a projection compatible with the current threshold and, thus, should be
		 * rendered. False otherwise (indicating that the traversal should proceed deeper in the hierarchy). */
		virtual bool isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh ) const = 0;
		
		/** Indicates that the node contents passed should be rendered. A static method was used to overcome C++ limitation of
		* class member specializations.
		* @param NodeContents is the type of nodes' contents. The octree is aware of this type.
		*/
		template< typename NodeContents >
		void handleNodeRendering( const NodeContents& contents );
		
		vector< Vec3 >& getPositions() { return m_positions; }
		vector< Vec3 >& getColors() { return m_colors; }
		vector< Vec3 >& getNormals() { return m_normals; }
		vector< unsigned int >& getIndices() { return m_indices; }
		Attributes getAttribs() { return m_attribs; }
	
		/** Clears all attrib vectors. */
		void clearAttribs();
		
		/** Clears indices. */
		void clearIndices();
		
	protected:
		vector< Vec3 > m_positions;
		vector< Vec3 > m_colors;
		vector< Vec3 > m_normals;
		vector< unsigned int > m_indices;
		Attributes m_attribs;
	};
	
	namespace NodeRenderingHandler
	{
		template< typename NodeContents >
		void handle( RenderingState& state, const NodeContents& contents );
		
		template<>
		inline void handle< PointPtr >( RenderingState& state, const PointPtr& point )
		{
			state.getPositions().push_back( point->getPos() );
			if( state.getAttribs() == COLORS )
			{
				state.getColors().push_back( point->getColor() );
			}
			else
			{
				state.getNormals().push_back( point->getColor() );
			}
		}
	
		template<>
		inline void handle< PointVectorPtr >( RenderingState& state, const PointVectorPtr& points )
		{
			for( PointPtr point : *points )
			{
				handle( state, point );
			}
		}
	
		template<>
		inline void handle< ExtendedPointPtr >( RenderingState& state, const ExtendedPointPtr& point )
		{
			state.getPositions().push_back( point->getPos() );
			state.getColors().push_back( point->getColor() );
			state.getNormals().push_back( point->getNormal() );
		}
	
		template<>
		inline void handle< ExtendedPointVectorPtr >(
			RenderingState& state, const ExtendedPointVectorPtr& points )
		{
			for( ExtendedPointPtr point : *points )
			{
				handle( state, point );
			}
		}
		
		template<>
		inline void handle< shared_ptr< vector< unsigned int > > >(
			RenderingState& state, const shared_ptr< vector< unsigned int > >& points )
		{
			for( unsigned int index : *points )
			{
				state.getIndices().push_back( index );
			}
		}
	}
	
	template< typename NodeContents >
	inline void RenderingState::handleNodeRendering( const NodeContents& contents )
	{
		NodeRenderingHandler::handle< NodeContents >( *this, contents );
	}
}

#endif