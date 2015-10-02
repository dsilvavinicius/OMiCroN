#ifndef BITMAP_MEMORY_MANAGER_H
#define BITMAP_MEMORY_MANAGER_H

#include <map>
#include <mutex>
#include "IMemoryPool.h"
#include "MemoryManager.h"

namespace model
{
	const int BIT_MAP_SIZE = 1048576; // 1024 ^ 2
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
	
		int Index;
		int BlocksAvailable;
		int BitMap[ BIT_MAP_ELEMENTS ];
	};

	/** Contains info of array allocation requests. */
	typedef struct ArrayInfo
	{
		int   MemPoolListIndex;
		int   StartPosition;
		int   Size;
	}
	ArrayMemoryInfo;
	
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
		
	public: 
		BitMapMemoryPool() {}
		
		~BitMapMemoryPool();
		
		T* allocate() override;
		
		T* allocateArray( const size_t& size ) override;
		
		void deallocate( T* p ) override;
		
		void deallocateArray( T* p) override;
		
		size_t usedBlocks() const override;
		
		size_t memoryUsage() const override;
		
	private:
		T* AllocateArrayMemory( size_t size );
		T* AllocateChunkAndInitBitMap();
		
		/** Creates an ArrayMemoryInfo, setting all necessary bits in the associated bitmap entry, inserting it into the
		 * array info map and returning the pointer to the allocated array. */
		T* createArrayInfo( int index, int start, int size );
		
		/** Sets the bit related with the given pointer. */
		void SetBlockBit( T* object, bool flag );
		
		/** Sets all bits associated with a given array described by the given ArrayMemoryInfo. */
		void SetMultipleBlockBits( ArrayMemoryInfo* info, bool flag );
		
		/** @returns the first free block of the given BitMapEntry. */
		T* firstFreeBlock( BitMapEntry* bitmap );
		
		/** @returns the object address of the given position in the memory chunk associated with the given BitMapEntry. */
		T* objectAddress( BitMapEntry* bitmap, int pos );
		
		vector< T* >& GetMemoryPoolList();
		
		vector< T* > MemoryPoolList;
		vector< BitMapEntry > BitMapEntryList;
		//the above two lists will maintain one-to-one correpondence and hence 
		//should be of same  size.
		set< BitMapEntry* > FreeMapEntries; // Bitmaps that currently have free entries.
		
		map< T*, ArrayMemoryInfo > ArrayMemoryList; // List of currently allocated arrays.
		set< int > freeArrayPoolIndices; // Indices for array memory pools that currently have no arrays allocated.
		mutex poolLock; // Lock to the pool
	};
	
	template< typename Morton, typename Point, typename Inner, typename Leaf >
	class BitMapMemoryManager
	: public MemoryManager< Morton, Point, Inner, Leaf >
	{
		using MemoryManager = model::MemoryManager< Morton, Point, Inner, Leaf >;
		using PointPtr = shared_ptr< Point >;
		using PtrInternals = std::_Sp_counted_ptr_inplace< Point, BitMapAllocator< Point >, (__gnu_cxx::_Lock_policy)2 >;
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
	void BitMapEntry< T >::SetMultipleBits( int position, bool flag, int count )
	{
		//cout << "SetMultipleBits " << flag << endl << endl;
		
		BlocksAvailable += flag ? count : -count;
		int elementNo = position / INT_SIZE;
		int bitNo = position % INT_SIZE;

		int bitSize = ( count <= INT_SIZE - bitNo ) ? count : INT_SIZE - bitNo;  
		SetRangeOfInt( &BitMap[ elementNo ], bitNo + bitSize - 1, bitNo, flag );
		count -= bitSize;
		if( !count ) return;
		
		int i = ++elementNo;
		while( count >= 0 )
		{
			if( count <= INT_SIZE )
			{
				SetRangeOfInt( &BitMap[ i ], count - 1, 0, flag );
				return;
			}
			else 
				BitMap[ i ] = flag ? unsigned ( -1 ) : 0;
			count -= 32; 
			i++;
		}
	}

	template< typename T >
	void BitMapEntry< T >::SetRangeOfInt( int* element, int msb, int lsb, bool flag )
	{
		if( flag )
		{
			int mask = ( unsigned( -1 ) << lsb ) & ( unsigned( -1 ) >> INT_SIZE - msb - 1 );
			*element |= mask;
		}
		else 
		{
			int mask = ( unsigned( -1 ) << lsb ) & ( unsigned( -1 ) >> INT_SIZE - msb - 1 );
			*element &= ~mask;
		}
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
		lock_guard< mutex > guard( poolLock );
		
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
		lock_guard< mutex > guard( poolLock );
		
		if( size > BIT_MAP_SIZE * sizeof( T ) )
		{
			throw logic_error( "Array allocation: size greater than maximum allowed." );
		}
		
		size_t neededBlocks = size / sizeof( T );
		
		// Checks if there is free array pools.
		if( freeArrayPoolIndices.size() != 0 )
		{
			auto freeArrayPoolIdxIt = freeArrayPoolIndices.begin();
			int index = *freeArrayPoolIdxIt;
			freeArrayPoolIndices.erase( freeArrayPoolIdxIt );
			return createArrayInfo( index, 0, neededBlocks );
		}
		
		typename std::map< T*, ArrayMemoryInfo >::iterator infoI = ArrayMemoryList.begin();
		typename std::map< T*, ArrayMemoryInfo >::iterator infoEndI = ArrayMemoryList.end();
		
		while( infoI != infoEndI )
		{
			ArrayMemoryInfo info = ( *infoI ).second;
			BitMapEntry* entry = &BitMapEntryList[ info.MemPoolListIndex ];
			
			if( entry->BlocksAvailable < neededBlocks ) 
				continue;
			else 
			{
				if( info.StartPosition != 0 )
				{
					// Check if have space behind current array.
					if( info.StartPosition >= neededBlocks )
					{
						return createArrayInfo( info.MemPoolListIndex, 0, neededBlocks );
					}
				}
				else
				{
					while( infoI != infoEndI )
					{
						info = infoI->second;
						auto nextI = std::next( infoI, 1 );
						if( nextI == infoEndI || nextI->second.MemPoolListIndex != info.MemPoolListIndex )
						{
							return createArrayInfo( info.MemPoolListIndex, info.StartPosition + info.Size, neededBlocks );
						}
						else
						{
							ArrayMemoryInfo nextInfo = nextI->second;
							if( info.StartPosition + info.Size - nextInfo.StartPosition >= neededBlocks )
							{
								return createArrayInfo( info.MemPoolListIndex, info.StartPosition + info.Size, neededBlocks );
							}
						}
						
						infoI++;
					}
				}
			}
		}
		
		return AllocateArrayMemory( size );
	}
	
	template< typename T >
	T* BitMapMemoryPool< T >::createArrayInfo( int index, int start, int size )
	{
		ArrayInfo info;
		info.MemPoolListIndex = index;
		info.StartPosition = start;
		info.Size = size;
		
		T* baseAddress = MemoryPoolList[ info.MemPoolListIndex ] + start;
		ArrayMemoryList[ baseAddress ] = info;
		SetMultipleBlockBits( &info, false );
		
		return baseAddress;
	}
	
	template< typename T >
	T* BitMapMemoryPool< T >::AllocateArrayMemory( size_t size )
	{
		T* chunkAddress = AllocateChunkAndInitBitMap();
		ArrayMemoryInfo info;
		info.MemPoolListIndex = MemoryPoolList.size() - 1;
		info.StartPosition = 0;
		info.Size = size / sizeof( T );
		ArrayMemoryList[ chunkAddress ] = info;
		SetMultipleBlockBits( &info, false );
		return chunkAddress;
	}

	template< typename T >
	void BitMapMemoryPool< T >::deallocate( T* object )
	{
		lock_guard< mutex > guard( poolLock );
		
		SetBlockBit( object, true );
	}
	
	template< typename T >
	void BitMapMemoryPool< T >::deallocateArray( T* object )
	{
		lock_guard< mutex > guard( poolLock );
		
		auto it = ArrayMemoryList.find( object );
		ArrayMemoryInfo *info = &it->second;
		SetMultipleBlockBits( info, true );
		
		// In case of all ArrayMemoryInfo of the pool being dealloc, the pool index is inserted into freeArrayPoolIndices
		// for fast alloc later on.
		if( BitMapEntryList[ info->MemPoolListIndex ].BlocksAvailable == BIT_MAP_SIZE )
		{
			freeArrayPoolIndices.insert( info->MemPoolListIndex );
		}
		
		ArrayMemoryList.erase( it );
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

	template< typename T >
	void BitMapMemoryPool< T >::SetMultipleBlockBits( ArrayMemoryInfo* info, bool flag )
	{
		BitMapEntry* mapEntry = &BitMapEntryList[ info->MemPoolListIndex ];
		mapEntry->SetMultipleBits( info->StartPosition, flag, info->Size );
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
		MemoryManager::m_mortonPool = new BitMapMemoryPool< Morton >();
		MemoryManager::m_pointPool = new BitMapMemoryPool< Point >();
		MemoryManager::m_pointPtrPool = new BitMapMemoryPool< PointPtr >();
		MemoryManager::m_ptrInternalsPool = new BitMapMemoryPool< PtrInternals >();
		MemoryManager::m_innerPool = new BitMapMemoryPool< Inner >();
		MemoryManager::m_leafPool = new BitMapMemoryPool< Leaf >();
		MemoryManager::m_maxAllowedMem = maxAllowedMem;
	}
}

#endif