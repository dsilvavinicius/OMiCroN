#ifndef RENDERING_STATE_H
#define RENDERING_STATE_H

#include <stdexcept>
#include <vector>
#include <QGLPainter>
#include "ExtendedPoint.h"

using namespace std;

namespace model
{
	enum Attributes
	{
		COLORS = 0x1,
		NORMALS = 0x2,
		COLORS_AND_NORMALS = 0x3
	};
	
	/** Rendering related data used while traversing octree. USAGE: call setPainter() when the painter is known in the
	 * rendering loop and call several handleNodeRendering() afterwards to indicate that the contents of octree nodes should be
	 * rendered. After all nodes are issued for rendering, call render() to render them all.
	 * @param Vec3 is the type for 3-dimensional vector. */
	template< typename Vec3 >
	class RenderingState
	{
	public:
		RenderingState( const Attributes& attribs );
		
		/** Renders the current state. Should be called at the end of the traversal, when all rendered nodes have
		 * been already handled.
		 * @returns the number of rendered points. */
		virtual unsigned int render() = 0;
		
		/** Indicates that the node contents passed should be rendered. A static method was used to overcome C++ limitation of
		* class member specializations.
		* @param NodeContents is the type of nodes' contents. The octree is aware of this type.
		*/
		template< typename NodeContents >
		void handleNodeRendering( RenderingState< Vec3 >& state, const NodeContents& contents );
		
		QGLPainter* getPainter() { return m_painter; };
		
		/** This method should be called on the starting of the rendering loop, when the painter is known. */
		void setPainter( QGLPainter* painter ) { m_painter = painter; }
		
		vector< Vec3 >& getPositions() { return m_positions; };
		vector< Vec3 >& getColors() { return m_colors; };
		vector< Vec3 >& getNormals() { return m_normals; };
	
	protected:
		QGLPainter* m_painter;
		vector< Vec3 > m_positions;
		vector< Vec3 > m_colors;
		vector< Vec3 > m_normals;
		Attributes m_attribs;
	};
	
	template< typename Vec3 >
	RenderingState< Vec3 >::RenderingState( const Attributes& attribs )
	: m_attribs( attribs ) {}
	
	namespace NodeRenderingHandler
	{
		template< typename Vec3, typename NodeContents >
		void handle( RenderingState< Vec3 >& state, const NodeContents& contents );
		
		template<>
		inline void handle< vec3, PointPtr< float, vec3 > >( RenderingState< vec3 >& state,
															 const PointPtr< float, vec3 >& point )
		{
			state.getPositions().push_back( *point->getPos() );
			state.getColors().push_back( *point->getColor() );
		}
	
		template<>
		inline void handle< vec3, PointVectorPtr< float, vec3 > >( RenderingState< vec3 >& state,
																   const PointVectorPtr< float, vec3 >& points )
		{
			for( PointPtr< float, vec3 > point : *points )
			{
				state.getPositions().push_back( *point->getPos() );
				state.getColors().push_back( *point->getColor() );
			}
		}
	
		template<>
		inline void handle< vec3, ExtendedPointPtr< float, vec3 > >( RenderingState< vec3 >& state,
																		 const ExtendedPointPtr< float, vec3 >& point )
		{
			state.getPositions().push_back( *point->getPos() );
			state.getColors().push_back( *point->getColor() );
			state.getNormals().push_back( *point->getNormal() );
		}
	
		template<>
		inline void handle< vec3, ExtendedPointVectorPtr< float, vec3 > >( RenderingState< vec3 >& state,
																		const ExtendedPointVectorPtr< float, vec3 >& points )
		{
			for( ExtendedPointPtr< float, vec3 > point : *points )
			{
				state.getPositions().push_back( *point->getPos() );
				state.getColors().push_back( *point->getColor() );
				state.getNormals().push_back( *point->getNormal() );
			}
		}
	}
	
	template< typename Vec3 >
	template< typename NodeContents >
	void RenderingState< Vec3 >::handleNodeRendering( RenderingState& state, const NodeContents& contents )
	{
		NodeRenderingHandler::handle< Vec3, NodeContents >( state, contents );
	}
}

#endif