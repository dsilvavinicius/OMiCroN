#include "Frustum.h"
#include <iostream>

namespace model
{
	Frustum::Frustum( const Matrix4f& modelViewProjection )
	{
		
		for( int i = 0; i < 6; ++i )
		{
			m_planes.push_back( new Plane() );
		}
		
		extractPlanes( modelViewProjection, true );
	}
	
	Frustum::~Frustum()
	{
		for( Plane* plane : m_planes )
		{
			delete plane;
		}
	}
	
	void Frustum::update( const Matrix4f& modelViewProjection )
	{
		extractPlanes( modelViewProjection, true );
	}
	
	void Frustum::extractPlanes( const Matrix4f& modelViewProjection, const bool& normalize )
	{
		// Left clipping plane
		Vector4f& leftCoeffs = m_planes[ 0 ]->coeffs();
		leftCoeffs[ 0 ] = modelViewProjection( 3, 0 ) + modelViewProjection( 0, 0 );
		leftCoeffs[ 1 ] = modelViewProjection( 3, 1 ) + modelViewProjection( 0, 1 );
		leftCoeffs[ 2 ] = modelViewProjection( 3, 2 ) + modelViewProjection( 0, 2 );
		leftCoeffs[ 3 ] = modelViewProjection( 3, 3 ) + modelViewProjection( 0, 3 );
		
		// Right clipping plane
		Vector4f& rightCoeffs = m_planes[ 1 ]->coeffs();
		rightCoeffs[ 0 ] = modelViewProjection( 3, 0 ) - modelViewProjection( 0, 0 );
		rightCoeffs[ 1 ] = modelViewProjection( 3, 1 ) - modelViewProjection( 0, 1 );
		rightCoeffs[ 2 ] = modelViewProjection( 3, 2 ) - modelViewProjection( 0, 2 );
		rightCoeffs[ 3 ] = modelViewProjection( 3, 3 ) - modelViewProjection( 0, 3 );
		
		// Top clipping plane
		Vector4f& topCoeffs = m_planes[ 2 ]->coeffs();
		topCoeffs[ 0 ] = modelViewProjection( 3, 0 ) - modelViewProjection( 1, 0 );
		topCoeffs[ 1 ] = modelViewProjection( 3, 1 ) - modelViewProjection( 1, 1 );
		topCoeffs[ 2 ] = modelViewProjection( 3, 2 ) - modelViewProjection( 1, 2 );
		topCoeffs[ 3 ] = modelViewProjection( 3, 3 ) - modelViewProjection( 1, 3 );
		
		// Bottom clipping plane
		Vector4f& botCoeffs = m_planes[ 3 ]->coeffs();
		botCoeffs[ 0 ] = modelViewProjection( 3, 0 ) + modelViewProjection( 1, 0 );
		botCoeffs[ 1 ] = modelViewProjection( 3, 1 ) + modelViewProjection( 1, 1 );
		botCoeffs[ 2 ] = modelViewProjection( 3, 2 ) + modelViewProjection( 1, 2 );
		botCoeffs[ 3 ] = modelViewProjection( 3, 3 ) + modelViewProjection( 1, 3 );
		
		// Near clipping plane
		Vector4f& nearCoeffs = m_planes[ 4 ]->coeffs();
		nearCoeffs[ 0 ] = modelViewProjection( 3, 0 ) + modelViewProjection( 2, 0 );
		nearCoeffs[ 1 ] = modelViewProjection( 3, 1 ) + modelViewProjection( 2, 1 );
		nearCoeffs[ 2 ] = modelViewProjection( 3, 2 ) + modelViewProjection( 2, 2 );
		nearCoeffs[ 3 ] = modelViewProjection( 3, 3 ) + modelViewProjection( 2, 3 );
		
		// Far clipping plane
		Vector4f& farCoeffs = m_planes[ 5 ]->coeffs();
		farCoeffs[ 0 ] = modelViewProjection( 3, 0 ) - modelViewProjection( 2, 0 );
		farCoeffs[ 1 ] = modelViewProjection( 3, 1 ) - modelViewProjection( 2, 1 );
		farCoeffs[ 2 ] = modelViewProjection( 3, 2 ) - modelViewProjection( 2, 2 );
		farCoeffs[ 3 ] = modelViewProjection( 3, 3 ) - modelViewProjection( 2, 3 );
		
		// Normalize the plane equations, if requested
		if ( normalize )
		{
			m_planes[ 0 ]->normalize();
			m_planes[ 1 ]->normalize();
			m_planes[ 2 ]->normalize();
			m_planes[ 3 ]->normalize();
			m_planes[ 4 ]->normalize();
			m_planes[ 5 ]->normalize();
		}
		
		//
		cout << "Frustum planes" << endl;
		for( Plane* plane : m_planes )
		{
			cout << "Normal:" << plane->normal() << ", offset: " << plane->offset() << endl;
		}
		//
	}
}