#ifndef IMEMORY_MANAGER_H
#define IMEMORY_MANAGER_H

#include <cstdlib>
#include <string>
#include <memory>

using namespace std;

namespace model
{
	/** Interface for MemoryManagers. It defines an API for octree morton code, point and node allocation, deallocation
	 * and usage statistics. */
	class IMemoryManager
	{
	public:
		/** Allocates memory for morton code type. */
		virtual void* allocMorton() = 0;
		
		/** Allocates memory for point type. */
		virtual void* allocPoint() = 0;
		
		/** Allocates memory for inner node type. */
		virtual void* allocInner() = 0;
		
		/** Allocates memory for leaf node type. */
		virtual void* allocLeaf() = 0;
		
		/** Allocates memory for morton code type array. */
		virtual void* allocMortonArray( const size_t& size ) = 0;
		
		/** Allocates memory for point type array. */
		virtual void* allocPointArray( const size_t& size ) = 0;
		
		/** Allocates memory for inner node type array. */
		virtual void* allocInnerArray( const size_t& size ) = 0;
		
		/** Allocates memory for leaf node type array. */
		virtual void* allocLeafArray( const size_t& size ) = 0;
		
		/** Deallocates memory for morton code type. */
		virtual void deallocMorton( void* p ) = 0;
		
		/** Deallocates memory for point type. */
		virtual void deallocPoint( void* p ) = 0;
		
		/** Deallocates memory for inner node type. */
		virtual void deallocInner( void* p ) = 0;
		
		/** Deallocates memory for leaf node type. */
		virtual void deallocLeaf( void* p ) = 0;
		
		/** Deallocates an array of morton code type. */
		virtual void deallocMortonArray( void* p ) = 0;
		
		/** Deallocates an array of point type. */
		virtual void deallocPointArray( void* p ) = 0;
		
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