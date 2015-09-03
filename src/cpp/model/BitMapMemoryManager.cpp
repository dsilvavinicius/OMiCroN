#include <iostream>
#include <vector>
#include <string>
#include "BitMapMemoryManager.h"

using namespace std;

namespace model
{

	void BitMapEntry::SetBit( int position, bool flag )
	{
		BlocksAvailable += flag ? 1 : -1;
		int elementNo = position / INT_SIZE;
		int bitNo = position % INT_SIZE;
		if( flag )
		{
			BitMap[elementNo] = BitMap[elementNo] | ( 1 << bitNo );
		}
		else
		{
			BitMap[elementNo] = BitMap[elementNo] & ~( 1 << bitNo ); 
		}
	}

	void BitMapEntry::SetMultipleBits(int position, bool flag, int count)
	{
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
			if ( count <= INT_SIZE )
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
	
	void BitMapEntry::SetRangeOfInt( int* element, int msb, int lsb, bool flag )
	{
		if( flag )
		{
			int mask = ( unsigned( -1 ) << lsb ) & ( unsigned( -1 ) >> INT_SIZE-msb-1 );
			*element |= mask;
		}
		else 
		{
			int mask = ( unsigned( -1 ) << lsb ) & ( unsigned( -1 ) >> INT_SIZE-msb-1 );
			*element &= ~mask;
		}
	}

	void* BitMapEntry::FirstFreeBlock( size_t size )
	{
		for( int i = 0 ; i < BIT_MAP_ELEMENTS; ++i )
		{
			if( BitMap[ i ] == 0 )
				continue;            // no free bit was found 

			int result = BitMap[ i ] & -( BitMap[ i ] ); // this expression yields the first bit position which is 1 in an int
														//from right.
			void* address = 0;
			int basePos = ( INT_SIZE * i );
			switch( result ) 
			{
				//make the corresponding bit 0 meaning block is no longer free
				case 0x00000001: return objectAddress(basePos + 0);
				case 0x00000002: return objectAddress(basePos + 1);
				case 0x00000004: return objectAddress(basePos + 2);
				case 0x00000008: return objectAddress(basePos + 3);
				case 0x00000010: return objectAddress(basePos + 4);
				case 0x00000020: return objectAddress(basePos + 5);
				case 0x00000040: return objectAddress(basePos + 6);
				case 0x00000080: return objectAddress(basePos + 7);
				case 0x00000100: return objectAddress(basePos + 8);
				case 0x00000200: return objectAddress(basePos + 9);
				case 0x00000400: return objectAddress(basePos + 10);
				case 0x00000800: return objectAddress(basePos + 11);
				case 0x00001000: return objectAddress(basePos + 12);
				case 0x00002000: return objectAddress(basePos + 13);
				case 0x00004000: return objectAddress(basePos + 14);
				case 0x00008000: return objectAddress(basePos + 15);
				case 0x00010000: return objectAddress(basePos + 16);
				case 0x00020000: return objectAddress(basePos + 17);
				case 0x00040000: return objectAddress(basePos + 18);
				case 0x00080000: return objectAddress(basePos + 19);
				case 0x00100000: return objectAddress(basePos + 20);
				case 0x00200000: return objectAddress(basePos + 21);
				case 0x00400000: return objectAddress(basePos + 22);
				case 0x00800000: return objectAddress(basePos + 23);
				case 0x01000000: return objectAddress(basePos + 24);
				case 0x02000000: return objectAddress(basePos + 25);
				case 0x04000000: return objectAddress(basePos + 26);
				case 0x08000000: return objectAddress(basePos + 27);
				case 0x10000000: return objectAddress(basePos + 28);
				case 0x20000000: return objectAddress(basePos + 29);
				case 0x40000000: return objectAddress(basePos + 30);
				case 0x80000000: return objectAddress(basePos + 31);
				default : break;
			}
		}
		return 0;
	}

	void* BitMapEntry::objectAddress( int pos )
	{
		SetBit( pos, false );
		int objectSize = static_cast< BitMapMemoryManager& >( BitMapMemoryManager::instance() ).getObjectSize();
		return static_cast< char* >( Head() ) + objectSize * ( ( pos / INT_SIZE ) + ( INT_SIZE - ( ( pos % INT_SIZE ) + 1 ) ) );
	}

	void* BitMapEntry::Head()
	{
		return static_cast< BitMapMemoryManager& >( BitMapMemoryManager::instance() ).GetMemoryPoolList()[Index];
	}

	void* BitMapMemoryManager::allocate( const size_t& size )
	{
		if( size == objectSize ) // non-array version
		{
			std::set< BitMapEntry* >::iterator freeMapI = FreeMapEntries.begin();
			if( freeMapI != FreeMapEntries.end() )
			{
				BitMapEntry* mapEntry = *freeMapI;
				return mapEntry->FirstFreeBlock( size );
			}
			else
			{
				AllocateChunkAndInitBitMap();
				FreeMapEntries.insert( &( BitMapEntryList[ BitMapEntryList.size() - 1 ] ) );
				return BitMapEntryList[ BitMapEntryList.size() - 1 ].FirstFreeBlock( size );
			}
		}
		else  // array version
		{
			if( ArrayMemoryList.empty() )
			{
				return AllocateArrayMemory( size );
			}
			else 
			{
				std::map< void*, ArrayMemoryInfo >::iterator infoI = ArrayMemoryList.begin();
				std::map< void*, ArrayMemoryInfo >::iterator infoEndI = ArrayMemoryList.end();
				while( infoI != infoEndI )
				{
					ArrayMemoryInfo info = ( *infoI ).second;
					if( info.StartPosition != 0 ) // search only in those mem blocks where allocation is done from first byte
						continue;
					else
					{
						BitMapEntry* entry = &BitMapEntryList[ info.MemPoolListIndex ];
						if( entry->BlocksAvailable < ( size / objectSize ) ) 
							return AllocateArrayMemory( size );
						else 
						{
							info.StartPosition = BIT_MAP_SIZE - entry->BlocksAvailable;
							info.Size = size / objectSize;
							void* baseAddress = static_cast< char* >( MemoryPoolList[ info.MemPoolListIndex ] ) +
												info.StartPosition * objectSize;

							ArrayMemoryList[ baseAddress ] = info;
							SetMultipleBlockBits( &info, false );

							return baseAddress;
						} 
					}
				}
			}
		} 
		return 0;
	}

	void* BitMapMemoryManager::AllocateArrayMemory( size_t size )
	{
		void* chunkAddress = AllocateChunkAndInitBitMap();
		ArrayMemoryInfo info;
		info.MemPoolListIndex = MemoryPoolList.size() - 1;
		info.StartPosition = 0;
		info.Size = size / objectSize;
		ArrayMemoryList[ chunkAddress ] = info;
		SetMultipleBlockBits( &info, false );
		return chunkAddress;
	}

	void* BitMapMemoryManager::AllocateChunkAndInitBitMap()
	{
		BitMapEntry mapEntry;
		void* memoryBeginAddress = new char[ objectSize * BIT_MAP_SIZE ];
		MemoryPoolList.push_back( memoryBeginAddress );
		mapEntry.Index = MemoryPoolList.size() - 1;
		BitMapEntryList.push_back( mapEntry );
		return memoryBeginAddress;
	}

	void BitMapMemoryManager::deallocate( void* object )
	{
		if( ArrayMemoryList.find( object ) == ArrayMemoryList.end() )
			SetBlockBit( object, true ); // simple block deletion 
		else 
		{	//memory block deletion 
			ArrayMemoryInfo *info = &ArrayMemoryList[ object ];
			SetMultipleBlockBits( info, true );
		}
	}

	void BitMapMemoryManager::SetBlockBit( void* object, bool flag )
	{
		int i = BitMapEntryList.size() - 1;
		for ( ; i >= 0; i-- )
		{
			BitMapEntry* bitMap = &BitMapEntryList[ i ];
			if( ( bitMap->Head() <= object ) && ( static_cast< char* >( bitMap->Head() ) + objectSize * ( BIT_MAP_SIZE-1 ) >= object ) )
			{
				int position = ( static_cast< char* >( object ) - static_cast< char* >( bitMap->Head() ) ) / objectSize;
				bitMap->SetBit( position, flag );
				flag ? bitMap->BlocksAvailable++ : bitMap->BlocksAvailable--;
			}
		}
	}

	void BitMapMemoryManager::SetMultipleBlockBits( ArrayMemoryInfo* info, bool flag )
	{
		BitMapEntry* mapEntry = &BitMapEntryList[ info->MemPoolListIndex ];
		mapEntry->SetMultipleBits( info->StartPosition, flag, info->Size );
	}

	std::vector<void*>& BitMapMemoryManager::GetMemoryPoolList()
	{
		return MemoryPoolList;
	}
}