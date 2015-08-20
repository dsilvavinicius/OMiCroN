#ifndef TUCANO_DEBUG_RENDERER_H
#define TUCANO_DEBUG_RENDERER_H

#include "TucanoRenderingState.h"
#include "TextEffect.h"

namespace model
{
	/** Tucano renderer wich also reports debug data on rendering. */
	template< typename Point >
	class TucanoDebugRenderer
	: public TucanoRenderingState
	{
		using PointVector = vector< shared_ptr< Point > >;
		
	public:
		TucanoDebugRenderer( const function< string( PointVector ) >& nodeContentHandler, Camera* camera, Camera* lightCam,
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
		
		TextEffect m_textEffect;
		
		function< string( PointVector ) > m_nodeContentHandler;
	};
	
	template< typename Point >
	TucanoDebugRenderer< Point >::TucanoDebugRenderer( const function< string( PointVector ) >& nodeContentHandler,
													   Camera* camera, Camera* lightCam , Mesh* mesh,
													const Attributes& attribs, const string& shaderPath,
													const int& jfpbrFrameskip, const Effect& effect )
	: TucanoRenderingState( camera, lightCam, mesh, attribs, shaderPath, jfpbrFrameskip, effect ),
	m_nodeContentHandler( nodeContentHandler )
	{
		m_textEffect.initialize( shaderPath + "/../Inconsolata.otf" );
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
		string debugStr = m_nodeContentHandler( points );
		const Vec3& pos = points[ 0 ]->getPos();
		m_textEffect.render( debugStr, Vector4f( pos.x, pos.y, pos.z, 1.f ), *m_camera );
		
		TucanoRenderingState::handleNodeRendering( points );
	}
}

#endif