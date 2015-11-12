#ifndef IMEMORY_MANAGER_H
#define IMEMORY_MANAGER_H

#include <cstdlib>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <iostream>
#include "BasicTypes.h"

using namespace std;

namespace model
{
	template< typename T >
	class DefaultManagedAllocator;
	
	template< typename T >
	using ManagedAllocator = DefaultManagedAllocator< T >;
	
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
	class OctreeNode;
	template< typename C >
	using OctreeNodePtr = shared_ptr< OctreeNode< C > >;
	
	// shared_ptr< T > internal allocated type.
	template< typename T, typename Alloc >
	using PtrInternals = std::_Sp_counted_ptr_inplace< T, Alloc, (__gnu_cxx::_Lock_policy)2 >;
	
	template< typename T >
	using DefaultPtrInternals = PtrInternals< T, std::allocator< T > >;
	
	template< typename T >
	using ManagedPtrInternals = PtrInternals< T, ManagedAllocator< T > >;
	
	// map< MortonPtr, OctreeNodePtr > internal allocated type.
	template< typename Morton, typename Node >
	using MapInternals = std::_Rb_tree_node< std::pair< shared_ptr< Morton > const, shared_ptr< Node > > >;
	
	using SPVMapInternals = MapInternals< ShallowMortonCode, OctreeNode< PointVector > >;
	using MPVMapInternals = MapInternals< MediumMortonCode, OctreeNode< PointVector > >;
	using SEVMapInternals = MapInternals< ShallowMortonCode, OctreeNode< ExtendedPointVector > >;
	using MEVMapInternals = MapInternals< MediumMortonCode, OctreeNode< ExtendedPointVector > >;
	using SIVMapInternals = MapInternals< ShallowMortonCode, OctreeNode< IndexVector > >;
	using MIVMapInternals = MapInternals< MediumMortonCode, OctreeNode< IndexVector > >;
	
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
		
		virtual ~IMemoryManager(){}
		
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
		
		DECLARE_POOL_INTERFACE(Node)
		DECLARE_POOL_INTERFACE(NodePtr)
		DECLARE_POOL_INTERFACE(NodePtrInternals)
		
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
	
	SPECIALIZE_ALLOC_DEALLOC(ManagedPtrInternals< ShallowMortonCode >,MortonPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(ManagedPtrInternals< MediumMortonCode >,MortonPtrInternals)
	
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
	
	SPECIALIZE_ALLOC_DEALLOC(ManagedPtrInternals< Point >,PointPtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(ManagedPtrInternals< ExtendedPoint >,PointPtrInternals)
	
	// ==================================
	// map internals specializations
	// ==================================
	
	SPECIALIZE_ALLOC_DEALLOC(SPVMapInternals,MapInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(MPVMapInternals,MapInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(SEVMapInternals,MapInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(MEVMapInternals,MapInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(SIVMapInternals,MapInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(MIVMapInternals,MapInternals)
	
	// =========================
	// Node specializations
	// =========================
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNode< IndexVector >,Node)
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNode< PointPtr >,Node)
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNode< ExtendedPointPtr >,Node)
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNode< PointVector >,Node)
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNode< ExtendedPointVector >,Node)
	
	// ============================
	// InnerNodePtr specializations
	// ============================
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNodePtr< IndexVector >,NodePtr)
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNodePtr< PointPtr >,NodePtr)
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNodePtr< ExtendedPointPtr >,NodePtr)
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNodePtr< PointVector >,NodePtr)
	
	SPECIALIZE_ALLOC_DEALLOC(OctreeNodePtr< ExtendedPointVector >,NodePtr)
	
	// ======================================
	// InnerNodePtr internals specializations
	// ======================================
	
	SPECIALIZE_ALLOC_DEALLOC(ManagedPtrInternals< OctreeNode< IndexVector > >,NodePtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(ManagedPtrInternals< OctreeNode< PointPtr > >,NodePtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(ManagedPtrInternals< OctreeNode< ExtendedPointPtr > >,NodePtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(ManagedPtrInternals< OctreeNode< PointVector > >,NodePtrInternals)
	
	SPECIALIZE_ALLOC_DEALLOC(ManagedPtrInternals< OctreeNode< ExtendedPointVector > >,NodePtrInternals)
	
	/** Provides suport for a singleton IMemoryManager. The derived class has the responsibility of initializing the
	 * singleton instance. */
	class SingletonMemoryManager
	: public IMemoryManager
	{
	public:
		virtual ~SingletonMemoryManager(){}
		
		static IMemoryManager& instance();
	
	protected:
		static unique_ptr< IMemoryManager > m_instance;
	};
	
	inline IMemoryManager& SingletonMemoryManager::instance(){ return *m_instance; }
}

#endif