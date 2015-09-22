#include <gtest/gtest.h>
#include "BitMapMemoryManager.h"
#include "OctreeMapTypes.h"
#include <InnerNode.h>

namespace model
{
	namespace test
	{
        class BitMapMemoryManagerTest : public ::testing::Test
		{
		protected:
			void SetUp()
			{
				IMemoryManager& imanager = BitMapMemoryManager::instance();
				if( typeid( imanager ) == typeid( MemoryManager ) )
				{
					m_restoreManager = true;
					MemoryManager& manager = dynamic_cast< MemoryManager& >( imanager );
					
					m_shallowBlocks = manager.numBlocks( IMemoryManager::SHALLOW_MORTON );
					m_mediumBlocks = manager.numBlocks( IMemoryManager::MEDIUM_MORTON );
					m_pointBlocks = manager.numBlocks( IMemoryManager::POINT );
					m_extendedBlocks = manager.numBlocks( IMemoryManager::EXTENDED_POINT );
					m_nodeBlocks = manager.numBlocks( IMemoryManager::NODE );
				}
				else
				{
					m_restoreManager = false;
				}
			}
			
			void TearDown()
			{
				if( m_restoreManager )
				{
					MemoryManager::initInstance( m_shallowBlocks, m_mediumBlocks, m_pointBlocks, m_extendedBlocks,
												 m_nodeBlocks );
				}
			}
			
		private:
			bool m_restoreManager;
			size_t m_shallowBlocks;
			size_t m_mediumBlocks;
			size_t m_pointBlocks;
			size_t m_extendedBlocks;
			size_t m_nodeBlocks;
		};
		
		TEST_F( BitMapMemoryManagerTest, ManagedTypes0 )
		{
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t totalMortonsSize = nNodes * sizeof( ShallowMortonCode );
			size_t totalPointsSize = nPoints * sizeof( Point );
			size_t totalNodesSize = nNodes * sizeof( ShallowLeafNode< PointVector > );
			size_t maxMemToUse = totalMortonsSize + totalPointsSize + totalNodesSize;
			
			BitMapMemoryManager::initInstance( maxMemToUse );
			BitMapMemoryManager& manager = dynamic_cast< BitMapMemoryManager& >( BitMapMemoryManager::instance() );
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			ShallowOctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				ShallowMortonCodePtr mortonCode( new ShallowMortonCode() );
				mortonCode->build( i );
				
				ShallowLeafNodePtr< PointVector > node( new ShallowLeafNode< PointVector >() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				ShallowMortonCodePtr expected( new ShallowMortonCode() );
				expected->build( i );
				
				ASSERT_TRUE( map.find( expected ) != map.end() );
			}
			
			map.clear();
			
			ASSERT_EQ( 0, manager.usedMemory() );
		}
		
		TEST_F( BitMapMemoryManagerTest, ManagedTypes1 )
		{
			uint nNodes = 500000u;
			uint nPoints = 2u * nNodes;
			size_t totalMortonsSize = nNodes * sizeof( MediumMortonCode );
			size_t totalPointsSize = nPoints * sizeof( ExtendedPoint );
			size_t totalNodesSize = nNodes * sizeof( MediumInnerNode< ExtendedPointVector > );
			size_t maxMemToUse = totalMortonsSize + totalPointsSize + totalNodesSize;
			
			BitMapMemoryManager::initInstance( maxMemToUse );
			BitMapMemoryManager& manager = dynamic_cast< BitMapMemoryManager& >( BitMapMemoryManager::instance() );
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			MediumOctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MediumMortonCodePtr mortonCode( new MediumMortonCode() );
				mortonCode->build( i );
				
				MediumInnerNodePtr< ExtendedPointVector > node( new MediumInnerNode< ExtendedPointVector >() );
				ExtendedPointPtr p0( new ExtendedPoint() );
				ExtendedPointPtr p1( new ExtendedPoint() );
				node->setContents( ExtendedPointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MediumMortonCodePtr expected( new MediumMortonCode() );
				expected->build( i );
				
				ASSERT_TRUE( map.find( expected ) != map.end() );
			}
			
			map.clear();
			
			ASSERT_EQ( 0, manager.usedMemory() );
		}
		
		TEST_F( BitMapMemoryManagerTest, ManagedTypes2 )
		{
			uint nNodes = 500000u;
			uint nPoints = nNodes;
			size_t totalMortonsSize = nNodes * sizeof( MediumMortonCode );
			size_t totalPointsSize = nPoints * sizeof( ExtendedPoint );
			size_t totalNodesSize = nNodes * sizeof( MediumInnerNode< ExtendedPointPtr > );
			size_t maxMemToUse = totalMortonsSize + totalPointsSize + totalNodesSize;
			
			cout << "Size of ExtendedPointPtr: " << sizeof( ExtendedPointPtr )
				 << " Size of PointVector: " << sizeof( PointVector ) << endl
				 << "Size of ShallowLeafNode< PointVector >" << sizeof( ShallowLeafNode< PointVector > )
				 << " Size of MediumInnerNode< ExtendedPointPtr >" << sizeof( MediumInnerNode< ExtendedPointPtr > )  << endl << endl;
			
			BitMapMemoryManager::initInstance( maxMemToUse );
			BitMapMemoryManager& manager = dynamic_cast< BitMapMemoryManager& >( BitMapMemoryManager::instance() );
			
			ASSERT_EQ( 0, manager.usedMemory() );
			
			MediumOctreeMap map;
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MediumMortonCodePtr mortonCode( new MediumMortonCode() );
				mortonCode->build( i );
				
				MediumInnerNodePtr< ExtendedPointPtr > node( new MediumInnerNode< ExtendedPointPtr >() );
				ExtendedPointPtr p( new ExtendedPoint() );
				node->setContents( p );
				
				map[ mortonCode ] = node;
			}
			
			ASSERT_EQ( maxMemToUse, manager.usedMemory() );
			
			for( unsigned int i = 0u; i < nNodes; ++i )
			{
				MediumMortonCodePtr expected( new MediumMortonCode() );
				expected->build( i );
				
				ASSERT_TRUE( map.find( expected ) != map.end() );
			}
			
			map.clear();
			
			ASSERT_EQ( 0, manager.usedMemory() );
		}
	}
}