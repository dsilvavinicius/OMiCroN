#ifndef TUCANO_DEBUG_RENDERER_H
#define TUCANO_DEBUG_RENDERER_H

#include "TucanoRenderingState.h"

namespace model
{
	/** Tucano renderer wich also reports debug data on rendering. */
	template< typename Point >
	class TucanoDebugRenderer
	: public TucanoRenderingState
	{
		using PointVector = vector< shared_ptr< Point > >;
		
	public:
		/** @param pointHandler is a function that takes the contents of a node, generate debug info and renders it. */
		TucanoDebugRenderer( const function< void( const PointVector& ) >& pointHandler, Camera* camera, Camera* lightCam ,
							 Mesh* mesh, const Attributes& attribs, const string& shaderPath, const int& jfpbrFrameskip = 1,
					   const Effect& effect = PHONG );
		
	protected:
		/** This implementation does the same as TucanoRenderingState plus change opengl matrices to camera's matrices
		 * in order to render texts later. */
		virtual void setupRendering();
		
		/** This implementation does the same as TucanoRenderingState plus restore the matrices changed by setupRendering
		 * to original state. */
		virtual void afterRendering();
		
		virtual void handleNodeRendering( const PointVector& points );
		
		function< void( const PointVector& ) > m_pointHandler;
	};
	
	template< typename Point >
	TucanoDebugRenderer< Point >::TucanoDebugRenderer( const function< void( const PointVector& ) >& pointHandler,
													   Camera* camera, Camera* lightCam , Mesh* mesh,
													const Attributes& attribs, const string& shaderPath,
													const int& jfpbrFrameskip, const Effect& effect )
	: TucanoRenderingState( camera, lightCam, mesh, attribs, shaderPath, jfpbrFrameskip, effect )
	{
		m_pointHandler = pointHandler;
	}
	
	template< typename Point >
	void TucanoDebugRenderer< Point >::setupRendering()
	{
		TucanoRenderingState::setupRendering();
		
		glColor3f( 0.f, 0.f, 0.f );
		
		GLdouble matrix[ 16 ];
		m_camera->getViewMatrix( matrix );
		
		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadMatrixd( matrix );
		
		m_camera->getProjectionMatrix( matrix );
		glMatrixMode( GL_PROJECTION );
		glPushMatrix();
		glLoadMatrixd( matrix );
	}
	
	template< typename Point >
	void TucanoDebugRenderer< Point >::afterRendering()
	{
		glMatrixMode( GL_PROJECTION );
		glPopMatrix();
		
		glMatrixMode( GL_MODELVIEW );
		glPopMatrix();
		
		TucanoRenderingState::afterRendering();
	}
	
	template< typename Point >
	inline void TucanoDebugRenderer< Point >::handleNodeRendering( const PointVector& points )
	{
		m_pointHandler( points );
		
		TucanoRenderingState::handleNodeRendering( points );
	}
}

#endif