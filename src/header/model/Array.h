#ifndef ARRAY_H
#define ARRAY_H
#include "Stream.h"
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
		{
			assert( m_size != 0 && "Array::Array( uint ) does not accept size 0. To create an empty array use "
					"Array::Array()" );
			
			initArray();
		}
		
		/** Ctor that takes resposibility of a given array with the given size. */
		Array( uint size, T* array )
		: m_size( size ),
		m_array( array )
		{
			assert( m_size != 0 && "Array::Array( uint, T* ) does not accept size 0. To create an empty array use "
					"Array::Array()" );
		}
		
		Array( const Array& other )
		{
			if( other.m_size == 0 )
			{
				m_size = 0;
				m_array = nullptr;
			}
			else
			{
				m_size = other.m_size;
				m_array = A().allocate( m_size );
				initArray();
				
				for( int i = 0; i < m_size; ++i )
				{
					m_array[ i ] = other[ i ];
				}
			}
		}
		
		Array& operator=( const Array& other )
		{
			this->~Array();
			
			if( other.m_size != 0 )
			{
				m_size = other.m_size;
				m_array = A().allocate( m_size );
				initArray();
				
				for( int i = 0; i < m_size; ++i )
				{
					m_array[ i ] = other.m_array[ i ];
				}
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
			if( m_size != 0 )
			{
				for( int i = 0; i < m_size; i++ )
				{
					( m_array + i )->~T();
				}
				
				A().deallocate( m_array );
				
				m_size = 0;
				m_array = nullptr;
			}
		}
		
		T& operator[]( uint i )
		{
			return m_array[ i ];
		}
		
		const T& operator[]( uint i ) const
		{
			return m_array[ i ];
		}
		
		bool operator==( const Array< T >& other ) const
		{
			if( m_size != other.m_size ) { return false; }
			
			for( int i = 0; i < m_size; i++ )
			{
				if( m_array[ i ] != other.m_array[ i ] ) { return false; }
			}
			
			return true;
		}
		
		uint size() const { return m_size; }
		
		T* data() { return m_array; }
		
		template< typename Type, typename Alloc >
		friend ostream& operator<<( ostream& out, const Array< Type, Alloc >& array );
		
		template< typename Type, typename Alloc >
		friend ostream& operator<<( ostream& out, const Array< shared_ptr< Type >, Alloc >& array );
		
	private:
		/** Inits the internal array. */
		void initArray()
		{
			for( int i = 0; i < m_size; ++i )
			{
				A().construct( m_array + i );
			}
		}
		
		T* m_array;
		uint m_size;
	};
	
	template< typename Type, typename Alloc >
	ostream& operator<<( ostream& out, const Array< Type, Alloc >& array )
	{
		for( int i = 0; i < array.m_size; ++i )
		{
			out << array[ i ] << endl;
		}
		out << endl;
		return out;
	}
	
	template< typename Type, typename Alloc >
	ostream& operator<<( ostream& out, const Array< shared_ptr< Type >, Alloc >& array )
	{
		for( int i = 0; i < array.m_size; ++i )
		{
			out << *array[ i ] << endl;
		}
		out << endl;
		return out;
	}
}

#endif