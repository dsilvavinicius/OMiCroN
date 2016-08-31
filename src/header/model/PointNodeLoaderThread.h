#ifndef GPU_LOADER_THREAD_H
#define GPU_LOADER_THREAD_H

#include <QThread>
#include <utils/qtfreecamerawidget.hpp>
#include "TbbAllocator.h"
#include "Point.h"
#include "O1OctreeNode.h"
#include "GpuAllocStatistics.h"

namespace model
{
	class PointNodeLoaderThread
	: public QThread
	{
		Q_OBJECT
	
	public:
		using Point = model::Point;
		using PointPtr = shared_ptr< Point >;
		using Alloc = TbbAllocator< Point >;
		using Node = O1OctreeNode< PointPtr >;
		using Siblings = Array< Node >;
		using NodePtrList = list< Node*, typename Alloc:: template rebind< Node* >::other >;
		using NodePtrListArray = Array< NodePtrList, typename Alloc:: template rebind< NodePtrList >::other >;
		
		using SiblingsList = list< Siblings, typename Alloc:: template rebind< Siblings >::other >;
		using SiblingsListArray = Array< SiblingsList, typename Alloc:: template rebind< SiblingsList >::other >;
		
		PointNodeLoaderThread( QGLWidget* widget, const ulong gpuMemQuota );
		
		void pushRequests( NodePtrList& load, NodePtrList& unload, SiblingsList& release );
		
		bool reachedGpuMemQuota();
		
		ulong memoryUsage();
		
		bool isReleasing();
		
		const QGLWidget* widget();
		
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
		
		/** true if the front has nodes to release yet, false otherwise. */
		atomic_bool m_releaseFlag;
		
		ulong m_totalGpuMem;
		
		QGLWidget* m_widget;
	};
	
	inline PointNodeLoaderThread::PointNodeLoaderThread( QGLWidget* widget, const ulong gpuMemQuota )
	: QThread( widget ),
	m_widget( widget ),
	m_totalGpuMem( gpuMemQuota ),
	m_releaseFlag( false )
	{
		m_widget->doneCurrent();
		m_widget->context()->moveToThread( this );
	}
	
	inline void PointNodeLoaderThread::pushRequests( NodePtrList& load, NodePtrList& unload, SiblingsList& release )
	{
		{
			lock_guard< mutex > lock( m_mutex );
			m_load.splice( m_load.end(), load );
			m_unload.splice( m_unload.end(), unload );
			m_release.splice( m_release.end(), release );
		}
		
		start();
	}
	
	inline bool PointNodeLoaderThread::reachedGpuMemQuota()
	{
		return float( memoryUsage() ) > 0.95f * float( m_totalGpuMem );
	}
	
	inline ulong PointNodeLoaderThread::memoryUsage()
	{
		return GpuAllocStatistics::totalAllocated();
	}
	
	inline bool PointNodeLoaderThread::isReleasing()
	{
		return m_releaseFlag;
	}
	
	inline const QGLWidget* PointNodeLoaderThread::widget()
	{
		return m_widget;
	}
	
	inline void PointNodeLoaderThread::run()
	{
		m_widget->makeCurrent();
		
		NodePtrList loadList;
		NodePtrList unloadList;
		SiblingsList releaseList;
		{
			lock_guard< mutex > lock( m_mutex );
			loadList.splice( loadList.end(), m_load );
			unloadList.splice( unloadList.end(), m_unload );
			releaseList.splice( releaseList.end(), m_release );
		}
		
		for( Node* node : loadList )
		{
			load( *node );
		}
		
		for( Node* node : unloadList )
		{
			unload( *node );
		}
		
		m_releaseFlag = ( releaseList.empty() ) ? false : true;
		
		for( Siblings& siblings : releaseList )
		{
			release( siblings );
		}
	}
	
	inline void PointNodeLoaderThread::load( Node& node )
	{
		Array< PointPtr > points = node.getContents();
		
		ulong neededGpuMem = GpuAllocStatistics::pointSize() * points.size();
		
		if( memoryUsage() + neededGpuMem < m_totalGpuMem )
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
			node.setLoadState( Node::LOADED );
			
			GpuAllocStatistics::notifyAlloc( neededGpuMem );
		}
	}
	
	inline void PointNodeLoaderThread::unload( Node& node )
	{
		for( Node& child : node.child() )
		{
			if( child.loadState() == Node::LOADED )
			{
				unload( child ); 
			}
		}
		
		GpuAllocStatistics::notifyDealloc( GpuAllocStatistics::pointSize() * node.getContents().size() );
		node.mesh().reset();
		node.setLoadState( Node::UNLOADED );
	}
	
	inline void PointNodeLoaderThread::release( Siblings& siblings )
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