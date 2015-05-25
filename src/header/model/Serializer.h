#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <string.h>
#include "LeafNode.h"
#include "InnerNode.h"
#include "Stream.h"

namespace model
{
	/** Serialization and deserialization routines. */
	class Serializer
	{
	public:
		/** Serializes an vector. The form is:
		 *	size_t with the size of the vector s;
		 *	s T elements.
		 *	@param T is the vector content type. */
		template< typename T >
		static size_t serialize( vector< T >& vector , byte** serialization );
		
		/** Deserializes an byte sequence created by serialize() into a vector.
		 *	@param T is the vector content type. */
		template< typename T >
		static void deserialize( byte* serialization, vector< T >& out );
		
		/** Disposes the byte sequence created by serialize(). */
		static void dispose( byte* serialization ) { delete[] serialization; }
	};
	
	template< typename T >
	inline size_t Serializer::serialize( vector< T >& vector , byte** serialization )
	{
		size_t count = vector.size();
		size_t countSize = sizeof( size_t );
		size_t elemSize = sizeof( T );
		size_t vecSize = count * elemSize;
		size_t serializationSize = countSize + vecSize;
		
		*serialization = new byte[ serializationSize ];
		byte* tempPointer = *serialization;
		
		memcpy( tempPointer, &count, countSize );
		tempPointer += countSize;
		memcpy( tempPointer, vector.data(), vecSize );
		
		return serializationSize;
	}
	
	template< typename T >
	inline void Serializer::deserialize( byte* serialization, vector< T >& out )
	{
		size_t count;
		size_t countSize = sizeof( size_t );
		memcpy( &count, serialization, countSize );
		
		cout << "count: " << count << endl;
		
		size_t vecSize = count * sizeof( T );
		T* array = ( T* ) malloc( vecSize );
		memcpy( array, serialization + countSize, vecSize );
		out = vector< T >( array, array + count );
		
		free( array );
	}
}

#endif