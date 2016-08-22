#ifndef GPU_LOADER_H
#define GPU_LOADER_H

#include <list>
#include <condition_variable>
#include <future>
#include "Array.h"
#include "O1OctreeNode.h"
#include "mesh.hpp"

using namespace std;

namespace model
{
	/** Asynchronous-threading-aware loader and unloader for point meshes in the GPU. Requests are handled in iterations
	 * in order to minimize locking overhead. In each iteration, user threads' can concurrently request load and unload
	 * operations. After all requests are issued, onIterationEnd() must be called in order to flush them to internal
	 * datastructures. */
	template< typename Point, typename Alloc = TbbAllocator< Point > >
	class GpuLoader
	{
	public:
		using PointPtr = shared_ptr< Point >;
		using Node = O1OctreeNode< PointPtr >;
		
		GpuLoader( const uint nUserThreads, const ulong gpuMemQuota );
		void load( Node& node, const uint threadIdx );
		void unload( Node& node, const uint threadIdx );
		void onIterationEnd();
		bool reachedQuota() {}
		
	private:
		using NodeList = list< Node*, typename Alloc::rebind< Node* >::other >;
		using NodeListArray = Array< NodeList, typename Alloc::rebind< NodeList >::other >;
		
		void load( Node& node );
		void unload( Node& node );
		
		NodeListArray m_iterLoad;
		NodeListArray m_iterUnload;
		
		atomic_ulong m_availableGpuMem;
	};
	
	template< typename Point, typename Alloc >
	GpuLoader< Point, Alloc >::GpuLoader( const uint nUserThreads, const ulong gpuMemQuota )
	: m_iterLoad( nUserThreads ),
	m_iterUnload( nUserThreads ),
	m_availableGpuMem( gpuMemQuota )
	{}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::load( Node& node, const uint threadIdx )
	{
		m_iterLoad[ threadIdx ].push_back( &node );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::unload( Node& node, const uint threadIdx )
	{
		m_iterUnload[ threadIdx ].push_back( &node );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::onIterationEnd()
	{
		NodeList loadList;
		for( NodeList& list : m_iterLoad )
		{
			loadList.splice( loadList.end(), list );
		}
		
		async( launch::async, [ & ]()
			{
				for( /**/; !loadList.empty(); loadList.pop_front() )
				{
					Node* node = loadList.front();
					load( *node );
				}
			}
		);
		
		NodeList unloadList;
		for( NodeList& list : m_iterUnload )
		{
			unloadList.splice( unloadList.end(), list );
		}

		async( launch::async, [ & ]()
			{
				for( /**/; !unloadList.empty(); unloadList.pop_front() )
				{
					Node* node = unloadList.front();
					unload( *node );
				}
			}
		);
	}
	
	template< typename Alloc >
	inline void GpuLoader< Point, Alloc >::load( Node& node )
	{
		Array< PointPtr > points = node.getContents();
		
		ulong neededGpuMem = 7 * sizeof( float ) * points.size(); // 4 floats for positions and 3 for normals.
		
		if( neededGpuMem <= m_availableGpuMem )
		{
			vector< Eigen::Vector4f > positions;
			vector< Eigen::Vector3f > normals;
			
			for( PointPtr point : points )
			{
				positions.push_back( point->getPos() );
				normals.push_back( point->getColor() );
			}
			
			node.mesh.loadVertices( positions );
			node.mesh.loadNormals( normals );
			node.isLoaded = true;
			
			m_availableGpuMem -= neededGpuMem;
		}
	}
	
	template< typename Alloc >
	inline void GpuLoader< ExtendedPoint, Alloc >::load( Node& node )
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
				positions.push_back( point->getPos() );
				normals.push_back( point->getNormal() );
				colors.push_back( point->getColor() );
			}
			
			node.mesh.loadVertices( positions );
			node.mesh.loadNormals( normals );
			node.mesh.loadColors( colors );
			node.isLoaded = true;
			
			m_availableGpuMem -= neededGpuMem;
		}
	}
	
	template< typename Alloc >
	inline void GpuLoader< Point, Alloc >::unload( Node& node )
	{
		m_availableGpuMem += 7 * sizeof( float ) * node.getContents.size();
		node.mesh.reset();
	}
	
	template< typename Alloc >
	inline void GpuLoader< ExtendedPoint, Alloc >::unload( Node& node )
	{
		m_availableGpuMem += 11 * sizeof( float ) * node.getContents.size();
		node.mesh.reset();
	}
}

#endif