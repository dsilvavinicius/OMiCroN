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
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		
	public:
		TucanoDebugRenderer( Camera* camera, Camera* lightCam, Mesh* mesh, const Attributes& attribs,
							 const string& shaderPath, const int& jfpbrFrameskip = 1, const Effect& effect = PHONG );
		
		virtual void setupRendering() override;
		
	protected:
		/** Handles node rendering, also passing a debug string to show in the final image. */
		virtual void handleNodeRendering( const PointVector& points, const string& debugStr );
		
		TextEffect m_textEffect;
	};
	
	template< typename Point >
	TucanoDebugRenderer< Point >::TucanoDebugRenderer( Camera* camera, Camera* lightCam , Mesh* mesh,
													   const Attributes& attribs, const string& shaderPath,
													const int& jfpbrFrameskip, const Effect& effect )
	: TucanoRenderingState( camera, lightCam, mesh, attribs, shaderPath, jfpbrFrameskip, effect )
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
		
		glEnable( GL_DEPTH_TEST );
		
		m_textEffect.setColor( Vector4f( 0.f, 0.f, 0.f, 1.f ) );
	}
	
	template< typename Point >
	inline void TucanoDebugRenderer< Point >::handleNodeRendering( const PointVector& points, const string& debugStr )
	{
		const Vec3& pos = points[ 0 ]->getPos();
		m_textEffect.render( debugStr, Vector4f( pos.x, pos.y, pos.z, 1.f ), *m_camera );
		
		TucanoRenderingState::handleNodeRendering( points );
	}
}

#endif