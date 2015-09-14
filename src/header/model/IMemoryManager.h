#ifndef IMEMORY_MANAGER_H
#define IMEMORY_MANAGER_H

#include <cstdlib>
#include <string>
#include <memory>

using namespace std;

namespace model
{
	/** Interface for MemoryManagers. */
	class IMemoryManager
	{
	public:
		enum MANAGED_TYPE_FLAG
		{
			SHALLOW_MORTON,
			MEDIUM_MORTON,
			POINT,
			EXTENDED_POINT,
			NODE,
			COUNT
		};
		
		/** Allocates memory for a managed type. */
		virtual void* allocate( const MANAGED_TYPE_FLAG& type ) = 0;
		
		/** Allocates memory for a managed type array. */
		virtual void* allocateArray( const size_t& size, const MANAGED_TYPE_FLAG& type ) = 0;
		
		/** Deallocates memory for a managed type. */
		virtual void deallocate( void* p, const MANAGED_TYPE_FLAG& type ) = 0;
		
		virtual void deallocateArray( void* p, const MANAGED_TYPE_FLAG& type ) = 0;
		
		/** Verifies if the free memory is above the passed percentage threshold. */
		virtual bool hasEnoughMemory( const float& percentageThreshold ) const = 0;
		
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