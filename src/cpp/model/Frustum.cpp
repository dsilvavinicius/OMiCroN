#include "Frustum.h"
#include <iostream>

namespace model
{
	Frustum::Frustum( const Matrix4f& mvp )
	{
		
		for( int i = 0; i < 6; ++i )
		{
			m_planes.push_back( new Plane() );
		}
		
		extractPlanes( mvp, true );
	}
	
	Frustum::~Frustum()
	{
		for( Plane* plane : m_planes )
		{
			delete plane;
		}
	}
	
	void Frustum::update( const Matrix4f& mvp )
	{
		extractPlanes( mvp, true );
	}
	
	void Frustum::extractPlanes( const Matrix4f& mvp, const bool& normalize )
	{
		// It seems that the d plane coefficient has wrong sign in the original algorithm, so all d coefficients are being
		// multiplied by -1 in this implementation.
		
		// Left clipping plane
		Vector4f& leftCoeffs = m_planes[ 0 ]->coeffs();
		leftCoeffs[ 0 ] = -( mvp( 3, 0 ) + mvp( 0, 0 ) );
		leftCoeffs[ 1 ] = -( mvp( 3, 1 ) + mvp( 0, 1 ) );
		leftCoeffs[ 2 ] = -( mvp( 3, 2 ) + mvp( 0, 2 ) );
		leftCoeffs[ 3 ] = -( mvp( 3, 3 ) + mvp( 0, 3 ) );
		
		// Right clipping plane
		Vector4f& rightCoeffs = m_planes[ 1 ]->coeffs();
		rightCoeffs[ 0 ] = -( mvp( 3, 0 ) - mvp( 0, 0 ) );
		rightCoeffs[ 1 ] = -( mvp( 3, 1 ) - mvp( 0, 1 ) );
		rightCoeffs[ 2 ] = -( mvp( 3, 2 ) - mvp( 0, 2 ) );
		rightCoeffs[ 3 ] = -( mvp( 3, 3 ) - mvp( 0, 3 ) );
		
		// Top clipping plane
		Vector4f& topCoeffs = m_planes[ 2 ]->coeffs();
		topCoeffs[ 0 ] = -( mvp( 3, 0 ) - mvp( 1, 0 ) );
		topCoeffs[ 1 ] = -( mvp( 3, 1 ) - mvp( 1, 1 ) );
		topCoeffs[ 2 ] = -( mvp( 3, 2 ) - mvp( 1, 2 ) );
		topCoeffs[ 3 ] = -( mvp( 3, 3 ) - mvp( 1, 3 ) );
		
		// Bottom clipping plane
		Vector4f& botCoeffs = m_planes[ 3 ]->coeffs();
		botCoeffs[ 0 ] = -( mvp( 3, 0 ) + mvp( 1, 0 ) );
		botCoeffs[ 1 ] = -( mvp( 3, 1 ) + mvp( 1, 1 ) );
		botCoeffs[ 2 ] = -( mvp( 3, 2 ) + mvp( 1, 2 ) );
		botCoeffs[ 3 ] = -( mvp( 3, 3 ) + mvp( 1, 3 ) );
		
		// Near clipping plane
		Vector4f& nearCoeffs = m_planes[ 4 ]->coeffs();
		nearCoeffs[ 0 ] = -( mvp( 3, 0 ) + mvp( 2, 0 ) );
		nearCoeffs[ 1 ] = -( mvp( 3, 1 ) + mvp( 2, 1 ) );
		nearCoeffs[ 2 ] = -( mvp( 3, 2 ) + mvp( 2, 2 ) );
		nearCoeffs[ 3 ] = -( mvp( 3, 3 ) + mvp( 2, 3 ) );
		
		// Far clipping plane
		Vector4f& farCoeffs = m_planes[ 5 ]->coeffs();
		farCoeffs[ 0 ] = -( mvp( 3, 0 ) - mvp( 2, 0 ) );
		farCoeffs[ 1 ] = -( mvp( 3, 1 ) - mvp( 2, 1 ) );
		farCoeffs[ 2 ] = -( mvp( 3, 2 ) - mvp( 2, 2 ) );
		farCoeffs[ 3 ] = -( mvp( 3, 3 ) - mvp( 2, 3 ) );
		
		if ( normalize )
		{
			for( Plane* plane : m_planes )
			{
				plane->normalize();
			}
		}
	}
	
	ostream& operator<<( ostream& out, const Frustum& f )
	{
		cout << "Frustum planes:" << endl << endl;
		for( int i = 0; i < 6; ++i )
		{
			switch( i )
			{
				case 0: cout << "Left: "; break;
				case 1: cout << "Right: "; break;
				case 2: cout << "Top: "; break;
				case 3: cout << "Bottom: "; break;
				case 4: cout << "Near: "; break;
				case 5: cout << "Far: "; break;
			}
			cout << *f.m_planes[ i ] << endl << endl;
		}
		cout << "End of frustum planes." << endl;
		
		return out;
	}
}