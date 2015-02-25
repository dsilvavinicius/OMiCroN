#include "Frustum.h"

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
		Vector4f& coeffs = m_planes[ 0 ]->coeffs();
		coeffs[ 0 ] = modelViewProjection( 3, 0 ) + modelViewProjection( 0, 0 );
		coeffs[ 1 ] = modelViewProjection( 3, 1 ) + modelViewProjection( 0, 1 );
		coeffs[ 2 ] = modelViewProjection( 3, 2 ) + modelViewProjection( 0, 2 );
		coeffs[ 3 ] = modelViewProjection( 3, 3 ) + modelViewProjection( 0, 3 );
		// Right clipping plane
		coeffs = m_planes[ 1 ]->coeffs();
		coeffs[ 0 ] = modelViewProjection( 3, 0 ) - modelViewProjection( 0, 0 );
		coeffs[ 1 ] = modelViewProjection( 3, 1 ) - modelViewProjection( 0, 1 );
		coeffs[ 2 ] = modelViewProjection( 3, 2 ) - modelViewProjection( 0, 2 );
		coeffs[ 3 ] = modelViewProjection( 3, 3 ) - modelViewProjection( 0, 3 );
		// Top clipping plane
		coeffs = m_planes[ 2 ]->coeffs();
		coeffs[ 0 ] = modelViewProjection( 3, 0 ) - modelViewProjection( 1, 0 );
		coeffs[ 1 ] = modelViewProjection( 3, 1 ) - modelViewProjection( 1, 1 );
		coeffs[ 2 ] = modelViewProjection( 3, 2 ) - modelViewProjection( 1, 2 );
		coeffs[ 3 ] = modelViewProjection( 3, 3 ) - modelViewProjection( 1, 3 );
		// Bottom clipping plane
		coeffs = m_planes[ 3 ]->coeffs();
		coeffs[ 0 ] = modelViewProjection( 3, 0 ) + modelViewProjection( 1, 0 );
		coeffs[ 1 ] = modelViewProjection( 3, 1 ) + modelViewProjection( 1, 1 );
		coeffs[ 2 ] = modelViewProjection( 3, 2 ) + modelViewProjection( 1, 2 );
		coeffs[ 3 ] = modelViewProjection( 3, 3 ) + modelViewProjection( 1, 3 );
		// Near clipping plane
		coeffs = m_planes[ 4 ]->coeffs();
		coeffs[ 0 ] = modelViewProjection( 3, 0 ) + modelViewProjection( 2, 0 );
		coeffs[ 1 ] = modelViewProjection( 3, 1 ) + modelViewProjection( 2, 1 );
		coeffs[ 2 ] = modelViewProjection( 3, 2 ) + modelViewProjection( 2, 2 );
		coeffs[ 3 ] = modelViewProjection( 3, 3 ) + modelViewProjection( 2, 3 );
		// Far clipping plane
		coeffs = m_planes[ 5 ]->coeffs();
		coeffs[ 0 ] = modelViewProjection( 3, 0 ) - modelViewProjection( 2, 0 );
		coeffs[ 1 ] = modelViewProjection( 3, 1 ) - modelViewProjection( 2, 1 );
		coeffs[ 2 ] = modelViewProjection( 3, 2 ) - modelViewProjection( 2, 2 );
		coeffs[ 3 ] = modelViewProjection( 3, 3 ) - modelViewProjection( 2, 3 );
		
		// Normalize the plane equations, if requested
		if ( normalize == true )
		{
			m_planes[ 0 ]->normalize();
			m_planes[ 1 ]->normalize();
			m_planes[ 2 ]->normalize();
			m_planes[ 3 ]->normalize();
			m_planes[ 4 ]->normalize();
			m_planes[ 5 ]->normalize();
		}
	}
	
	bool Frustum::isCullable( const Box& box )
	{
		Vector3f boxSizes = box.sizes();
		Vector3f boxMin = box.min();
		Vector3f boxMax = box.max();
		
		// Performs frustum plane dilatation, finds n only, since p is necessary just for detecting intersections points
		// and verify if they are inside or outside the frustum.
		for( int i = 0; i < 6; ++i )
		{
			Plane* plane = m_planes[ i ];
			Vector3f normal = plane->normal();
			float dilatation = abs( boxSizes.dot( normal ) );
			float offset = plane->offset() + dilatation;
			Plane dilatatedPlane( normal, offset );
			
			Vector3f n;
			
			n[ 0 ] = ( normal[ 0 ] < 0 ) ? boxMax[ 0 ] : boxMin[ 0 ];
			n[ 1 ] = ( normal[ 1 ] < 0 ) ? boxMax[ 1 ] : boxMin[ 1 ];
			n[ 2 ] = ( normal[ 2 ] < 0 ) ? boxMax[ 2 ] : boxMin[ 2 ];
			
			if( dilatatedPlane.signedDistance( n ) > 0 ) return true;
		}
		return false;
	}
}