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
	template< typename Vec3, typename Float >
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
	
		/** Clear all attrib vectors. */
		void clearAttribs();
		
	protected:
		vector< Vec3 > m_positions;
		vector< Vec3 > m_colors;
		vector< Vec3 > m_normals;
		vector< unsigned int > m_indices;
		Attributes m_attribs;
	};
	
	template< typename Vec3, typename Float >
	RenderingState< Vec3, Float >::RenderingState( const Attributes& attribs )
	: m_attribs( attribs ) {}
	
	template< typename Vec3, typename Float >
	void RenderingState< Vec3, Float >::clearAttribs()
	{
		m_positions.clear();
		m_colors.clear();
		m_normals.clear();
		m_indices.clear();
	}
	
	namespace NodeRenderingHandler
	{
		template< typename Vec3, typename Float, typename NodeContents >
		void handle( RenderingState< Vec3, Float >& state, const NodeContents& contents );
		
		template<>
		inline void handle< vec3, float, PointPtr< float, vec3 > >( RenderingState< vec3, float >& state,
																	const PointPtr< float, vec3 >& point )
		{
			state.getPositions().push_back( *point->getPos() );
			state.getColors().push_back( *point->getColor() );
		}
	
		template<>
		inline void handle< vec3, float, PointVectorPtr< float, vec3 > >( RenderingState< vec3, float >& state,
																	const PointVectorPtr< float, vec3 >& points )
		{
			for( PointPtr< float, vec3 > point : *points )
			{
				state.getPositions().push_back( *point->getPos() );
				state.getColors().push_back( *point->getColor() );
			}
		}
	
		template<>
		inline void handle< vec3, float, ExtendedPointPtr< float, vec3 > >( RenderingState< vec3, float >& state,
																	const ExtendedPointPtr< float, vec3 >& point )
		{
			state.getPositions().push_back( *point->getPos() );
			state.getColors().push_back( *point->getColor() );
			state.getNormals().push_back( *point->getNormal() );
		}
	
		template<>
		inline void handle< vec3, float, ExtendedPointVectorPtr< float, vec3 > >(
			RenderingState< vec3, float >& state, const ExtendedPointVectorPtr< float, vec3 >& points )
		{
			for( ExtendedPointPtr< float, vec3 > point : *points )
			{
				state.getPositions().push_back( *point->getPos() );
				state.getColors().push_back( *point->getColor() );
				state.getNormals().push_back( *point->getNormal() );
			}
		}
		
		template<>
		inline void handle< vec3, float, shared_ptr< vector< unsigned int > > >(
			RenderingState< vec3, float >& state, const shared_ptr< vector< unsigned int > >& points )
		{
			for( unsigned int index : *points )
			{
				state.getIndices().push_back( index );
			}
		}
	}
	
	template< typename Vec3, typename Float >
	template< typename NodeContents >
	inline void RenderingState< Vec3, Float >::handleNodeRendering( const NodeContents& contents )
	{
		NodeRenderingHandler::handle< Vec3, Float, NodeContents >( *this, contents );
	}
}

#endif