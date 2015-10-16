#ifndef IMEMORY_MANAGER_H
#define IMEMORY_MANAGER_H

#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include "BasicTypes.h"

using namespace std;

namespace model
{
	template< typename T >
	class ManagedAllocator;
	
	class Point;
	using PointPtr = shared_ptr< Point >;
	using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
	
	class ExtendedPoint;
	using ExtendedPointPtr = shared_ptr< ExtendedPoint >;
	using ExtendedPointVector = vector< ExtendedPointPtr, ManagedAllocator< ExtendedPointPtr > >;
	
	using Index = uint;
	using IndexVector = vector< Index, ManagedAllocator< Index > >;
	using IndexVectorPtr = shared_ptr< IndexVector >;
	
	template< typename T >
	class MortonCode;
	using ShallowMortonCode = MortonCode< unsigned int >;
	using ShallowMortonCodePtr = shared_ptr< ShallowMortonCode >;
	using MediumMortonCode = MortonCode< unsigned long >;
	using MediumMortonCodePtr = shared_ptr< MediumMortonCode >;
	
	template< typename C >
	class InnerNode;
	template< typename C >
	using InnerNodePtr = shared_ptr< InnerNode< C > >;
	
	template< typename C >
	class LeafNode;
	template< typename C >
	using LeafNodePtr = shared_ptr< LeafNode< C > >;
	
	class OctreeNode;
	using OctreeNodePtr = shared_ptr< OctreeNode >;
	
	// shared_ptr< T > internal allocated type.
	template< typename T, typename Alloc >
	using PtrInternals = std::_Sp_counted_ptr_inplace< T, Alloc, (__gnu_cxx::_Lock_policy)2 >;
	
	template< typename T >
	using BitMapPtrInternals = PtrInternals< T, ManagedAllocator< T > >;
	
	template< typename T >
	using DefaultPtrInternals = PtrInternals< T, std::allocator< T > >;
	
	// map< MortonPtr, OctreeNodePtr > internal allocated type.
	template< typename Morton >
	using MapInternals = std::_Rb_tree_node< std::pair< shared_ptr< Morton > const, OctreeNodePtr > >;
	
	// Macro that declares everything in IMemoryManager related with a given memory pool type.
	#define DECLARE_POOL_INTERFACE(TYPE) \
	virtual void* alloc##TYPE() = 0; \
	\
	virtual void* alloc##TYPE##Array( const size_t& size ) = 0;\
	\
	virtual void dealloc##TYPE( void* p ) = 0;\
	\
	virtual void dealloc##TYPE##Array( void* p ) = 0;\
	\
	size_t num##TYPE##Blocks() const;
	
	/** Interface for MemoryManagers. It defines an API for octree morton code, point and node allocation, deallocation
	 * and usage statistics. */
	class IMemoryManager
	{
	public:
		/** Generic allocation that chooses which allocation method to call based on type. */
		template< typename T >
		T* alloc();
		
		/** Generic array allocation that chooses which allocation method to call based on type. */
		template< typename T >
		T* allocArray( const size_t& size );
		
		/** Generic deallocation that chooses what allocation method to call based on type. */
		template< typename T >
		void dealloc( T* p );
		
		/** Generic array deallocation that chooses which allocation method to call based on type. */
		template< typename T >
		void deallocArray( T* p );
		
		DECLARE_POOL_INTERFACE(Morton)
		DECLARE_POOL_INTERFACE(MortonPtr)
		DECLARE_POOL_INTERFACE(MortonPtrInternals)
		
		DECLARE_POOL_INTERFACE(Index)
		
		DECLARE_POOL_INTERFACE(Point)
		DECLARE_POOL_INTERFACE(PointPtr)
		DECLARE_POOL_INTERFACE(PointPtrInternals)
		
		DECLARE_POOL_INTERFACE(Inner)
		DECLARE_POOL_INTERFACE(InnerPtr)
		DECLARE_POOL_INTERFACE(InnerPtrInternals)
		
		DECLARE_POOL_INTERFACE(Leaf)
		DECLARE_POOL_INTERFACE(LeafPtr)
		DECLARE_POOL_INTERFACE(LeafPtrInternals)
		
		DECLARE_POOL_INTERFACE(MapInternals)
		
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
	// MortonCodePtr specializations
	// =========================
	
	SPECIALIZE_ALLOC_DEALLOC(ShallowMortonCodePtr,MortonPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(MediumMortonCodePtr,MortonPtr)
	
	// =======================================
	// MortonCodePtr internals specializations
	// =======================================
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< ShallowMortonCode >,MortonPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< MediumMortonCode >,MortonPtrInternals)
	
	// =======================================
	// Index specializations
	// =======================================
	
	SPECIALIZE_ALLOC_DEALLOC(Index,Index)
	
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
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< Point >,PointPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< ExtendedPoint >,PointPtrInternals)
	
	// ==================================
	// map internals specializations
	// ==================================
	
	SPECIALIZE_ALLOC_DEALLOC(MapInternals< ShallowMortonCode >,MapInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(MapInternals< MediumMortonCode >,MapInternals)
	
	// =========================
	// InnerNode specializations
	// =========================
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNode< IndexVector >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNode< PointPtr >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNode< ExtendedPointPtr >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNode< PointVector >,Inner)
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNode< ExtendedPointVector >,Inner)
	
	// ============================
	// InnerNodePtr specializations
	// ============================
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNodePtr< IndexVector >,InnerPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNodePtr< PointPtr >,InnerPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNodePtr< ExtendedPointPtr >,InnerPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNodePtr< PointVector >,InnerPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(InnerNodePtr< ExtendedPointVector >,InnerPtr)
	
	// ======================================
	// InnerNodePtr internals specializations
	// ======================================
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< InnerNode< IndexVector > >,InnerPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< InnerNode< PointPtr > >,InnerPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< InnerNode< ExtendedPointPtr > >,InnerPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< InnerNode< PointVector > >,InnerPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< InnerNode< ExtendedPointVector > >,InnerPtrInternals)
	
	// ========================
	// LeafNode specializations
	// ========================
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNode< IndexVector >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNode< PointPtr >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNode< ExtendedPointPtr >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNode< PointVector >,Leaf)
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNode< ExtendedPointVector >,Leaf)
	
	// ============================
	// LeafNodePtr specializations
	// ============================
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNodePtr< IndexVector >,LeafPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNodePtr< PointPtr >,LeafPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNodePtr< ExtendedPointPtr >,LeafPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNodePtr< PointVector >,LeafPtr)
	
	SPECIALIZE_ALLOC_DEALLOC(LeafNodePtr< ExtendedPointVector >,LeafPtr)
	
	// ======================================
	// LeafNodePtr internals specializations
	// ======================================
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< LeafNode< IndexVector > >,LeafPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< LeafNode< PointPtr > >,LeafPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< LeafNode< ExtendedPointPtr > >,LeafPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< LeafNode< PointVector > >,LeafPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(BitMapPtrInternals< LeafNode< ExtendedPointVector > >,LeafPtrInternals)
	
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