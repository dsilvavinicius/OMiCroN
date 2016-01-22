#ifndef TBB_ALLOCATOR_H
#define TBB_ALLOCATOR_H

#include <iostream>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <tbb/scalable_allocator.h>

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
		using map = unordered_map< thread::id, ulong >;
		
		static void notifyAlloc( size_t bytes )
		{
			m_stats[ this_thread::get_id() ] += bytes;
		}
		
		static void notifyDealloc( size_t bytes )
		{
			m_stats[ this_thread::get_id() ] -= bytes;
		}
		
		static ulong totalAllocated()
		{
			ulong allocated = 0;
			for( map::value_type entry : m_stats ) 
			{
				allocated += entry.second;
			}
			
			return allocated;
		}
	private:
		static map m_stats;
	};
	
	/** Threading Building Blocks scalable_allocator wrapper. Reports allocations and deallocations in order to maintain
	 * statists of use. */
	template< typename T >
	class TbbAllocator
	{
		using ScalableAlloc = scalable_allocator< T >;
	public:
		using value_type = typename ScalableAlloc::value_type;
		using pointer = typename ScalableAlloc::pointer;
		using const_pointer = typename ScalableAlloc::const_pointer;
		using reference = typename ScalableAlloc::reference;
		using const_reference = typename ScalableAlloc::const_reference;
		using size_type = typename ScalableAlloc::size_type;
		using difference_type = typename ScalableAlloc::difference_type;
		
		template< typename U >
		struct rebind{ using other = TbbAllocator< U >; };
		
		TbbAllocator(){}
		TbbAllocator( const TbbAllocator< T >& ){}
		
		template< typename U >
		TbbAllocator( const TbbAllocator< U >& ){}
		
		pointer allocate( size_type n )
		{
			pointer p = ScalableAlloc().allocate( n );
			AllocStatistics::notifyAlloc( scalable_msize( p ) );
			return p;
		}
		void deallocate( pointer p, size_type = 0 )
		{
			AllocStatistics::notifyDealloc( scalable_msize( p ) );
			ScalableAlloc().deallocate( p, 0 );
		}
		
		template< class U, class... Args >
		void construct( U* p, Args&&... args ) { ScalableAlloc().construct( p, std::forward< Args >( args )... ); }
		
		template< typename U >
		void destroy( U* p ){ ScalableAlloc().destroy( p ); }
		
		bool operator==( const TbbAllocator& ){ return true; }
		bool operator!=( const TbbAllocator& ){ return false; }
	};
}

#endif