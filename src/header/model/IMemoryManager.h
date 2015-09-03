#ifndef IMEMORY_MANAGER_H
#define IMEMORY_MANAGER_H

#include <cstdlib>
#include <string>

using namespace std;

namespace model
{
	/** Interface for MemoryManagers. */
	class IMemoryManager
	{
	public:
		/** Allocates memory for a managed type. */
		virtual void* allocate( const size_t& size ) = 0;
		
		/** Deallocates memory for a managed type. */
		virtual void deallocate( void* p ) = 0;
		
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
		static IMemoryManager& instance(){ return *m_instance; }
		static void deleteInstance(){ delete m_instance; }
	
	protected:
		static IMemoryManager* m_instance;
	};
}

#endif