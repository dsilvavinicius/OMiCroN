#ifndef ARRAY_H
#define ARRAY_H
#include "Stream.h"
#include "TbbAllocator.h"
#include "Serializer.h"
// #include <boost/concept_check.hpp>
#include "HierarchyCreationLog.h"
#include "global_malloc.h"

// #define DETAILED_STREAM
// #define DEBUG

namespace model
{
	/** Lightweight array to use in place of std::vector. It has less memory overhead than the aforementioned container.
	 * @param T is the array element type.
	 * @param A is the allocator type. */
	template< typename T, typename A = TbbAllocator< T > >
	class Array
	{
	public:
		using iterator = T*;
		using const_iterator = const T*;
		
		/** Ctor to construct an empty non-usable array. */
		Array()
		: Array( 0 )
		{}
		
		/** Ctor that allocates a new array with the given size. */
		Array( uint size )
		: m_size( size )
		{
			#ifdef DEBUG
			{
				stringstream ss; ss << "[ t " << omp_get_thread_num() << " ] array addr: " << this << endl << endl;
				HierarchyCreationLog::logDebugMsg( ss.str() );
			}
			#endif
			
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
		
		/** Ctor that allocates a new array with the given size and populates it with the given initialization value. */
		explicit Array( uint size, const T& initValue )
		: m_size( size )
		{
			if( m_size )
			{
				m_array = A().allocate( size );
				initArray( initValue );
			}
			else
			{
				m_array = nullptr;
			}
		}
		
// 		explicit Array( const T&& initValue )
// 		: m_size( 1 )
// 		{
// 			m_array = A().allocate( 1 );
// 			A().construct( m_array, std::move( initValue ) );
// 		}
		
		/** Ctor that takes resposibility of a given array with the given size. */
		explicit Array( uint size, T* array )
		: m_size( size ),
		m_array( array )
		{}
		
		/** Moves each vector element to the array, clearing up v afterwards. */
		template< template< typename , typename > class container >
		Array( container< T, ManagedAllocator< T > >&& v )
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
				
				auto otherIt = other.begin();
				for( auto it = begin(); it != end(); it++, otherIt++ )
				{
					*it = *otherIt;
				}
			}
		}
		
		Array& operator=( const Array& other )
		{
			this->clear();
			
			if( other.m_size != 0 )
			{
				m_size = other.m_size;
				m_array = A().allocate( m_size );
				initArray();
				
				auto otherIt = other.begin();
				for( auto it = begin(); it != end(); it++, otherIt++ )
				{
					*it = *otherIt;
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
			this->clear();
			
			m_array = other.m_array;
			m_size = other.m_size;
			
			other.m_array = nullptr;
			other.m_size = 0;
			
			return *this;
		}
		
		/** Ctor to init from stream.
		 @param input is expected to be binary and writen with persist() */
		Array( ifstream& input )
		{
			input.read( reinterpret_cast< char* >( &m_size ), sizeof( uint ) );
			m_array = A().allocate( m_size );
			initArray( input );
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
				for( auto iter = begin(); iter != end(); iter++  )
				{
					A().destroy( iter );
				}
				
				// Debug
// 				{
// 					cout << "Deallocating. Before: " << AllocStatistics::totalAllocated() << endl;
// 				}
				
				A().deallocate( m_array );
				
				// Debug
// 				{
// 					cout << "After: " << AllocStatistics::totalAllocated() << endl << endl;
// 				}
				
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
		
		string toString() const
		{
			stringstream ss; ss << "size: " << m_size << " addr: " << m_array;
		
			#ifdef DETAILED_STREAM
				out << endl;
				for( int i = 0; i < m_size; ++i )
				{
					out << m_array[ i ] << endl;
				}
			#endif
			return ss.str();
		}
		
		uint size() const { return m_size; }
		
		bool empty() const { return m_size == 0; }
		
		T* data() { return m_array; }
		
		const T* data() const { return m_array; }
		
		iterator begin() noexcept { return m_array; }
		iterator end() noexcept { return m_array + m_size; }
		const_iterator begin() const noexcept { return m_array; }
		const_iterator end() const noexcept { return m_array + m_size; }
		
		// Binary persistence. Structure: | array size | contents |
		void persist( ostream& out ) const
		{
			out.write( reinterpret_cast< const char* >( &m_size ), sizeof( uint ) );
			for( const T& content : *this )
			{
				content.persist( out );
			}
		}
		
		template< typename Type, typename Alloc >
		friend ostream& operator<<( ostream& out, const Array< Type, Alloc >& array );
		
// 		template< typename Type, typename Alloc >
// 		friend ostream& operator<<( ostream& out, const Array< shared_ptr< Type >, Alloc >& array );
		
	private:
		/** Inits the internal array. */
		void initArray()
		{
			for( auto iter = begin(); iter != end(); iter++ )
			{
				A().construct( iter );
			}
		}
		
		void initArray( ifstream& input )
		{
			for( auto iter = begin(); iter != end(); iter++ )
			{
				A().construct( iter, input );
			}
		}
		
		/** Inits the internal array. */
		void initArray( const T& initValue )
		{
			for( auto iter = begin(); iter != end(); iter++ )
			{
				A().construct( iter, initValue );
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
		out << array.toString();
		
		return out;
	}
	
// 	template< typename Type, typename Alloc >
// 	ostream& operator<<( ostream& out, const Array< shared_ptr< Type >, Alloc >& array )
// 	{
// 		out << " addr: " << &array << "size: " << array.size() << " data addr: " << array.m_array;
// 		
// 		#ifdef DETAILED_STREAM
// 			out << endl;
// 			for( int i = 0; i < array.m_size; ++i )
// 			{
// 				out << *array[ i ] << endl;
// 			}
// 		#endif
// 		
// 		return out;
// 	}
}

#undef DETAILED_STREAM
#undef DEBUG

#endif