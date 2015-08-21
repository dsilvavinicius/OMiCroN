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
	inline void TucanoDebugRenderer< Point >::handleNodeRendering( const PointVector& points )
	{
		string debugStr = m_nodeContentHandler( points );
		const Vec3& pos = points[ 0 ]->getPos();
		m_textEffect.render( debugStr, Vector4f( pos.x, pos.y, pos.z, 1.f ), *m_camera );
		
		TucanoRenderingState::handleNodeRendering( points );
	}
}

#endif