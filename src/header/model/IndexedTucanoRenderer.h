#ifndef INDEXED_TUCANO_RENDERER_H
#define INDEXED_TUCANO_RENDERER_H

#include "TucanoRenderingState.h"

namespace model
{
	template< typename Point >
	struct MeshInitializer;
	
	/** Tucano renderer that sends all points to device at initialization time. After that, just sends indices to indicate
	 * 	which points should be rendered. */
	template< typename Point >
	class IndexedTucanoRenderer
	: public TucanoRenderingState
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		using MeshInitializer = model::MeshInitializer< Point >;
		
	public:
		/** This ctor sends all points to the device. */
		IndexedTucanoRenderer( const PointVector& points, Camera* camera, Camera* lightCamera, Mesh* mesh,
							   const Attributes& attribs, const string& shaderPath, const int& jfpbrFrameskip = 1,
						 const typename TucanoRenderingState::Effect& effect = TucanoRenderingState::PHONG );
		
		unsigned int render();
		
		friend MeshInitializer;
	};
	
	template< typename Point >
	IndexedTucanoRenderer< Point >::IndexedTucanoRenderer(
		const PointVector& points, Camera* camera, Camera* lightCamera, Mesh* mesh,
		const Attributes& attribs, const string& shaderPath, const int& jfpbrFrameskip,
		const typename TucanoRenderingState::Effect& effect )
	: TucanoRenderingState( camera, lightCamera, mesh, attribs, shaderPath, jfpbrFrameskip, effect )
	{
		MeshInitializer::initMesh( points, *this );
	}
	
	template< typename Point >
	inline unsigned int IndexedTucanoRenderer< Point >::render()
	{
		++TucanoRenderingState::m_nFrames;
		
		TucanoRenderingState::m_mesh->loadIndices( TucanoRenderingState::m_indices );
		
		switch( TucanoRenderingState::m_effect )
		{
			case TucanoRenderingState::PHONG: TucanoRenderingState::m_phong->render( *TucanoRenderingState::m_mesh,
				*TucanoRenderingState::m_camera, *TucanoRenderingState::m_lightCamera ); break;
			case TucanoRenderingState::JUMP_FLOODING:
			{
				bool newFrame = TucanoRenderingState::m_nFrames % TucanoRenderingState::m_jfpbrFrameskip == 0;
				TucanoRenderingState::m_jfpbr->render( TucanoRenderingState::m_mesh, TucanoRenderingState::m_camera,
													   TucanoRenderingState::m_lightCamera, newFrame );
				break;
			}
		}
		
		return TucanoRenderingState::m_indices.size();
	}
	
	/** Initializes the mesh in IndexedTucanoRenderer. */
	template< typename Point >
	struct MeshInitializer {};
	
	/** Specialization for Point type. */
	template<>
	struct MeshInitializer< Point >
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		using IndexedTucanoRenderer = model::IndexedTucanoRenderer< Point >;
		
	public:
		static void initMesh( const PointVector& points, IndexedTucanoRenderer& renderer )
		{
			unsigned long nPoints = points.size();
			
			cout << "Initializing renderer with " << nPoints << " points." << endl;
			
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
			
			for( unsigned long i = 0; i < nPoints; ++i )
			{
				PointPtr point = points[ i ];
				
				const Vec3& pos = point->getPos();
				positions[ i ] = Vector4f( pos.x, pos.y, pos.z, 1.f );
				
				if( hasColors )
				{
					const Vec3& color = point->getColor();
					colors[ i ] = Vector4f( color.x, color.y, color.z, 1.f );
				}
				else
				{
					const Vec3& normal = point->getColor();
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
	template<>
	struct MeshInitializer< ExtendedPoint >
	{
		using Point = model::ExtendedPoint;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		using IndexedTucanoRenderer = model::IndexedTucanoRenderer< Point >;
		
	public:
		static void initMesh( const PointVector& points, IndexedTucanoRenderer& renderer )
		{
			unsigned long nPoints = points.size();
			cout << "Initializing renderer with " << nPoints << " points." << endl;
			
			vector< Vector4f > positions( nPoints );
			vector< Vector4f > colors;
			vector< Vector3f > normals;
			
			bool hasColors = renderer.m_attribs & Attributes::COLORS;
			bool hasNormals = renderer.m_attribs & Attributes::NORMALS;
			
			cout << "Has colors: " << hasColors << " has normals: " << hasNormals << endl;
			
			if( hasColors )
			{
				colors.resize( nPoints );
			}
			if( hasNormals )
			{
				normals.resize( nPoints );
			}
			
			cout << "Renderer position, color and normal vectors instantiated." << endl;
			
			for( unsigned long i = 0; i < nPoints; ++i )
			{
				PointPtr point = points[ i ];
				
				const Vec3& pos = point->getPos();
				positions[ i ] = Vector4f( pos.x, pos.y, pos.z, 1.f );
				
				if( hasColors )
				{
					const Vec3& color = point->getColor();
					colors[ i ] = Vector4f( color.x, color.y, color.z, 1.f );
				}
				if( hasNormals )
				{
					const Vec3& normal = point->getNormal();
					normals[ i ] = Vector3f( normal.x, normal.y, normal.z );
				}
			}
			
			cout << "Renderer position, color and normal vectors populated." << endl;
			
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