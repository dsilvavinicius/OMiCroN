#include "Frustum.h"

namespace model
{
	void Frustum::extractPlanes( const Matrix4f& modelViewProjection, const bool& normalize )
	{
		/*
		// Left clipping plane
		m_planes[ 0 ].a = modelViewProjection._41 + modelViewProjection._11;
		m_planes[ 0 ].b = modelViewProjection._42 + modelViewProjection._12;
		m_planes[ 0 ].c = modelViewProjection._43 + modelViewProjection._13;
		m_planes[ 0 ].d = modelViewProjection._44 + modelViewProjection._14;
		// Right clipping plane
		m_planes[ 1 ].a = modelViewProjection._41 - modelViewProjection._11;
		m_planes[ 1 ].b = modelViewProjection._42 - modelViewProjection._12;
		m_planes[ 1 ].c = modelViewProjection._43 - modelViewProjection._13;
		m_planes[ 1 ].d = modelViewProjection._44 - modelViewProjection._14;
		// Top clipping plane
		m_planes[ 2 ].a = modelViewProjection._41 - modelViewProjection._21;
		m_planes[ 2 ].b = modelViewProjection._42 - modelViewProjection._22;
		m_planes[ 2 ].c = modelViewProjection._43 - modelViewProjection._23;
		m_planes[ 2 ].d = modelViewProjection._44 - modelViewProjection._24;
		// Bottom clipping plane
		m_planes[ 3 ].a = modelViewProjection._41 + modelViewProjection._21;
		m_planes[ 3 ].b = modelViewProjection._42 + modelViewProjection._22;
		m_planes[ 3 ].c = modelViewProjection._43 + modelViewProjection._23;
		m_planes[ 3 ].d = modelViewProjection._44 + modelViewProjection._24;
		// Near clipping plane
		m_planes[ 4 ].a = modelViewProjection._41 + modelViewProjection._31;
		m_planes[ 4 ].b = modelViewProjection._42 + modelViewProjection._32;
		m_planes[ 4 ].c = modelViewProjection._43 + modelViewProjection._33;
		m_planes[ 4 ].d = modelViewProjection._44 + modelViewProjection._34;
		// Far clipping plane
		m_planes[ 5 ].a = modelViewProjection._41 - modelViewProjection._31;
		m_planes[ 5 ].b = modelViewProjection._42 - modelViewProjection._32;
		m_planes[ 5 ].c = modelViewProjection._43 - modelViewProjection._33;
		m_planes[ 5 ].d = modelViewProjection._44 - modelViewProjection._34;
		// Normalize the plane equations, if requested
		if ( normalize == true )
		{
			NormalizePlane( m_planes[0] );
			NormalizePlane( m_planes[1] );
			NormalizePlane( m_planes[2] );
			NormalizePlane( m_planes[3] );
			NormalizePlane( m_planes[4] );
			NormalizePlane( m_planes[5] );
		}*/
	}
}