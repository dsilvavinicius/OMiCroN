#ifndef TBB_ALLOCATOR_H
#define TBB_ALLOCATOR_H

#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>

#define SCALABLE

#ifdef SCALABLE
	#include <scalable_allocator.h>
#else
	#include <malloc.h>
	#include <tbb_allocator.h>
#endif

#include <sstream>
#include "HierarchyCreationLog.h"

#define DEBUG

using namespace std;
using namespace tbb;

namespace model
{
	/** Statistics for allocations in the multithreaded environment. A counter of allocated bytes is maintened per
	 * thread. The counters can be negative in case bytes are allocated in a thread and deallocated in another.
	 * The total allocated report will be correct, though. IMPORTANT: the scalable allocator memory overhead is not
	 * taken into consideration in the report. */
	class AllocStatistics
	{
	public:
		//using map = unordered_map< thread::id, ulong >;
		
		static void notifyAlloc( size_t bytes )
		{
			//m_stats[ this_thread::get_id() ] += bytes;
			m_allocated += bytes;
		}
		
		static void notifyDealloc( size_t bytes )
		{
			//m_stats[ this_thread::get_id() ] -= bytes;
			m_allocated -= bytes;
		}
		
		static ulong totalAllocated()
		{
// 			ulong allocated = 0;
// 			for( map::value_type entry : m_stats ) 
// 			{
// 				allocated += entry.second;
// 			}
// 			
// 			return allocated;
			return m_allocated.load();
		}
	private:
		static atomic_ulong m_allocated;
		//static map m_stats;
	};
	
	/** Threading Building Blocks scalable_allocator wrapper. Reports allocations and deallocations in order to maintain
	 * statists of use. */
	template< typename T >
	class TbbAllocator
	{
		#ifdef SCALABLE
			using InternalAlloc = scalable_allocator< T >;
		#else
			using InternalAlloc = tbb_allocator< T >;
		#endif
		
	public:
		using value_type = typename InternalAlloc::value_type;
		using pointer = typename InternalAlloc::pointer;
		using const_pointer = typename InternalAlloc::const_pointer;
		using reference = typename InternalAlloc::reference;
		using const_reference = typename InternalAlloc::const_reference;
		using size_type = typename InternalAlloc::size_type;
		using difference_type = typename InternalAlloc::difference_type;
		
		template< typename U >
		struct rebind{ using other = TbbAllocator< U >; };
		
		TbbAllocator(){}
		TbbAllocator( const TbbAllocator< T >& ){}
		
		template< typename U >
		TbbAllocator( const TbbAllocator< U >& ){}
		
		pointer allocate( size_type n )
		{
			#ifdef DEBUG
			{
				if( n == 158 )
				{
					stringstream ss; ss << "[ t" << omp_get_thread_num() << " ] starting allocation" << endl << endl;
					HierarchyCreationLog::logDebugMsg( ss.str() );
				}
			}
			#endif
			
			#ifdef SCALABLE
				pointer p = InternalAlloc().allocate( n );
				AllocStatistics::notifyAlloc( scalable_msize( p ) );
			#else
				pointer p = ( pointer ) malloc( sizeof( T ) * n );
				AllocStatistics::notifyAlloc( malloc_usable_size( p ) );
			#endif
			
			#ifdef DEBUG
			{
				if( n == 158 )
				{
					stringstream ss; ss << "[ t" << omp_get_thread_num() << " ] allocated" << endl << endl;
					HierarchyCreationLog::logDebugMsg( ss.str() );
				}
			}
			#endif
				
			return p;
		}
		void deallocate( pointer p, size_type = 0 )
		{
			#ifdef SCALABLE
				AllocStatistics::notifyDealloc( scalable_msize( p ) );
				InternalAlloc().deallocate( p, 0 );
			#else
				AllocStatistics::notifyDealloc( malloc_usable_size( p ) );
				free( p );
			#endif
		}
		
		template< class U, class... Args >
		void construct( U* p, Args&&... args ) { InternalAlloc().construct( p, std::forward< Args >( args )... ); }
		
		void destroy( T* p ){ InternalAlloc().destroy( p ); }
		
		bool operator==( const TbbAllocator& ){ return true; }
		bool operator!=( const TbbAllocator& ){ return false; }
	};
}

#undef DEBUG

#endif