#ifndef GPU_LOADER_THREAD_H
#define GPU_LOADER_THREAD_H

#include <QThread>
#include <utils/qtfreecamerawidget.hpp>
#include "TbbAllocator.h"

namespace model
{
	template< typename Point, typename Alloc = TbbAllocator< Point > >
	class GpuLoaderThread
	: public QThread
	{
		Q_OBJECT
	
	public:
		using PointPtr = shared_ptr< Point >;
		using Node = O1OctreeNode< PointPtr >;
		using Siblings = Array< Node >;
		using NodePtrList = list< Node*, typename Alloc:: template rebind< Node* >::other >;
		using NodePtrListArray = Array< NodePtrList, typename Alloc:: template rebind< NodePtrList >::other >;
		
		using SiblingsList = list< Siblings, typename Alloc:: template rebind< Siblings >::other >;
		using SiblingsListArray = Array< SiblingsList, typename Alloc:: template rebind< SiblingsList >::other >;
		
		GpuLoaderThread( QGLWidget* widget, const ulong gpuMemQuota );
		
		void pushRequests( NodePtrList& load, NodePtrList& unload, SiblingsList& release );
		
		bool reachedGpuMemQuota();
		
	protected:
		void run() Q_DECL_OVERRIDE;
		
	private:
		void load( Node& node );
		void unload( Node& node );
		void release( Siblings& siblings );
		
		NodePtrList m_load;
		NodePtrList m_unload;
		SiblingsList m_release;
		
		mutex m_mutex;
		
		atomic_ulong m_availableGpuMem;
		ulong m_totalGpuMem;
	};
	
	template< typename Point, typename Alloc >
	inline GpuLoaderThread< Point, Alloc >( QGLWidget* widget, const ulong gpuMemQuota )
	: m_availableGpuMem( gpuMemQuota ),
	m_totalGpuMem( gpuMemQuota )
	{
		widget->doneCurrent();
		widget->context()->moveToThread( this );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoaderThread< Point, Alloc >
	::pushRequests( NodePtrList& load, NodePtrList& unload, SiblingsList& release )
	{
		lock_guard< mutex > lock;
		m_load.splice( m_load.end(), load );
		m_unload.splice( m_unload.end(), unload );
		m_release.splice( m_release.end(), release );
		
		start();
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoaderThread< Point, Alloc >::reachedGpuMemQuota()
	{
		return float( m_availableGpuMem ) < 0.05f * float( m_totalGpuMem );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoaderThread< Point, Alloc >::run()
	{
		NodePtrList load;
		NodePtrList unload;
		SiblingsList release;
		{
			lock_guard< mutex > loadList( m_mutex );
			load.splice( load.end(), m_load );
			unload.splice( unload.end(), m_unload );
			release.splice( release.end(), m_release );
		}
		
		for( Node* node : load )
		{
			load( node );
		}
		
		for( Node* node : unload )
		{
			unload( node );
		}
		
		for( Siblings& siblings : release )
		{
			release( siblings );
		}
	}
	
	template<>
	inline void GpuLoaderThread< Point, TbbAllocator< Point > >::load( Node& node )
	{
		Array< PointPtr > points = node.getContents();
		
		ulong neededGpuMem = 7 * sizeof( float ) * points.size(); // 4 floats for positions and 3 for normals.
		
		if( neededGpuMem <= m_availableGpuMem )
		{
			vector< Eigen::Vector4f > positions;
			vector< Eigen::Vector3f > normals;
			
			for( PointPtr point : points )
			{
				const Vec3& pos = point->getPos();
				positions.push_back( Vector4f( pos.x(), pos.y(), pos.z(), 1.f ) );
				
				normals.push_back( point->getColor() );
			}
			
			Mesh& mesh = node.mesh();
			mesh.selectPrimitive( Mesh::POINT );
			mesh.loadVertices( positions );
			mesh.loadNormals( normals );
			node.setLoaded( true );
			
			m_availableGpuMem -= neededGpuMem;
		}
	}
	
	template<>
	inline void GpuLoaderThread< ExtendedPoint, TbbAllocator< ExtendedPoint > >::load( Node& node )
	{
		Array< ExtendedPointPtr > points = node.getContents();
		
		ulong neededGpuMem = 11 * sizeof( float ) * points.size(); // 4 floats for positions, 4 colors and 3 for normals.
		
		if( neededGpuMem <= m_availableGpuMem )
		{
			vector< Eigen::Vector4f > positions;
			vector< Eigen::Vector3f > normals;
			vector< Eigen::Vector4f > colors;
			
			for( ExtendedPointPtr point : points )
			{
				const Vec3& pos = point->getPos();
				positions.push_back( Vector4f( pos.x(), pos.y(), pos.z(), 1.f ) );
				
				normals.push_back( point->getNormal() );
				
				const Vec3& color = point->getColor();
				colors.push_back( Vector4f( color.x(), color.y(), color.z(), 1.f ) );
			}
			
			Mesh& mesh = node.mesh();
			mesh.selectPrimitive( Mesh::POINT );
			mesh.loadVertices( positions );
			mesh.loadNormals( normals );
			mesh.loadColors( colors );
			node.setLoaded( true );
			
			m_availableGpuMem -= neededGpuMem;
		}
	}
	
	template<>
	inline void GpuLoaderThread< Point, TbbAllocator< Point > >::unload( Node& node )
	{
		for( Node& node : node.child() )
		{
			if( node.isLoaded() )
			{
				unload( node ); 
			}
		}
		
		m_availableGpuMem += 7 * sizeof( float ) * node.getContents().size();
		node.mesh().reset();
	}
	
	template<>
	inline void GpuLoaderThread< ExtendedPoint, TbbAllocator< ExtendedPoint > >::unload( Node& node )
	{
		for( Node& node : node.child() )
		{
			if( node.isLoaded() )
			{
				unload( node ); 
			}
		}
		
		m_availableGpuMem += 11 * sizeof( float ) * node.getContents().size();
		node.mesh().reset();
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoaderThread< Point, Alloc >::release( Siblings& siblings )
	{
		for( Node& sibling : siblings )
		{
			Siblings& children = sibling.child();
			if( !children.empty() )
			{
				release( children );
			}
		}
		
		siblings.clear();
	}
}

#endif