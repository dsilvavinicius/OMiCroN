#ifndef ARRAY_H
#define ARRAY_H
#include "TbbAllocator.h"

namespace model
{
	/** Lightweight array to use in place of std::vector. It has less memory overhead than the aforementioned container.
	 * @param T is the array element type.
	 * @param A is the allocator type. */
	template< typename T, typename A = TbbAllocator< T > >
	class Array
	{
	public:
		/** Ctor to construct an empty non-usable array. */
		Array()
		: m_size( 0 ),
		m_array( nullptr )
		{}
		
		/** Ctor that allocates a new array with the given size. */
		Array( uint size )
		: m_size( size ),
		m_array( A().allocate( size ) )
		{}
		
		/** Ctor that takes resposibility of a given array with the given size. */
		Array( uint size, T* array )
		: m_size( size ),
		m_array( array )
		{}
		
		Array( const Array& other )
		: Array( other.m_size )
		{
			for( int i = 0; i < m_size; ++i )
			{
				m_array[ i ] = other[ i ];
			}
		}
		
		Array& operator=( const Array& other )
		{
			~Array();
			m_size = other.size;
			m_array = A().allocate( m_size );
			
			for( int i = 0; i < m_size; ++i )
			{
				m_array[ i ] = other[ i ];
			}
			
			return *this;
		}
		
		Array( Array&& other )
		: Array( other.m_size, other.m_array ) 
		{
			other.m_array = nullptr;
			other.m_size = 0;
		}
		
		Array& operator=( Array&& other )
		{
			m_array = other.m_array;
			m_size = other.m_size;
			
			other.m_array = nullptr;
			other.m_size = 0;
			
			return *this;
		}
		
		~Array()
		{
			if( m_array != nullptr )
			{
				for( int i = 0; i < m_size; i++ )
				{
					( m_array + i )->~T();
				}
				
				A().deallocate( m_array );
			}
		}
		
		T& operator[]( uint i )
		{
			return m_array[ i ];
		}
		
		bool operator==( const Array< T >& other )
		{
			if( m_size != other.m_size ) { return false; }
			for( int i = 0; i < m_size; i++ )
			{
				if( m_array[ i ] != other.m_array[ i ] ) { return false; }
			}
			return true;
		}
		
		uint size() { return m_size; }
		
		T* data() { return m_array; }
		
	private:
		T* m_array;
		uint m_size;
	};
}

#endif