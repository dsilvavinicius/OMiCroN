#ifndef GPU_LOADER_H
#define GPU_LOADER_H

#include <list>
#include <future>
#include "Array.h"
#include "O1OctreeNode.h"
#include "GpuLoaderThread.h"
#include "mesh.hpp"

using namespace std;

namespace model
{
	/** Asynchronous-threading-aware loader and unloader for point meshes in the GPU. Requests are handled in iterations
	 * in order to minimize locking overhead. In each iteration, user threads can concurrently request load and unload
	 * operations. After all requests are issued, onIterationEnd() must be called in order to flush them to internal
	 * datastructures. */
	template< typename Point, typename Alloc = TbbAllocator< Point > >
	class GpuLoader
	{
	public:
		using GpuLoaderThread = model::GpuLoaderThread< Point, Alloc >;
		using Node = typename GpuLoaderThread::Node;
		
		using NodePtrList = typename GpuLoaderThread::NodePtrList;
		using NodePtrListArray = typename GpuLoaderThread::NodePtrListArray;
		
		using SiblingsList = typename GpuLoaderThread::SiblingList;
		using SiblingsListArray = typename GpuLoaderThread::SiblingsListArray;
		
		GpuLoader( GpuLoaderThread* loaderThread, const uint nUserThreads );
		
		/** Makes a request for loading a node into GPU memory. THREAD SAFE. */
		void asyncLoad( Node& node );
		
		/** Makes a request for unloading a node in GPU memory. THREAD SAFE. */
		void asyncUnload( Node& node );
		
		/** Makes a request for releasing a sibling group in main memory. THREAD SAFE. */
		void asyncRelease( Siblings&& siblings );
		
		/** Notifies that all requests for the current iteration are done. */
		void onIterationEnd();
		
		/** Indicates if the gpu memory quota is reached. THREAD SAFE. */
		bool reachedGpuMemQuota();
		
		/** Indicates if there are sibling groups to release yet. THREAD SAFE. */
		bool isReleasing();
		
	private:
		NodePtrListArray m_iterLoad;
		NodePtrListArray m_iterUnload;
		SiblingsListArray m_iterRelease;
		
		GpuLoaderThread* m_loaderThread;
		
		/** true if the front has nodes to release yet, false otherwise. */
		atomic_bool m_releaseFlag;
	};
	
	template< typename Point, typename Alloc >
	GpuLoader< Point, Alloc >::GpuLoader( GpuLoaderThread* loaderThread, const uint nUserThreads )
	: m_iterLoad( nUserThreads ),
	m_iterUnload( nUserThreads ),
	m_iterRelease( nUserThreads ),
	m_loaderThread( loaderThread ),
	m_releaseFlag( false )
	{}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::asyncLoad( Node& node, const uint threadIdx )
	{
		m_iterLoad[ threadIdx ].push_back( node );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::asyncUnload( Node& node, const uint threadIdx )
	{
		node.setLoaded( false );
		m_iterUnload[ threadIdx ].push_back( node );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::asyncRelease( Siblings&& siblings, const uint threadIdx )
	{
		m_iterRelease[ threadIdx ].push_back( siblings );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::onIterationEnd()
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
	inline bool GpuLoader< Point, Alloc >::reachedGpuMemQuota()
	{
		return m_loaderThread->reachedGpuMemQuota();
	}
	
	template< typename Point, typename Alloc >
	inline bool GpuLoader< Point, Alloc >::isReleasing()
	{
		return m_releaseFlag;
	}
}

#endif