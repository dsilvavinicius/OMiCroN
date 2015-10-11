#ifndef BITMAP_MEMORY_MANAGER_H
#define BITMAP_MEMORY_MANAGER_H

#include <map>
#include <mutex>
#include <boost/heap/fibonacci_heap.hpp>
#include "IMemoryPool.h"
#include "MemoryManager.h"

using namespace boost::heap;

namespace model
{
	const int BIT_MAP_SIZE = 1024 * 1024 * 10;
	const int INT_SIZE = sizeof( int ) * 8;
	const int BIT_MAP_ELEMENTS = BIT_MAP_SIZE / INT_SIZE;

	/**
	 * Type-safe implementation of http://www.ibm.com/developerworks/aix/tutorials/au-memorymanager/ BitMapEntry.
	 * Any 1 bit indicates a free block. 0 bits indicate the contrary.
	 * 
	 * Memory Allocation Pattern
	 * 11111111 11111111 11111111
	 * 11111110 11111111 11111111
	 * 11111100 11111111 11111111
	 * if all bits for 1st section become 0 proceed to next section
	 *
	 *...
	 * 00000000 11111111 11111111
	 * 00000000 11111110 11111111
	 * 00000000 11111100 11111111
	 * 00000000 11111000 11111111
	 *
	 * The reason for this strategy is that lookup becomes O(1) inside the map 
	 * for the first available free block
	 */
	template< typename T >
	class BitMapEntry
	{
	public:
		BitMapEntry()
		: BlocksAvailable( BIT_MAP_SIZE )
		{
			memset( BitMap, 0xff, BIT_MAP_ELEMENTS * sizeof( int ) ); 
			// initially all blocks are free and bit value 1 in the map denotes 
			// available block
		}
		/** Sets a bit, given a position in the bitmap. */
		void SetBit( int position, bool flag );
		
		/** Same as SetBit, but for more than one consecutive bits. */
		void SetMultipleBits( int position, bool flag, int count );
		
		/** Same as SetBit, but should be used in case of the bits span in more than one BitMap element. */
		void SetRangeOfInt( int* element, int msb, int lsb, bool flag );
		
		/** @returns the position of the first free block associated with this BitMapEntry. */
		int FirstFreeBlockPos();
	
		int BitMap[ BIT_MAP_ELEMENTS ];
		int Index;
		int BlocksAvailable;
	};
	
	template< typename T >
	class ArrayInfo;
	
	template< typename T >
	class ArrayInfoComparator
	{
	public:
		bool operator()( const ArrayInfo< T >& info0, const ArrayInfo< T >& info1 ) const
		{
			return info0.m_size < info1.m_size;
		};
	};
	
	/** Contains info of array allocation requests. */
	template< typename T >
	class ArrayInfo
	{
		using Heap = fibonacci_heap< ArrayInfo< T >, std::allocator< ArrayInfo< T > >, ArrayInfoComparator< T > >;
		using Map = map< T*, typename Heap::handle_type >;
	public:
		ArrayInfo( const uint& size, const typename Map::iterator& it )
		: m_size( size ),
		m_mapIt( it )
		{}
		
		typename Map::iterator m_mapIt;
		uint m_size;
	};
	
	/**
	 * BitMapMemoryPool, which provides a thread-safe type-safe implementation of
	 * http://www.ibm.com/developerworks/aix/tutorials/au-memorymanager/.
	 * @param T is the managed type.
	 */
	template< typename T >
	class BitMapMemoryPool
	: public IMemoryPool< T >
	{
		using BitMapEntry = model::BitMapEntry< T >;
		using ArrayInfo = model::ArrayInfo< T >;
		using Heap = fibonacci_heap< ArrayInfo, std::allocator< ArrayInfo >, ArrayInfoComparator< T > >;
		using Map = map< T*, typename Heap::handle_type >;
		
	public: 
		BitMapMemoryPool()
		{
			m_arrayHeaderSize = sizeof( uint ) / sizeof( T ); // Number of blocks needed to contain the array size header.
			if( sizeof( uint ) % sizeof( T ) != 0 )
			{
				m_arrayHeaderSize += 1;
			};
		}
		
		~BitMapMemoryPool();
		
		T* allocate() override;
		
		T* allocateArray( const size_t& size ) override;
		
		void deallocate( T* p ) override;
		
		void deallocateArray( T* p) override;
		
		size_t usedBlocks() const override;		
		size_t memoryUsage() const override;
		
	private:
		T* AllocateChunkAndInitBitMap();
		
		/** Sets the bit related with the given pointer. */
		void SetBlockBit( T* object, bool flag );
		
		/** @returns the first free block of the given BitMapEntry. */
		T* firstFreeBlock( BitMapEntry* bitmap );
		
		/** @returns the object address of the given position in the memory chunk associated with the given BitMapEntry. */
		T* objectAddress( BitMapEntry* bitmap, int pos );
		
		/** Setups the header of the array.
		 * @returns a pointer to the array after the header, pointing to the first actual array element. */
		T* setHeader( T* array, const uint& arraySize );
		
		/** Allocates chunk for array memory. Setup all needed data in free heap and map. */
		T* allocArrayChunk();
		
		vector< T* >& GetMemoryPoolList();
		
		vector< T* > MemoryPoolList;
		vector< BitMapEntry > BitMapEntryList;
		//the above two lists will maintain one-to-one correpondence and hence 
		//should be of same  size.
		set< BitMapEntry* > FreeMapEntries; // Bitmaps that currently have free entries.
		
		Map m_freeMap; // List of current free memory areas.
		Heap m_freeHeap; // Heap of free memory areas. Used to query allocations fast. 
		uint m_arrayHeaderSize; // Header size for arrays in units of T.
		
		mutex m_poolLock; // Lock to the pool
	};
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	class BitMapMemoryManager
	: public MemoryManager< Morton, Point, Inner, Leaf, BitMapAllocGroup< Morton, Point, Inner, Leaf > >
	{
		using AllocGroup = model::BitMapAllocGroup< Morton, Point, Inner, Leaf >;
		using MemoryManager = model::MemoryManager< Morton, Point, Inner, Leaf, AllocGroup >;
		
		using MortonPtr = shared_ptr< Morton >;
		using MortonPtrInternals = model::PtrInternals< Morton, typename AllocGroup::MortonAlloc >;
		
		using PointPtr = shared_ptr< Point >;
		using PointPtrInternals = model::PtrInternals< Point, typename AllocGroup::PointAlloc >;
		
		using InnerPtr = shared_ptr< Inner >;
		using InnerPtrInternals = model::PtrInternals< Inner, typename AllocGroup::InnerAlloc >;
		
		using LeafPtr = shared_ptr< Leaf >;
		using LeafPtrInternals = model::PtrInternals< Leaf, typename AllocGroup::LeafAlloc >;
		
		using MapInternals = model::MapInternals< Morton >;
	public:
		static void initInstance( const size_t& maxAllowedMem );
	
	private:
		BitMapMemoryManager( const size_t& maxAllowedMem );
	};
	
	template< typename T >
	void BitMapEntry< T >::SetBit( int position, bool flag )
	{
		//cout << "SetBit " << flag << " pos " << position << endl << endl;
		
		BlocksAvailable += flag ? 1 : -1;
		int elementNo = position / INT_SIZE;
		int bitNo = position % INT_SIZE;
		if( flag )
			BitMap[ elementNo ] = BitMap[ elementNo ] | ( 1 << bitNo );
		else
			BitMap[ elementNo ] = BitMap[ elementNo ] & ~( 1 << bitNo ); 
	}

	template< typename T >
	int BitMapEntry< T >::FirstFreeBlockPos()
	{
		for( int i = 0 ; i < BIT_MAP_ELEMENTS; ++i )
		{
			//cout << "Bitmap " << i << ": " << BitMap[ i ] << endl << endl;
			
			if( BitMap[ i ] == 0 )
				continue;            // no free bit was found 
			
			//cout << "Bitmap: 0x" << hex << BitMap[ i ] << dec << endl << endl;
			
			int result = BitMap[ i ] & -( BitMap[ i ] ); // this expression yields the first 
														// bit position which is 1 in an int from right.
			
			//cout << "first bit from right: " << result << endl << endl;
			
			int basePos = ( INT_SIZE * i );
			switch( result )
			{
				//make the corresponding bit 0 meaning block is no longer free
				case 0x00000001: return basePos + 0;
				case 0x00000002: return basePos + 1;
				case 0x00000004: return basePos + 2;
				case 0x00000008: return basePos + 3;
				case 0x00000010: return basePos + 4;
				case 0x00000020: return basePos + 5;
				case 0x00000040: return basePos + 6;
				case 0x00000080: return basePos + 7;
				case 0x00000100: return basePos + 8;
				case 0x00000200: return basePos + 9;
				case 0x00000400: return basePos + 10;
				case 0x00000800: return basePos + 11;
				case 0x00001000: return basePos + 12;
				case 0x00002000: return basePos + 13;
				case 0x00004000: return basePos + 14;
				case 0x00008000: return basePos + 15;
				case 0x00010000: return basePos + 16;
				case 0x00020000: return basePos + 17;
				case 0x00040000: return basePos + 18;
				case 0x00080000: return basePos + 19;
				case 0x00100000: return basePos + 20;
				case 0x00200000: return basePos + 21;
				case 0x00400000: return basePos + 22;
				case 0x00800000: return basePos + 23;
				case 0x01000000: return basePos + 24;
				case 0x02000000: return basePos + 25;
				case 0x04000000: return basePos + 26;
				case 0x08000000: return basePos + 27;
				case 0x10000000: return basePos + 28;
				case 0x20000000: return basePos + 29;
				case 0x40000000: return basePos + 30;
				case 0x80000000: return basePos + 31;
				default : throw logic_error( "Unexpected bit position for allocation." );
			}
		}
		
		throw logic_error( "Cannot allocate memory." );
		return 0;
	}
	
	template< typename T >
	BitMapMemoryPool< T >::~BitMapMemoryPool()
	{
		for( int i = 0; i < MemoryPoolList.size(); ++i )
		{
			delete[] ( char* ) MemoryPoolList[ i ];
		}
	}
	
	template< typename T >
	T* BitMapMemoryPool< T >::allocate()
	{
		lock_guard< mutex > guard( m_poolLock );
		
		typename std::set< BitMapEntry* >::iterator freeMapI = FreeMapEntries.begin();
		if( freeMapI != FreeMapEntries.end() )
		{
			BitMapEntry* mapEntry = *freeMapI;
			T* block = firstFreeBlock( mapEntry );
			
			//cout << "Blocks available: " << mapEntry->BlocksAvailable << endl << endl;
			if( mapEntry->BlocksAvailable == 0 )
			{
				//cout << "Block is not free anymore" << endl << endl;
				FreeMapEntries.erase( freeMapI );
			}
			
			return block;
		}
		else
		{
			//cout << "Free entry NOT found." << endl << endl;
			AllocateChunkAndInitBitMap();
			BitMapEntry* mapEntry = &( BitMapEntryList[ BitMapEntryList.size() - 1 ] );
			FreeMapEntries.insert( mapEntry );
			
			return firstFreeBlock( mapEntry );
		}
	}

	template< typename T >
	T* BitMapMemoryPool< T >::allocateArray( const size_t& size )
	{
		lock_guard< mutex > guard( m_poolLock );
		
		size_t arraySize = size / sizeof( T );
		size_t neededBlocks = arraySize + m_arrayHeaderSize;
		
		assert( neededBlocks <= BIT_MAP_SIZE && "Array allocation: size greater than maximum allowed." );
		
		if( !m_freeHeap.empty() )
		{
			ArrayInfo arrayInfo = m_freeHeap.top();
			if( arrayInfo.m_size >= neededBlocks )
			{
				if( arrayInfo.m_size == neededBlocks )
				{
					// Delete this free entry in heap and map
					m_freeHeap.pop();
					typename Map::iterator mapIt = arrayInfo.m_mapIt;
					T* address = mapIt->first;
					m_freeMap.erase( mapIt );
					
					return setHeader( address, arraySize );
				}
				else
				{
					typename Map::iterator mapIt = arrayInfo.m_mapIt;
					T* address = mapIt->first;
					typename Heap::handle_type handler = mapIt->second;
					
					typename Map::iterator nextIt = m_freeMap.erase( mapIt );
					m_freeMap.insert( nextIt, typename Map::value_type( address + neededBlocks, handler ) );
					arrayInfo.m_size -= neededBlocks;
					m_freeHeap.decrease( handler, arrayInfo );
					return setHeader( mapIt->first, arraySize );
				}
			}
		}
		
		return setHeader( allocArrayChunk(), arraySize );
	}

	template< typename T >
	void BitMapMemoryPool< T >::deallocate( T* object )
	{
		lock_guard< mutex > guard( m_poolLock );
		
		SetBlockBit( object, true );
	}
	
	template< typename T >
	void BitMapMemoryPool< T >::deallocateArray( T* array )
	{
		lock_guard< mutex > guard( m_poolLock );
		
		T* header = array - m_arrayHeaderSize;
		uint arraySize = *( reinterpret_cast< uint* >( header ) ) + m_arrayHeaderSize;
		
		typename Map::iterator nextIt = m_freeMap.upper_bound( header );
		T* nextAddr = nextIt->first;
		uint nextSize = ( *nextIt->second ).m_size;
		
		typename Map::iterator prevIt = std::prev( nextIt );
		T* prevAddr = prevIt->first;
		uint prevSize = ( *prevIt->second ).m_size;
		
		bool canMergePrev = prevAddr + prevSize == header;
		bool canMergeNext = array + arraySize == nextAddr;
		
		if( canMergePrev && canMergeNext )
		{
			// Free area can be merged with previous and next. Next will be deleted and previous will grow.
			ArrayInfo arrayInfo = *prevIt->second;
			arrayInfo.m_size += arraySize + nextSize;
			m_freeHeap.increase( prevIt->second, arrayInfo );
			
			m_freeHeap.erase( nextIt->second );
			m_freeMap.erase( nextIt );
		}
		else if( canMergePrev )
		{
			ArrayInfo arrayInfo = *prevIt->second;
			arrayInfo.m_size += arraySize;
			m_freeHeap.increase( prevIt->second, arrayInfo );
		}
		else if( canMergeNext )
		{
			typename Heap::handle_type nextHeapHandle = nextIt->second;
			typename Map::iterator nextHint = m_freeMap.erase( nextIt );
			m_freeMap.insert( nextHint, typename Map::value_type( nextAddr - arraySize, nextHeapHandle ) );
			
			ArrayInfo arrayInfo = *nextHeapHandle;
			arrayInfo.m_size += arraySize;
			m_freeHeap.increase( nextHeapHandle, arrayInfo );
		}
		else
		{
			// Cannot merge. Just insert a new free entry.
			typename Map::value_type mapEntry( header, typename Heap::handle_type() );
			typename Map::iterator it = m_freeMap.insert( mapEntry ).first;
			ArrayInfo arrayInfo( arraySize, it );
			it->second = m_freeHeap.push( arrayInfo );
		}
	}
	
	template< typename T >
	std::vector< T* >& BitMapMemoryPool< T >::GetMemoryPoolList()
	{ 
		return MemoryPoolList;
	}
	
	template< typename T >
	size_t BitMapMemoryPool< T >::usedBlocks() const
	{
		size_t nUsedBlocks = 0;
		
		for( BitMapEntry bitmap : BitMapEntryList )
		{
			nUsedBlocks += BIT_MAP_SIZE - bitmap.BlocksAvailable;
		}
		
		return nUsedBlocks;
	}
	
	template< typename T >
	size_t BitMapMemoryPool< T >::memoryUsage() const
	{
		return usedBlocks() * sizeof( T );
	}
	
	template< typename T >
	T* BitMapMemoryPool< T >::objectAddress( BitMapEntry* bitmap, int pos )
	{
		bitmap->SetBit( pos, false );
		int elementIdx = pos / INT_SIZE;
		int bitIdx = INT_SIZE - ( ( pos % INT_SIZE ) + 1);
		
		T* address = MemoryPoolList[ bitmap->Index ] + elementIdx * INT_SIZE + bitIdx;
		//cout << "Free pos: " << bitIdx << " address: " << address << endl << endl;
		
		return address;
	}
	
	template< typename T >
	T* BitMapMemoryPool< T >::firstFreeBlock( BitMapEntry* bitmap )
	{
		int pos = bitmap->FirstFreeBlockPos();
		return objectAddress( bitmap, pos );
	}
	
	template< typename T >
	T* BitMapMemoryPool< T >::AllocateChunkAndInitBitMap()
	{
		//cout << "Allocating chunk." << endl << endl;
		BitMapEntry mapEntry;
		T* memoryBeginAddress = reinterpret_cast< T* >( new char [ sizeof( T ) * BIT_MAP_SIZE ] );
		MemoryPoolList.push_back( memoryBeginAddress );
		mapEntry.Index = MemoryPoolList.size() - 1;
		BitMapEntryList.push_back( mapEntry );
		return memoryBeginAddress;
	}
	
	template< typename T >
	inline T* BitMapMemoryPool< T >::setHeader( T* array, const uint& arraySize )
	{
		uint* header = reinterpret_cast< uint* >( array );
		*header = arraySize;
		return array + m_arrayHeaderSize;
	}
	
	template< typename T >
	T* BitMapMemoryPool< T >::allocArrayChunk()
	{
		T* address = reinterpret_cast< T* >( new char [ sizeof( T ) * BIT_MAP_SIZE ] );
		
		typename Map::value_type mapEntry( address, typename Heap::handle_type() );
		typename Map::iterator mapIt = m_freeMap.insert( mapEntry ).first;
		ArrayInfo arrayInfo( BIT_MAP_SIZE, mapIt );
		mapIt->second = m_freeHeap.push( arrayInfo );
		
		return address;
	}

	template< typename T >
	void BitMapMemoryPool< T >::SetBlockBit( T* object, bool flag )
	{
		//cout << "BitMapEntryList size " << BitMapEntryList.size() << endl << endl;
		int i = BitMapEntryList.size() - 1;
		for( ; i >= 0 ; i-- )
		{
			BitMapEntry* bitMap = &BitMapEntryList[ i ];
			T* head = MemoryPoolList[ bitMap->Index ];
			if( ( head <= object ) && ( head + BIT_MAP_SIZE - 1 >= object ) )
			{
				int position = object - head;
				position = ( position / INT_SIZE ) * INT_SIZE + ( INT_SIZE - ( position % INT_SIZE ) - 1 );
				
				//cout << "dealloc ptr: " << object << " pos:" << position << endl << endl;
				
				bitMap->SetBit( position, flag );
				
				if( bitMap->BlocksAvailable == 1 && flag )
				{
					FreeMapEntries.insert( bitMap );
				}
			}
		}
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	void BitMapMemoryManager< Morton, Point, Inner, Leaf >::initInstance( const size_t& maxAllowedMem )
	{
		MemoryManager::m_instance = unique_ptr< IMemoryManager >(
			new BitMapMemoryManager< Morton, Point, Inner, Leaf >( maxAllowedMem )
		);
	}
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	BitMapMemoryManager< Morton, Point, Inner, Leaf >::BitMapMemoryManager( const size_t& maxAllowedMem )
	{
		MemoryManager::m_MortonPool = new BitMapMemoryPool< Morton >();
		MemoryManager::m_MortonPtrPool = new BitMapMemoryPool< MortonPtr >();
		MemoryManager::m_MortonPtrInternalsPool = new BitMapMemoryPool< MortonPtrInternals >();
		
		MemoryManager::m_IndexPool = new BitMapMemoryPool< Index >();
		
		MemoryManager::m_PointPool = new BitMapMemoryPool< Point >();
		MemoryManager::m_PointPtrPool = new BitMapMemoryPool< PointPtr >();
		MemoryManager::m_PointPtrInternalsPool = new BitMapMemoryPool< PointPtrInternals >();
		
		MemoryManager::m_InnerPool = new BitMapMemoryPool< Inner >();
		MemoryManager::m_InnerPtrPool = new BitMapMemoryPool< InnerPtr >();
		MemoryManager::m_InnerPtrInternalsPool = new BitMapMemoryPool< InnerPtrInternals >();
		
		MemoryManager::m_LeafPool = new BitMapMemoryPool< Leaf >();
		MemoryManager::m_LeafPtrPool = new BitMapMemoryPool< LeafPtr >();
		MemoryManager::m_LeafPtrInternalsPool = new BitMapMemoryPool< LeafPtrInternals >();
		
		MemoryManager::m_MapInternalsPool = new BitMapMemoryPool< MapInternals >();
		
		MemoryManager::m_maxAllowedMem = maxAllowedMem;
	}
}

#endif