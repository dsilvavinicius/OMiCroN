#ifndef INDEXED_TUCANO_RENDERER_H
#define INDEXED_TUCANO_RENDERER_H

#include "TucanoRenderingState.h"

namespace model
{
	template< typename Vec3, typename Float, typename Point >
	struct MeshInitializer;
	
	/** Tucano renderer that sends all points to device at initialization time. After that, just sends indices to indicate
	 * 	which points should be rendered. */
	template< typename Vec3, typename Float, typename Point >
	class IndexedTucanoRenderer
	: public TucanoRenderingState< Vec3, Float >
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using TucanoRenderingState = model::TucanoRenderingState< Vec3, Float >;
		using MeshInitializer = model::MeshInitializer< Vec3, Float, Point >;
		
	public:
		/** This ctor sends all points to the device. */
		IndexedTucanoRenderer( const PointVector& points, Trackball* camTrackball, Trackball* lightTrackball, Mesh* mesh,
							   const Attributes& attribs, const string& shaderPath,
						 const typename TucanoRenderingState::Effect& effect = TucanoRenderingState::PHONG );
		
		unsigned int render();
		
		friend MeshInitializer;
	};
	
	template< typename Vec3, typename Float, typename Point >
	IndexedTucanoRenderer< Vec3, Float, Point >::IndexedTucanoRenderer(
		const PointVector& points, Trackball* camTrackball, Trackball* lightTrackball, Mesh* mesh,
		const Attributes& attribs, const string& shaderPath, const typename TucanoRenderingState::Effect& effect )
	: TucanoRenderingState( camTrackball, lightTrackball, mesh, attribs, shaderPath, effect )
	{
		cout << "points passed to indexed renderer: " << points.size() << endl << endl;  
		
		MeshInitializer::initMesh( points, *this );
	}
	
	template< typename Vec3, typename Float, typename Point >
	inline unsigned int IndexedTucanoRenderer< Vec3, Float, Point >::render()
	{
		TucanoRenderingState::m_mesh->loadIndices( TucanoRenderingState::m_indices );
		
		switch( TucanoRenderingState::m_effect )
		{
			case TucanoRenderingState::PHONG: TucanoRenderingState::m_phong->render( *TucanoRenderingState::m_mesh,
				*TucanoRenderingState::m_camTrackball, *TucanoRenderingState::m_lightTrackball ); break;
			case TucanoRenderingState::JUMP_FLOODING: TucanoRenderingState::m_jfpbr->render( TucanoRenderingState::m_mesh,
				TucanoRenderingState::m_camTrackball, TucanoRenderingState::m_lightTrackball, true ); break;
		}
		
		return TucanoRenderingState::m_positions.size();
	}
	
	/** Initializes the mesh in IndexedTucanoRenderer. */
	template< typename Vec3, typename Float, typename Point >
	struct MeshInitializer {};
	
	/** Specialization for Point type. */
	template< typename Vec3, typename Float >
	struct MeshInitializer< Vec3, Float, Point< Float, Vec3 > >
	{
		using Point = model::Point< Float, Vec3 >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using IndexedTucanoRenderer = model::IndexedTucanoRenderer< Vec3, Float, Point >;
		
	public:
		static void initMesh( const PointVector& points, IndexedTucanoRenderer& renderer )
		{
			int nPoints = renderer.m_positions.size();
			vector< Vector4f > positions( nPoints );
			vector< Vector4f > colors;
			vector< Vector3f > normals;
			
			bool hasColors = renderer.m_attribs & Attributes::COLORS;
			bool hasNormals = renderer.m_attribs & Attributes::NORMALS;
			bool hasColorsOrNormals = hasColors || hasNormals;
			
			assert( !( hasColors && hasNormals ) && "Models using Point type cannot have both colors and normals." );
			assert( hasColorsOrNormals && "Model does not have colors nor normals." );
			
			if( hasColors )
			{
				colors.resize( nPoints );
			}
			else
			{
				normals.resize( nPoints );
			}
			
			for( int i = 0; i < nPoints; ++i )
			{
				PointPtr point = points[ i ];
				
				Vec3 pos = *point->getPos();
				positions[ i ] = Vector4f( pos.x, pos.y, pos.z, 1.f );
				
				if( hasColors )
				{
					Vec3 color = *point->getColor();
					colors[ i ] = Vector4f( color.x, color.y, color.z, 1.f );
				}
				else
				{
					Vec3 normal = *point->getNormal();
					normals[ i ] = Vector3f( normal.x, normal.y, normal.z );
				}
			}
			
			renderer.m_mesh->loadVertices( positions );
			if( hasColors )
			{
				renderer.m_mesh->loadColors( colors );
			}
			else
			{
				renderer.m_mesh->loadNormals( normals );
			}
		}
	};
	
	/** Specialization for ExtendedPoint type. */
	template< typename Vec3, typename Float >
	struct MeshInitializer< Vec3, Float, ExtendedPoint< Float, Vec3 > >
	{
		using Point = model::ExtendedPoint< Float, Vec3 >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		using IndexedTucanoRenderer = model::IndexedTucanoRenderer< Vec3, Float, Point >;
		
	public:
		static void initMesh( const PointVector& points, IndexedTucanoRenderer& renderer )
		{
			cout << "Init ExtendedPoints." << endl << endl;
			
			int nPoints = renderer.m_positions.size();
			vector< Vector4f > positions( nPoints );
			vector< Vector4f > colors;
			vector< Vector3f > normals;
			
			bool hasColors = renderer.m_attribs & Attributes::COLORS;
			bool hasNormals = renderer.m_attribs & Attributes::NORMALS;
			
			if( hasColors )
			{
				colors.resize( nPoints );
			}
			if( hasNormals )
			{
				normals.resize( nPoints );
			}
			
			for( int i = 0; i < nPoints; ++i )
			{
				PointPtr point = points[ i ];
				
				Vec3 pos = *point->getPos();
				positions[ i ] = Vector4f( pos.x, pos.y, pos.z, 1.f );
				
				if( hasColors )
				{
					Vec3 color = *point->getColor();
					colors[ i ] = Vector4f( color.x, color.y, color.z, 1.f );
				}
				if( hasNormals )
				{
					Vec3 normal = *point->getNormal();
					normals[ i ] = Vector3f( normal.x, normal.y, normal.z );
				}
			}
			
			renderer.m_mesh->loadVertices( positions );
			if( hasColors )
			{
				renderer.m_mesh->loadColors( colors );
			}
			if( hasNormals )
			{
				renderer.m_mesh->loadNormals( normals );
			}
		}
	};
}

#endif