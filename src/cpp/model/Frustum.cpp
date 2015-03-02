#include "Frustum.h"
#include <iostream>

namespace model
{
	Frustum::Frustum( const Matrix4f& modelView, const Matrix4f& projection )
	{
		
		for( int i = 0; i < 6; ++i )
		{
			m_planes.push_back( new Plane() );
		}
		
		extractPlanes( modelView, projection, true );
	}
	
	Frustum::~Frustum()
	{
		for( Plane* plane : m_planes )
		{
			delete plane;
		}
	}
	
	void Frustum::update( const Matrix4f& modelView, const Matrix4f& projection )
	{
		extractPlanes( modelView, projection, true );
	}
	
	void Frustum::extractPlanes( const Matrix4f& modelView, const Matrix4f& projection, const bool& normalize )
	{
		// It seems that the d plane coefficient has wrong sign in the original algorithm, so all d coefficients are being
		// multiplied by -1 in this implementation.
		
		// Left clipping plane
		Vector4f& leftCoeffs = m_planes[ 0 ]->coeffs();
		leftCoeffs[ 0 ] = projection( 3, 0 ) + projection( 0, 0 );
		leftCoeffs[ 1 ] = projection( 3, 1 ) + projection( 0, 1 );
		leftCoeffs[ 2 ] = projection( 3, 2 ) + projection( 0, 2 );
		leftCoeffs[ 3 ] = -( projection( 3, 3 ) + projection( 0, 3 ) );
		
		// Right clipping plane
		Vector4f& rightCoeffs = m_planes[ 1 ]->coeffs();
		rightCoeffs[ 0 ] = projection( 3, 0 ) - projection( 0, 0 );
		rightCoeffs[ 1 ] = projection( 3, 1 ) - projection( 0, 1 );
		rightCoeffs[ 2 ] = projection( 3, 2 ) - projection( 0, 2 );
		rightCoeffs[ 3 ] = -( projection( 3, 3 ) - projection( 0, 3 ) );
		
		// Top clipping plane
		Vector4f& topCoeffs = m_planes[ 2 ]->coeffs();
		topCoeffs[ 0 ] = projection( 3, 0 ) - projection( 1, 0 );
		topCoeffs[ 1 ] = projection( 3, 1 ) - projection( 1, 1 );
		topCoeffs[ 2 ] = projection( 3, 2 ) - projection( 1, 2 );
		topCoeffs[ 3 ] = -( projection( 3, 3 ) - projection( 1, 3 ) );
		
		// Bottom clipping plane
		Vector4f& botCoeffs = m_planes[ 3 ]->coeffs();
		botCoeffs[ 0 ] = projection( 3, 0 ) + projection( 1, 0 );
		botCoeffs[ 1 ] = projection( 3, 1 ) + projection( 1, 1 );
		botCoeffs[ 2 ] = projection( 3, 2 ) + projection( 1, 2 );
		botCoeffs[ 3 ] = -( projection( 3, 3 ) + projection( 1, 3 ) );
		
		// Near clipping plane
		Vector4f& nearCoeffs = m_planes[ 4 ]->coeffs();
		nearCoeffs[ 0 ] = projection( 3, 0 ) + projection( 2, 0 );
		nearCoeffs[ 1 ] = projection( 3, 1 ) + projection( 2, 1 );
		nearCoeffs[ 2 ] = projection( 3, 2 ) + projection( 2, 2 );
		nearCoeffs[ 3 ] = -( projection( 3, 3 ) + projection( 2, 3 ) );
		
		// Far clipping plane
		Vector4f& farCoeffs = m_planes[ 5 ]->coeffs();
		farCoeffs[ 0 ] = projection( 3, 0 ) - projection( 2, 0 );
		farCoeffs[ 1 ] = projection( 3, 1 ) - projection( 2, 1 );
		farCoeffs[ 2 ] = projection( 3, 2 ) - projection( 2, 2 );
		farCoeffs[ 3 ] = -( projection( 3, 3 ) - projection( 2, 3 ) );
		
		if ( normalize )
		{
			for( Plane* plane : m_planes )
			{
				plane->normalize();
			}
		}
		
		Matrix4f inverse = modelView.inverse();
		Affine3f toWorld( inverse );
		
		for( Plane* plane : m_planes )
		{
			plane->transform( toWorld );
		}
		
		//
		cout << "Frustum planes:" << endl << m_planes;
		//
	}
}