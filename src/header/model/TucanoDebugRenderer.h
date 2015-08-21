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
		
		virtual void setupRendering() override;
		
	protected:
		virtual void handleNodeRendering( const PointVector& points ) override;
		
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
		
		// Alpha is needed for text rendering
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		
		m_textEffect.setColor( Vector4f( 0.f, 0.f, 0.f, 1.f ) );
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