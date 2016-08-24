#ifndef GPU_LOADER_H
#define GPU_LOADER_H

#include <list>
#include <future>
#include "Array.h"
#include "O1OctreeNode.h"
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
		using PointPtr = shared_ptr< Point >;
		using Node = O1OctreeNode< PointPtr >;
		using Siblings = Array< Node >;
		
		GpuLoader( const uint nUserThreads, const ulong gpuMemQuota );
		
		/** Makes a request for loading a node into GPU memory. THREAD SAFE. */
		void load( Node* node, const uint threadIdx );
		
		/** Makes a request for unloading a node in GPU memory. THREAD SAFE. */
		void unload( Node* node, const uint threadIdx );
		
		/** Makes a request for releasing a sibling group in main memory. THREAD SAFE. */
		void release( Siblings&& siblings, const uint threadIdx );
		
		/** Notifies that all requests for the current iteration are done. */
		void onIterationEnd();
		
		/** Indicates if the gpu memory quota is reached. THREAD SAFE. */
		bool reachedQuota();
		
		/** Indicates if there are sibling groups to release yet. THREAD SAFE. */
		bool isReleasing();
		
	private:
		using NodePtrList = list< Node*, typename Alloc:: template rebind< Node* >::other >;
		using NodePtrListArray = Array< NodePtrList, typename Alloc:: template rebind< NodePtrList >::other >;
		
		using SiblingsList = list< Siblings, typename Alloc:: template rebind< Siblings >::other >;
		using SiblingsListArray = Array< SiblingsList, typename Alloc:: template rebind< SiblingsList >::other >;
		
		void load( Node& node );
		void unload( Node& node );
		void release( Siblings& siblings );
		
		NodePtrListArray m_iterLoad;
		NodePtrListArray m_iterUnload;
		SiblingsListArray m_iterRelease;
		
		atomic_ulong m_availableGpuMem;
		
		/** true if the front has nodes to release yet, false otherwise. */
		atomic_bool m_releaseFlag;
		
		ulong m_totalGpuMem;
	};
	
	template< typename Point, typename Alloc >
	GpuLoader< Point, Alloc >::GpuLoader( const uint nUserThreads, const ulong gpuMemQuota )
	: m_iterLoad( nUserThreads ),
	m_iterUnload( nUserThreads ),
	m_iterRelease( nUserThreads ),
	m_availableGpuMem( gpuMemQuota ),
	m_totalGpuMem( gpuMemQuota ),
	m_releaseFlag( false )
	{}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::load( Node* node, const uint threadIdx )
	{
		m_iterLoad[ threadIdx ].push_back( node );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::unload( Node* node, const uint threadIdx )
	{
		node->setLoaded( false );
		m_iterUnload[ threadIdx ].push_back( node );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::release( Siblings&& siblings, const uint threadIdx )
	{
		m_iterRelease[ threadIdx ].push_back( std::move( siblings ) );
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::onIterationEnd()
	{
		NodePtrList loadList;
		for( NodePtrList& list : m_iterLoad )
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
		
		NodePtrList unloadList;
		for( NodePtrList& list : m_iterUnload )
		{
			unloadList.splice( unloadList.end(), list );
		}

		auto futureUnload = async( launch::async, [ & ]()
			{
				for( /**/; !unloadList.empty(); unloadList.pop_front() )
				{
					Node* node = unloadList.front();
					unload( *node );
				}
			}
		);
		
		SiblingsList releaseList;
		for( SiblingsList& list : m_iterRelease )
		{
			releaseList.splice( releaseList.end(), list );
		}
		if( !releaseList.empty() )
		{
			m_releaseFlag = true;
			futureUnload.get(); // It is necessary to wait unloads in order to avoid unloading an already released node.
			async( launch::async, [ & ]()
				{
					for( /**/; !releaseList.empty(); releaseList.pop_front() )
					{
						release( releaseList.front() );
					}
				}
			);
		}
		else
		{
			m_releaseFlag = false;
		}
	}
	
	template< typename Point, typename Alloc >
	inline bool GpuLoader< Point, Alloc >::reachedQuota()
	{
		return float( m_availableGpuMem ) < 0.05f * float( m_totalGpuMem );
	}
	
	template< typename Point, typename Alloc >
	inline bool GpuLoader< Point, Alloc >::isReleasing()
	{
		return m_releaseFlag;
	}
	
	template<>
	inline void GpuLoader< Point, TbbAllocator< Point > >::load( Node& node )
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
			mesh.loadVertices( positions );
			mesh.loadNormals( normals );
			node.setLoaded( true );
			
			m_availableGpuMem -= neededGpuMem;
		}
	}
	
	template<>
	inline void GpuLoader< ExtendedPoint, TbbAllocator< ExtendedPoint > >::load( Node& node )
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
			mesh.loadVertices( positions );
			mesh.loadNormals( normals );
			mesh.loadColors( colors );
			node.setLoaded( true );
			
			m_availableGpuMem -= neededGpuMem;
		}
	}
	
	template<>
	inline void GpuLoader< Point, TbbAllocator< Point > >::unload( Node& node )
	{
		m_availableGpuMem += 7 * sizeof( float ) * node.getContents().size();
		node.mesh().reset();
	}
	
	template<>
	inline void GpuLoader< ExtendedPoint, TbbAllocator< ExtendedPoint > >::unload( Node& node )
	{
		m_availableGpuMem += 11 * sizeof( float ) * node.getContents().size();
		node.mesh().reset();
	}
	
	template< typename Point, typename Alloc >
	inline void GpuLoader< Point, Alloc >::release( Siblings& siblings )
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