#ifndef GPU_LOADER_H
#define GPU_LOADER_H

#include <list>
#include <future>
#include "omicron/basic/array.h"
#include "omicron/hierarchy/o1_octree_node.h"
#include "omicron/hierarchy/node_loader_thread.h"
#include "tucano/mesh.hpp"

using namespace std;

namespace omicron::hierarchy
{
	/** Asynchronous-threading-aware loader and unloader for point meshes in the GPU. Requests are handled in iterations
	 * in order to minimize locking overhead. In each iteration, user threads can concurrently request load and unload
	 * operations. After all requests are issued, onIterationEnd() must be called in order to flush them to internal
	 * datastructures. */
	template< typename Point, typename Alloc = TbbAllocator< Point > >
	class NodeLoader
	{
	public:
		using Node = typename NodeLoaderThread::Node;
		
		using NodePtrList = typename NodeLoaderThread::NodePtrList;
		using NodePtrListArray = typename NodeLoaderThread::NodePtrListArray;
		
		using Siblings = typename NodeLoaderThread::Siblings;
		using SiblingsList = typename NodeLoaderThread::SiblingsList;
		using SiblingsListArray = typename NodeLoaderThread::SiblingsListArray;
		
		NodeLoader( NodeLoaderThread* loaderThread, const uint nUserThreads );
		
		~NodeLoader();
		
		/** Makes a request for loading a node into GPU memory. THREAD SAFE. */
		void asyncLoad( Node& node, const uint threadIdx );
		
		/** Makes a request for unloading a node in GPU memory. THREAD SAFE. */
		void asyncUnload( Node& node, const uint threadIdx );
		
		/** Makes a request for releasing a sibling group in main memory. THREAD SAFE. */
		void asyncRelease( Siblings&& siblings, const uint threadIdx );
		
		/** Notifies that all requests for the current iteration are done. */
		void onIterationEnd();
		
		/** Indicates if the gpu memory quota is reached. THREAD SAFE. */
		bool reachedGpuMemQuota();
		
		ulong memoryUsage();
		
		/** Indicates if there are sibling groups to release yet. THREAD SAFE. */
		bool isReleasing();
		
		const QGLWidget* widget();
		
	private:
		NodePtrListArray m_iterLoad;
		NodePtrListArray m_iterUnload;
		SiblingsListArray m_iterRelease;
		
		NodeLoaderThread* m_loaderThread;
	};
	
	template< typename Point, typename Alloc >
	NodeLoader< Point, Alloc >::NodeLoader( NodeLoaderThread* loaderThread, const uint nUserThreads )
	: m_iterLoad( nUserThreads ),
	m_iterUnload( nUserThreads ),
	m_iterRelease( nUserThreads ),
	m_loaderThread( loaderThread )
	{}
	
	template< typename Point, typename Alloc >
	NodeLoader< Point, Alloc >::~NodeLoader()
	{
		m_loaderThread->wait();
	}
	
	template< typename Point, typename Alloc >
	inline void NodeLoader< Point, Alloc >::asyncLoad( Node& node, const uint threadIdx )
	{
		if( m_loaderThread->hasMemoryFor( node.getContents() ) && !node.isLoaded() )
		{
			node.loadInGpu();
			m_iterLoad[ threadIdx ].push_back( &node );
		}
	}
	
	template< typename Point, typename Alloc >
	inline void NodeLoader< Point, Alloc >::asyncUnload( Node& node, const uint threadIdx )
	{
// 		if( node.isLoaded() )
// 		{
// 			m_iterUnload[ threadIdx ].push_back( &node );
// 		}

// 		node.unloadGPU();
	}
	
	template< typename Point, typename Alloc >
	inline void NodeLoader< Point, Alloc >::asyncRelease( Siblings&& siblings, const uint threadIdx )
	{
		m_iterRelease[ threadIdx ].push_back( std::move( siblings ) );
	}
	
	template< typename Point, typename Alloc >
	inline void NodeLoader< Point, Alloc >::onIterationEnd()
	{
		NodePtrList load;
		for( NodePtrList& list : m_iterLoad )
		{
			load.splice( load.end(), list );
		}
		
		NodePtrList unload;
		for( NodePtrList& list: m_iterUnload )
		{
			unload.splice( unload.end(), list );
		}
		
		SiblingsList release;
		for( SiblingsList& list : m_iterRelease )
		{
			release.splice( release.end(), list );
		}
		
		m_loaderThread->pushRequests( load, unload, release );
	}
	
	template< typename Point, typename Alloc >
	inline bool NodeLoader< Point, Alloc >::reachedGpuMemQuota()
	{
		return m_loaderThread->reachedGpuMemQuota();
	}
	
	template< typename Point, typename Alloc >
	inline ulong NodeLoader< Point, Alloc >::memoryUsage()
	{
		return m_loaderThread->memoryUsage();
	}
	
	template< typename Point, typename Alloc >
	inline bool NodeLoader< Point, Alloc >::isReleasing()
	{
		return m_loaderThread->isReleasing();
	}
	
	template< typename Point, typename Alloc >
	inline const QGLWidget* NodeLoader< Point, Alloc >::widget()
	{
		return m_loaderThread->widget();
	}
}

#endif
