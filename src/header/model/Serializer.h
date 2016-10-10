#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <string.h>
#include "Stream.h"
#include "MemoryUtils.h"

namespace model
{
	/** Serialization and deserialization routines. */
	class Serializer
	{
		template< typename T >
		using Vector = vector< T, ManagedAllocator< T > >;
		
	public:
		/** Serializes an vector. The form is:
		 *	size_t with the size of the vector s;
		 *	s T elements.
		 *	Default implementation assumes vector contents POD (plain old data). Specialize this method for more specific
		 *	behavior.
		 *	@param T is the vector content type. */
		template< typename T >
		static size_t serialize( const Vector< T >& vector , byte** serialization );
		
		/** Deserializes an byte sequence created by serialize() into a vector. Default implementation assumes vector contents POD
		 *	(plain old data). Specialize this method for more specific behavior.
		 *	@param T is the vector content type. */
		template< typename T >
		static void deserialize( byte* serialization, Vector< T >& out );
		
		/** Creates a byte array for serialization purposes. */
		static byte* newByteArray( const size_t& size ) { return new byte[ size ]; }
		
		/** Disposes the byte sequence created by newByteArray(). */
		static void dispose( byte* serialization ) { delete[] serialization; }
	};
	
	template< typename T >
	inline size_t Serializer::serialize( const Vector< T >& vector , byte** serialization )
	{
		size_t count = vector.size();
		size_t countSize = sizeof( size_t );
		size_t elemSize = sizeof( T );
		size_t vecSize = count * elemSize;
		size_t serializationSize = countSize + vecSize;
		
		*serialization = newByteArray( serializationSize );
		byte* tempPointer = *serialization;
		
		memcpy( tempPointer, &count, countSize );
		tempPointer += countSize;
		memcpy( tempPointer, vector.data(), vecSize );
		
		return serializationSize;
	}
	
	template<>
	inline size_t Serializer::serialize< PointPtr >( const PointVector& vector , byte** serialization )
	{
		size_t count = vector.size();
		size_t countSize = sizeof( size_t );
		
		if( count > 0 )
		{
			byte* pointBytes;
			size_t elemSize = vector[ 0 ]->serialize( &pointBytes );
			
			size_t vecSize = count * elemSize;
			size_t serializationSize = countSize + vecSize;
			
			*serialization = newByteArray( serializationSize );
			byte* tempPointer = *serialization;
			memcpy( tempPointer, &count, countSize );
			tempPointer += countSize;
			memcpy( tempPointer, pointBytes, elemSize );
			
			for( int i = 1; i < vector.size(); ++i )
			{
				PointPtr point = vector[ i ];
				tempPointer += elemSize;
				Serializer::dispose( pointBytes );
				point->serialize( &pointBytes );
				memcpy( tempPointer, pointBytes, elemSize );
			}
			
			Serializer::dispose( pointBytes );
			return serializationSize;
		}
		else
		{
			size_t serializationSize = countSize;
			*serialization = newByteArray( serializationSize );
			memcpy( *serialization, &count, countSize );
			
			return serializationSize;
		}
	}
	
	template< typename T >
	inline void Serializer::deserialize( byte* serialization, Vector< T >& out )
	{
		size_t count;
		size_t countSize = sizeof( size_t );
		memcpy( &count, serialization, countSize );
		
		size_t vecSize = count * sizeof( T );
		T* array = ( T* ) malloc( vecSize );
		memcpy( array, serialization + countSize, vecSize );
		out = Vector< T >( array, array + count );
		
		free( array );
	}
	
	template<>
	inline void Serializer::deserialize< PointPtr >( byte* serialization, PointVector& out )
	{
		size_t count;
		size_t countSize = sizeof( size_t );
		memcpy( &count, serialization, countSize );
		
		byte* tempPtr0 = serialization + countSize;
		byte* tempPtr1 = nullptr;
		
		for( int i = 0; i < count; ++i )
		{
			Point p( tempPtr0, tempPtr1 );
			out.push_back( makeManaged< Point >( p ) );
			swap( tempPtr0, tempPtr1 );
		}
	}
}

#endif