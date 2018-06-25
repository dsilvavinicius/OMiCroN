#ifndef IMEMORY_POOL_H
#define IMEMORY_POOL_H

namespace omicron
{
	/** Interface for memory pools which manage memory for a given type.
	 * @param T is the managed type. */
	template< typename T >
	class IMemoryPool
	{
	public:
		virtual ~IMemoryPool(){}
		
		/** Allocates memory for T */
		virtual T* allocate() = 0;
		
		/** Allocates an array of type T.
		 * @param size is the size IN BYTES of the final array. */
		virtual T* allocateArray( const size_t& size ) = 0;
		
		/** Deallocates a pointer p previously allocated by this pool. */
		virtual void deallocate( T* p ) = 0;
		
		/** Deallocates a pointer to array p previously allocated by this pool. */
		virtual void deallocateArray( T* p) = 0;
		
		/** Calculates how much memory blocks are currently used. */
		virtual size_t usedBlocks() const = 0;
		
		/** Calculates how much memory is currently used in this pool in bytes. */
		virtual size_t memoryUsage() const = 0;
	};
}

#endif
