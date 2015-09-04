#ifndef BITMAP_MEMORY_POOL_H
#define BITMAP_MEMORY_POOL_H

#include <cstring>
#include <vector>
#include <set>
#include <map>
#include <bitset>

using namespace std;

namespace model
{
	const int BIT_MAP_SIZE = 1024;
	const int INT_SIZE = sizeof( int ) * 8;
	const int BIT_MAP_ELEMENTS = BIT_MAP_SIZE / INT_SIZE;

	//Memory Allocation Pattern
	//11111111 11111111 11111111
	//11111110 11111111 11111111
	//11111100 11111111 11111111
	//if all bits for 1st section become 0 proceed to next section

	//...
	//00000000 11111111 11111111
	//00000000 11111110 11111111
	//00000000 11111100 11111111
	//00000000 11111000 11111111

	//The reason for this strategy is that lookup becomes O(1) inside the map 
	//for the first available free block

	class BitMapMemoryPool;
	
	typedef struct BitMapEntry
	{
	public:
		BitMapEntry( BitMapMemoryPool& pool )
		: m_pool( pool ),
		BlocksAvailable( BIT_MAP_SIZE )
		{
			memset( BitMap, 0xff, BIT_MAP_SIZE / sizeof( char ) ); 
			// initially all blocks are free and bit value 1 in the map denotes 
			// available block
		}
		void SetBit( int position, bool flag );
		void SetMultipleBits( int position, bool flag, int count );
		void SetRangeOfInt( int* element, int msb, int lsb, bool flag );
		void* FirstFreeBlock( size_t size );
		void* objectAddress( int pos );
		void* Head();
	
		int Index;
		int BlocksAvailable;
		int BitMap[ BIT_MAP_SIZE ];
		BitMapMemoryPool& m_pool;
	}
	BitMapEntry;

	typedef struct ArrayInfo
	{
		int   MemPoolListIndex;
		int   StartPosition;
		int   Size;
	}
	ArrayMemoryInfo;
	
	class BitMapMemoryPool
	{
	public: 
		BitMapMemoryPool( int objSize )
		: objectSize( objSize )
		{}
		~BitMapMemoryPool( ) {}
		void* allocate();
		void* allocateArray( const size_t& size );
		bool deallocate( void* p );
		vector< void* >& GetMemoryPoolList(); 
		int getObjectSize(){ return objectSize; }
		
	private:
		void* AllocateArrayMemory( size_t size );
		void* AllocateChunkAndInitBitMap();
		bool SetBlockBit( void* object, bool flag );
		void SetMultipleBlockBits( ArrayMemoryInfo* info, bool flag );
		
		vector< void* > MemoryPoolList;
		vector< BitMapEntry > BitMapEntryList;
		//the above two lists will maintain one-to-one correpondence and hence 
		//should be of same  size.
		set< BitMapEntry* > FreeMapEntries;
		map< void*, ArrayMemoryInfo > ArrayMemoryList;
		int objectSize;
	};
}

#endif