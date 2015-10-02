#ifndef IMEMORY_MANAGER_H
#define IMEMORY_MANAGER_H

#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include "BasicTypes.h"

using namespace std;

namespace model
{
	template< typename T >
	class BitMapAllocator;
	
	class Point;
	using PointPtr = shared_ptr< Point >;
	using PointVector = vector< PointPtr >;
	
	class ExtendedPoint;
	using ExtendedPointPtr = shared_ptr< ExtendedPoint >;
	using ExtendedPointVector = vector< ExtendedPointPtr >;
	
	using IndexVector = vector< uint >;
	
	template< typename T >
	class MortonCode;
	using ShallowMortonCode = MortonCode< unsigned int >;
	using MediumMortonCode = MortonCode< unsigned long >;
	
	template< typename M, typename C >
	class InnerNode;
	
	template< typename C >
	using ShallowInnerNode = InnerNode< ShallowMortonCode, C >;
	
	template< typename C >
	using MediumInnerNode = InnerNode< MediumMortonCode, C >;
	
	template< typename M, typename C >
	class LeafNode;
	
	template< typename C >
	using ShallowLeafNode = LeafNode< ShallowMortonCode, C >;
	
	template< typename C >
	using MediumLeafNode = LeafNode< MediumMortonCode, C >;
	
	// shared_ptr< Point > internal allocated type.
	template< typename Point >
	using PtrInternals = std::_Sp_counted_ptr_inplace< Point, BitMapAllocator< Point >, (__gnu_cxx::_Lock_policy)2 >;
	
	/** Interface for MemoryManagers. It defines an API for octree morton code, point and node allocation, deallocation
	 * and usage statistics. */
	class IMemoryManager
	{
	public:
		/** Generic allocation method that chooses what allocation method to call based on type. */
		template< typename T >
		T* alloc();
		
		/** Generic array allocation method that chooses what allocation method to call based on type. */
		template< typename T >
		T* allocArray( const size_t& size );
		
		/** Generic deallocation method that chooses what allocation method to call based on type. */
		template< typename T >
		void dealloc( T* p );
		
		/** Generic array deallocation method that chooses what allocation method to call based on type. */
		template< typename T >
		void deallocArray( T* p );
		
		/** Allocates memory for morton code type. */
		virtual void* allocMorton() = 0;
		
		/** Allocates memory for point type. */
		virtual void* allocPoint() = 0;
		
		/** Allocates memory for shared_ptr< point type > */
		virtual void* allocPointPtr() = 0;
		
		/** Allocates memory for shared_ptr< point type > internal data structure. */
		virtual void* allocPtrInternals() = 0;
		
		/** Allocates memory for inner node type. */
		virtual void* allocInner() = 0;
		
		/** Allocates memory for leaf node type. */
		virtual void* allocLeaf() = 0;
		
		/** Allocates memory for morton code type array. */
		virtual void* allocMortonArray( const size_t& size ) = 0;
		
		/** Allocates memory for point type array. */
		virtual void* allocPointArray( const size_t& size ) = 0;
		
		/** Allocates memory for shared_ptr< point type > array. */
		virtual void* allocPointPtrArray( const size_t& size ) = 0;
		
		/** Allocates memory for shared_ptr< point type > interl data structure array. */
		virtual void* allocPtrInternalsArray( const size_t& size ) = 0;
		
		/** Allocates memory for inner node type array. */
		virtual void* allocInnerArray( const size_t& size ) = 0;
		
		/** Allocates memory for leaf node type array. */
		virtual void* allocLeafArray( const size_t& size ) = 0;
		
		/** Deallocates memory for morton code type. */
		virtual void deallocMorton( void* p ) = 0;
		
		/** Deallocates memory for point type. */
		virtual void deallocPoint( void* p ) = 0;
		
		/** Deallocates memory for shared_ptr< point type >. */
		virtual void deallocPointPtr( void* p ) = 0;
		
		/** Deallocates memory for shared_ptr< point type > internal data structure. */
		virtual void deallocPtrInternals( void* p ) = 0;
		
		/** Deallocates memory for inner node type. */
		virtual void deallocInner( void* p ) = 0;
		
		/** Deallocates memory for leaf node type. */
		virtual void deallocLeaf( void* p ) = 0;
		
		/** Deallocates an array of morton code type. */
		virtual void deallocMortonArray( void* p ) = 0;
		
		/** Deallocates an array of point type. */
		virtual void deallocPointArray( void* p ) = 0;
		
		/** Deallocates an array of shared_ptr< point type >. */
		virtual void deallocPointPtrArray( void* p ) = 0;
		
		/** Deallocates an array of shared_ptr< point type > internal data structures. */
		virtual void deallocPtrInternalsArray( void* p ) = 0;
		
		/** Deallocates an array of inner node type. */
		virtual void deallocInnerArray( void* p ) = 0;
		
		/** Deallocates an array of leaf node type. */
		virtual void deallocLeafArray( void* p ) = 0;
		
		/** Reports all memory currently being used. */
		virtual size_t usedMemory() const = 0;
		
		/** Reports the maximum amount of allowed memory. */
		virtual size_t maxAllowedMem() const = 0;
		
		/** Verifies if the free memory is above the passed percentage threshold. */
		virtual bool hasEnoughMemory( const float& percentageThreshold ) const = 0;
		
		/** Outputs a string info representing the state of the manager. */
		virtual string toString() const = 0;
	};
	
	inline ostream& operator<<( ostream& out, const IMemoryManager& manager )
	{
		out << manager.toString();
		return out;
	}
	
	// Macro to create all alloc dealloc methods for a given type.
	#define SPECIALIZE_ALLOC_DEALLOC(TYPE,METHOD_ID) \
	template<> \
	inline TYPE* IMemoryManager::alloc< TYPE >() \
	{ \
		return static_cast< TYPE* >( alloc##METHOD_ID() ); \
	} \
	\
	template<> \
	inline TYPE* IMemoryManager::allocArray< TYPE >( const size_t& size ) \
	{ \
		return static_cast< TYPE* >( alloc##METHOD_ID##Array( size ) ); \
	} \
	\
	template<> \
	inline void IMemoryManager::dealloc< TYPE >( TYPE* p ) \
	{ \
		dealloc##METHOD_ID( p ); \
	} \
	\
	template<> \
	inline void IMemoryManager::deallocArray< TYPE >( TYPE* p ) \
	{ \
		dealloc##METHOD_ID##Array( p ); \
	}
	
	// =========================
	// MortonCode specializations
	// =========================
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowMortonCode,Morton)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumMortonCode,Morton)
	
	// =========================
	// Point specializations
	// =========================
	
	SPECIALIZE_ALLOC_DEALLOC(Point,Point)
	
	SPECIALIZE_ALLOC_DEALLOC(ExtendedPoint,Point)
	
	// ==================================
	// PointPtr specializations
	// ==================================
	
	SPECIALIZE_ALLOC_DEALLOC(PointPtr,PointPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(ExtendedPointPtr,PointPtr)
	
	// ==================================
	// PointPtr internals specializations
	// ==================================
	
	SPECIALIZE_ALLOC_DEALLOC(PtrInternals< Point >,PtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(PtrInternals< ExtendedPoint >,PtrInternals)
	
	// =========================
	// InnerNode specializations
	// =========================
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowInnerNode< IndexVector >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowInnerNode< PointPtr >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowInnerNode< ExtendedPointPtr >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowInnerNode< PointVector >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowInnerNode< ExtendedPointVector >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumInnerNode< IndexVector >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumInnerNode< PointPtr >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumInnerNode< ExtendedPointPtr >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumInnerNode< PointVector >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumInnerNode< ExtendedPointVector >,Inner)
	
	// ========================
	// LeafNode specializations
	// ========================
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowLeafNode< IndexVector >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowLeafNode< PointPtr >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowLeafNode< ExtendedPointPtr >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowLeafNode< PointVector >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowLeafNode< ExtendedPointVector >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumLeafNode< IndexVector >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumLeafNode< PointPtr >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumLeafNode< ExtendedPointPtr >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumLeafNode< PointVector >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumLeafNode< ExtendedPointVector >,Leaf)
	
	/** Provides suport for a singleton IMemoryManager. The derived class has the responsibility of initializing the
	 * singleton instance. */
	class SingletonMemoryManager
	: public IMemoryManager
	{
	public:
		static IMemoryManager& instance();
	
	protected:
		static unique_ptr< IMemoryManager > m_instance;
	};
	
	inline IMemoryManager& SingletonMemoryManager::instance(){ return *m_instance; }
}

#endif