#ifndef GPU_LOADER_THREAD_H
#define GPU_LOADER_THREAD_H

#include <QThread>
#include <utils/qtfreecamerawidget.hpp>
#include "TbbAllocator.h"
#include "Point.h"
#include "O1OctreeNode.h"
#include "GpuAllocStatistics.h"
#include "splat_renderer/surfel.hpp"

namespace model
{
	class NodeLoaderThread
	: public QThread
	{
		Q_OBJECT
	
	public:
		using Point = Surfel;
		using PointArray = Array< Surfel >;
		using Alloc = TbbAllocator< Point >;
		using Node = O1OctreeNode< Point >;
		using Siblings = Array< Node >;
		using NodePtrList = list< Node*, typename Alloc:: template rebind< Node* >::other >;
		using NodePtrListArray = Array< NodePtrList, typename Alloc:: template rebind< NodePtrList >::other >;
		
		using SiblingsList = list< Siblings, typename Alloc:: template rebind< Siblings >::other >;
		using SiblingsListArray = Array< SiblingsList, typename Alloc:: template rebind< SiblingsList >::other >;
		
		NodeLoaderThread( QGLWidget* widget, const ulong gpuMemQuota );
		
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
	
	inline NodeLoaderThread::NodeLoaderThread( QGLWidget* widget, const ulong gpuMemQuota )
	: QThread( widget ),
	m_widget( widget ),
	m_totalGpuMem( gpuMemQuota ),
	m_releaseFlag( false )
	{
		m_widget->doneCurrent();
		m_widget->context()->moveToThread( this );
	}
	
	inline void NodeLoaderThread::pushRequests( NodePtrList& load, NodePtrList& unload, SiblingsList& release )
	{
		{
			lock_guard< mutex > lock( m_mutex );
			m_load.splice( m_load.end(), load );
			m_unload.splice( m_unload.end(), unload );
			m_release.splice( m_release.end(), release );
		}
		
		start();
	}
	
	inline bool NodeLoaderThread::reachedGpuMemQuota()
	{
		return float( memoryUsage() ) > 0.95f * float( m_totalGpuMem );
	}
	
	inline ulong NodeLoaderThread::memoryUsage()
	{
		return GpuAllocStatistics::totalAllocated();
	}
	
	inline bool NodeLoaderThread::isReleasing()
	{
		return m_releaseFlag;
	}
	
	inline const QGLWidget* NodeLoaderThread::widget()
	{
		return m_widget;
	}
	
	inline void NodeLoaderThread::run()
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
	
	inline void NodeLoaderThread::load( Node& node )
	{
		PointArray points = node.getContents();
		
		ulong neededGpuMem = GpuAllocStatistics::pointSize() * points.size();
		
		if( memoryUsage() + neededGpuMem < m_totalGpuMem )
		{
			SurfelCloud cloud( points );
			node.loadCloud( cloud );
		}
	}
	
	inline void NodeLoaderThread::unload( Node& node )
	{
		for( Node& child : node.child() )
		{
			if( child.loadState() == Node::LOADED )
			{
				unload( child ); 
			}
		}
		
		node.unloadCloud();
	}
	
	inline void NodeLoaderThread::release( Siblings& siblings )
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