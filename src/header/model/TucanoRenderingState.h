#ifndef TUCANO_RENDERING_STATE_H
#define TUCANO_RENDERING_STATE_H

#include "RenderingState.h"
#include "Frustum.h"

namespace model
{
	/** RenderingState using Tucano library ( @link http://terra.lcg.ufrj.br/tucano/ ). */
	template< typename Vec3, typename Float >
	class TucanoRenderingState
	: public RenderingState< Vec3, Float >
	{
		using RenderingState = model::RenderingState< Vec3, Float >;
	public:
		/** @param vp is the view-projection matrix.
		 *	@param attribs is the vertex attributes setup flag. */
		TucanoRenderingState( const Matrix4f& vp, const Attributes& attribs );
		
		/** Updates the frustum after changes on camera.
		 *	@param vp is the view-projection matrix. */
		void updateFrustum( const Matrix4f &vp );
		
		virtual unsigned int render();
		
		virtual bool isCullable( const pair< Vec3, Vec3 >& box ) const;
		
		virtual bool isRenderable( const pair< Vec3, Vec3 >& box, const Float& projThresh ) const;
	
	private:
		Frutum* m_frustum;
	};
	
	template< typename Vec3, typename Float >
	TucanoRenderingState< Vec3, Float >::TucanoRenderingState( const Matrix4f& vp, const Attributes& attribs )
	: RenderingState( attribs )
	{
		m_frustum = new Frustum( vp );
	}
	
	template< typename Vec3, typename Float >
	void TucanoRenderingState< Vec3, Float >::updateFrustum( const Matrix4f &vp )
	{
		m_frustum->update( vp );
	}
	
	template< typename Vec3, typename Float >
	unsigned int TucanoRenderingState< Vec3, Float >::render()
	{
		
	}
	
	template< typename Vec3, typename Float >
	bool TucanoRenderingState< Vec3, Float >::isCullable()
	{
		
	}
	
	template< typename Vec3, typename Float >
	bool TucanoRenderingState< Vec3, Float >::isRenderable()
	{
		
	}
}

#endif