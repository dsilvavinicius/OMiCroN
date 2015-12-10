#ifndef ARRAY_H
#define ARRAY_H
#include "Stream.h"
#include "TbbAllocator.h"
#include "Serializer.h"
#include <boost/concept_check.hpp>

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
		: Array( 0 )
		{}
		
		/** Ctor that allocates a new array with the given size. */
		Array( uint size )
		: m_size( size )
		{
			if( m_size )
			{
				m_array = A().allocate( size );
				initArray();
			}
			else
			{
				m_array = nullptr;
			}
		}
		
		/** Ctor that takes resposibility of a given array with the given size. */
		Array( uint size, T* array )
		: m_size( size ),
		m_array( array )
		{}
		
		/** Moves each vector element to the array, clearing up v afterwards. */
		Array( vector< T, ManagedAllocator< T > >&& v )
		: Array( v.size() )
		{
			for( int i = 0; i < v.size(); ++i )
			{
				m_array[ i ] = std::move( v[ i ] );
			}
			
			v.clear();
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
			clear();
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
		
		/** Destructs all elements and deletes internal array. The empty Array cannot be resized and is useless. */
		void clear()
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
		
		size_t serialize( byte** serialization ) const
		{
			return serialize( m_array, serialization );
		}
		
		static Array deserialize( byte* serialization )
		{
			uint count;
			size_t countSize = sizeof( m_size );
			memcpy( &count, serialization, countSize );
			serialization += countSize;
			
			Array array( count );
			array.deserialize( array.m_array, serialization );
			
			return array;
		}
		
		uint size() const { return m_size; }
		
		T* data() { return m_array; }
		
		const T* data() const { return m_array; }
		
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
		
		template< typename U >
		size_t serialize( shared_ptr< U >*, byte** serialization ) const
		{
			size_t countSize = sizeof( decltype( m_size ) );
			
			if( m_size > 0 )
			{
				byte* elemBytes;
				size_t elemSize = m_array[ 0 ]->serialize( &elemBytes );
				
				size_t vecSize = m_size * elemSize;
				size_t serializationSize = countSize + vecSize;
				
				*serialization = Serializer::newByteArray( serializationSize );
				byte* tempPointer = *serialization;
				memcpy( tempPointer, &m_size, countSize );
				tempPointer += countSize;
				memcpy( tempPointer, elemBytes, elemSize );
				Serializer::dispose( elemBytes );
				
				for( int i = 1; i < m_size; ++i )
				{
					shared_ptr< U > elem = m_array[ i ];
					tempPointer += elemSize;
					
					elem->serialize( &elemBytes );
					memcpy( tempPointer, elemBytes, elemSize );
					Serializer::dispose( elemBytes );
				}
				
				return serializationSize;
			}
			else
			{
				size_t serializationSize = countSize;
				*serialization = Serializer::newByteArray( serializationSize );
				memcpy( *serialization, &m_size, countSize );
				
				return serializationSize;
			}
		}
		
		template< typename U >
		inline void deserialize( shared_ptr< U >*, byte* serialization )
		{
			byte* tempPtr0 = serialization;
			byte* tempPtr1 = nullptr;
			
			for( int i = 0; i < m_size; ++i )
			{
				m_array[ i ] = makeManaged< U >( tempPtr0, tempPtr1 );
				swap( tempPtr0, tempPtr1 );
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