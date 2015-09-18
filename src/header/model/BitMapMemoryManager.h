#ifndef BITMAP_MEMORY_MANAGER_H
#define BITMAP_MEMORY_MANAGER_H

#include <map>
#include "IMemoryManager.h"
#include "Point.h"
#include "MortonCode.h"
#include "OctreeNode.h"
#include "IMemoryPool.h"

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
		void SetRangeOfInt( int* element, int msb, int lsb, bool flag );
		T* FirstFreeBlock();
		T* objectAddress( int pos );
		T* Head();
	
		int Index;
		int BlocksAvailable;
		int BitMap[ BIT_MAP_ELEMENTS ];
	};

	typedef struct ArrayInfo
	{
		int   MemPoolListIndex;
		int   StartPosition;
		int   Size;
	}
	ArrayMemoryInfo;
	
	template< typename T >
	class BitMapMemoryPool
	: IMemoryPool< T >
	{
		using BitMapEntry = model::BitMapEntry< T >;
		
	public: 
		BitMapMemoryPool() {}
		~BitMapMemoryPool();
		T* allocate() override;
		T* allocateArray( const size_t& size ) override;
		void deallocate( T* p ) override;
		void deallocateArray( T* p) override;
		vector< T* >& GetMemoryPoolList();
		
		/** Calculates how much memory blocks are currently used. */
		size_t usedBlocks() const override;
		
		/** Calculates how much memory is currently used in this pool in bytes. */
		size_t memoryUsage() const override;
		
	private:
		T* AllocateArrayMemory( size_t size );
		T* AllocateChunkAndInitBitMap();
		/** Sets the bit related with the given pointer. */
		void SetBlockBit( T* object, bool flag );
		void SetMultipleBlockBits( ArrayMemoryInfo* info, bool flag );
		
		vector< T* > MemoryPoolList;
		vector< BitMapEntry > BitMapEntryList;
		//the above two lists will maintain one-to-one correpondence and hence 
		//should be of same  size.
		set< BitMapEntry* > FreeMapEntries;
		map< T*, ArrayMemoryInfo > ArrayMemoryList;
	};
	
	class BitMapMemoryManager
	: public SingletonMemoryManager 
	{
	public:
		static void initInstance( const size_t& maxAllowedMem );
		
		~BitMapMemoryManager() {}
		
		void* allocate( const MANAGED_TYPE_FLAG& type ) override;
		
		void* allocateArray( const size_t& size, const MANAGED_TYPE_FLAG& type ) override;
		
		void deallocate( void* p, const MANAGED_TYPE_FLAG& type ) override;
		
		void deallocateArray( void* p, const MANAGED_TYPE_FLAG& type ) override;
		
		bool hasEnoughMemory( const float& percentageThreshold ) const override;
		
		size_t usedMemory() const override;
		
		size_t maxAllowedMem() const override { return m_maxAllowedMem; }
		
		string toString() const override;
		
		template< typename T > BitMapMemoryPool< T >& getPool();
	
	private:
		BitMapMemoryManager( const size_t& maxAllowedMem )
		: m_maxAllowedMem( maxAllowedMem ){}
		
		size_t m_maxAllowedMem;
		BitMapMemoryPool< ShallowMortonCode > m_shallowMortonPool;
		BitMapMemoryPool< MediumMortonCode > m_mediumMortonPool;
		BitMapMemoryPool< Point > m_pointPool;
		BitMapMemoryPool< ExtendedPoint > m_extendedPointPool;
		BitMapMemoryPool< ShallowLeafNode< PointVector > > m_nodePool; 	// All nodes require the same memory amount, since
																		// the contents are a vector of smart pointers.
	};
	
	template< typename T >
	void BitMapEntry< T >::SetBit( int position, bool flag )
	{
		//cout << "SetBit " << flag << endl << endl;
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
	T* BitMapEntry< T >::FirstFreeBlock()
	{
		for( int i = 0 ; i < BIT_MAP_ELEMENTS; ++i )
		{
			//cout << "Bitmap " << i << ": " << BitMap[ i ] << endl << endl;
			
			if( BitMap[ i ] == 0 )
				continue;            // no free bit was found 
			
			int result = BitMap[ i ] & -( BitMap[ i ] ); // this expression yields the first 
														// bit position which is 1 in an int from right.
			int basePos = ( INT_SIZE * i );
			switch( result )
			{
				//make the corresponding bit 0 meaning block is no longer free
				case 0x00000001: return objectAddress( basePos + 0 );
				case 0x00000002: return objectAddress( basePos + 1 );
				case 0x00000004: return objectAddress( basePos + 2 );
				case 0x00000008: return objectAddress( basePos + 3 );
				case 0x00000010: return objectAddress( basePos + 4 );
				case 0x00000020: return objectAddress( basePos + 5 );
				case 0x00000040: return objectAddress( basePos + 6 );
				case 0x00000080: return objectAddress( basePos + 7 );
				case 0x00000100: return objectAddress( basePos + 8 );
				case 0x00000200: return objectAddress( basePos + 9 );
				case 0x00000400: return objectAddress( basePos + 10 );
				case 0x00000800: return objectAddress( basePos + 11 );
				case 0x00001000: return objectAddress( basePos + 12 );
				case 0x00002000: return objectAddress( basePos + 13 );
				case 0x00004000: return objectAddress( basePos + 14 );
				case 0x00008000: return objectAddress( basePos + 15 );
				case 0x00010000: return objectAddress( basePos + 16 );
				case 0x00020000: return objectAddress( basePos + 17 );
				case 0x00040000: return objectAddress( basePos + 18 );
				case 0x00080000: return objectAddress( basePos + 19 );
				case 0x00100000: return objectAddress( basePos + 20 );
				case 0x00200000: return objectAddress( basePos + 21 );
				case 0x00400000: return objectAddress( basePos + 22 );
				case 0x00800000: return objectAddress( basePos + 23 );
				case 0x01000000: return objectAddress( basePos + 24 );
				case 0x02000000: return objectAddress( basePos + 25 );
				case 0x04000000: return objectAddress( basePos + 26 );
				case 0x08000000: return objectAddress( basePos + 27 );
				case 0x10000000: return objectAddress( basePos + 28 );
				case 0x20000000: return objectAddress( basePos + 29 );
				case 0x40000000: return objectAddress( basePos + 30 );
				case 0x80000000: return objectAddress( basePos + 31 );
				default : throw logic_error( "Unexpected bit position for allocation." );
			}
		}
		
		throw logic_error( "Cannot allocate memory." );
		return 0;
	}

	template< typename T >
	T* BitMapEntry< T >::objectAddress( int pos )
	{
		SetBit( pos, false ); 
		int elementIdx = pos / INT_SIZE;
		int bitIdx = INT_SIZE - ( ( pos % INT_SIZE ) + 1 );
		
		//cout << "Pos: "<< pos << ", elemIdx: " << elementIdx << ", bitIdx: " << bitIdx << endl << endl;
		
		return &( ( Head() + elementIdx * INT_SIZE )[ bitIdx ] );
	} 

	template< typename T >
	T* BitMapEntry< T >::Head()
	{
		return static_cast< BitMapMemoryManager& >( BitMapMemoryManager::instance() ).getPool< T >()
				.GetMemoryPoolList()[ Index ];
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
		typename std::set< BitMapEntry* >::iterator freeMapI = FreeMapEntries.begin();
		if( freeMapI != FreeMapEntries.end() )
		{
			//cout << "Free entry found." << endl << endl;
			BitMapEntry* mapEntry = *freeMapI;
			T* block = mapEntry->FirstFreeBlock();
			
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
			FreeMapEntries.insert( &( BitMapEntryList[ BitMapEntryList.size() - 1 ] ) );
			return BitMapEntryList[ BitMapEntryList.size() - 1 ].FirstFreeBlock();
		}
	}

	template< typename T >
	T* BitMapMemoryPool< T >::allocateArray( const size_t& size )
	{
		if( ArrayMemoryList.empty() )
		{
			return AllocateArrayMemory( size );
		}
		else 
		{
			typename std::map< T*, ArrayMemoryInfo >::iterator infoI = ArrayMemoryList.begin();
			typename std::map< T*, ArrayMemoryInfo >::iterator infoEndI = ArrayMemoryList.end();
			while( infoI != infoEndI )
			{
				ArrayMemoryInfo info = ( *infoI ).second;
				if( info.StartPosition != 0 )	// search only in those mem blocks  
					continue;             		// where allocation is done from first byte
				else 
				{
					BitMapEntry* entry = &BitMapEntryList[ info.MemPoolListIndex ];
					if( entry->BlocksAvailable < ( size / sizeof( T ) ) ) 
						return AllocateArrayMemory( size );
					else 
					{
						info.StartPosition = BIT_MAP_SIZE - entry->BlocksAvailable;
						info.Size = size / sizeof( T );
						T* baseAddress = MemoryPoolList[ info.MemPoolListIndex ] + info.StartPosition;

						ArrayMemoryList[ baseAddress ] = info;
						SetMultipleBlockBits( &info, false );
			
						return baseAddress;
					} 
				}
			}
		}
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
		//cout << "Deallocate" << endl << endl;
		SetBlockBit( object, true );
	}
	
	template< typename T >
	void BitMapMemoryPool< T >::deallocateArray( T* object )
	{
		ArrayMemoryInfo *info = &ArrayMemoryList[ object ];
		SetMultipleBlockBits( info, true );
	}
	
	template< typename T >
	std::vector< T* >& BitMapMemoryPool< T >::GetMemoryPoolList()
	{ 
		return MemoryPoolList;
	}
	
	template< typename T >
	size_t BitMapMemoryPool< T >::usedBlocks() const
	{
		size_t nUsedBlocks = ( BitMapEntryList.size() - FreeMapEntries.size() ) * BIT_MAP_SIZE;
		for( BitMapEntry* entry : FreeMapEntries )
		{
			nUsedBlocks += BIT_MAP_SIZE - entry->BlocksAvailable;
		}
		
		return nUsedBlocks;
	}
	
	template< typename T >
	size_t BitMapMemoryPool< T >::memoryUsage() const
	{
		return usedBlocks() * sizeof( T );
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
		//cout << "SetBlockBit " << flag << endl << endl;
		int i = BitMapEntryList.size() - 1;
		for( ; i >= 0 ; i-- )
		{
			BitMapEntry* bitMap = &BitMapEntryList[ i ];
			if( ( bitMap->Head() <= object ) && ( bitMap->Head() + BIT_MAP_SIZE - 1 >= object ) )
			{
				int position = object - bitMap->Head();
				bitMap->SetBit( position, flag );
				
				//cout << "New available: " << bitMap->BlocksAvailable << endl << endl;
				
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
		
		if( mapEntry->BlocksAvailable == info->Size && flag )
		{
			FreeMapEntries.insert( mapEntry );
		}
	}
	
	inline size_t BitMapMemoryManager::usedMemory() const
	{
		return 	m_shallowMortonPool.memoryUsage() + m_mediumMortonPool.memoryUsage() + m_pointPool.memoryUsage()
				+ m_extendedPointPool.memoryUsage() + m_nodePool.memoryUsage();
	}
}

#endif