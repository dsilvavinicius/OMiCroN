#include <gtest/gtest.h>
#include "MemoryManager.h"
#include "OctreeMapTypes.h"
#include "MortonCode.h"
#include "LeafNode.h"
#include <BitMapMemoryManager.h>

using namespace std;

namespace model
{
	namespace test
	{
        class MemoryManagerTest : public ::testing::Test
		{
		protected:
			void SetUp()
			{
				m_nNodes = 500000u;
				m_nPoints = 2u * m_nNodes;
				
				IMemoryManager& imanager = MemoryManager::instance();
				if( typeid( imanager ) == typeid( BitMapMemoryManager ) )
				{
					m_restoreManager = true;
					BitMapMemoryManager& manager = dynamic_cast< BitMapMemoryManager& >( imanager );
					m_maxAllowedMemory = manager.maxAllowedMem();
					MemoryManager::initInstance( m_nNodes, m_nNodes, m_nPoints, m_nPoints, m_nNodes );
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
					BitMapMemoryManager::initInstance( m_maxAllowedMemory );
				}
			}
		
		protected:
			size_t m_nNodes;
			size_t m_nPoints;
			
		private:
			bool m_restoreManager;
			size_t m_maxAllowedMemory;
		};
		
		TEST_F( MemoryManagerTest, ShallowPointVectorLeafNodes )
		{
			IMemoryManager& manager = MemoryManager::instance();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
			
			ShallowOctreeMap map;
			
			for( unsigned int i = 0u; i < m_nNodes; ++i )
			{
				if( i == 0.5 * m_nNodes )
				{
					size_t expectedHalfSize = 	0.5 * m_nNodes * sizeof( ShallowMortonCode )
												+ 0.5 * m_nPoints * sizeof( Point )
												+ 0.5 * m_nNodes * sizeof( ShallowLeafNode< PointVector > );
					ASSERT_EQ( manager.usedMemory(), expectedHalfSize );
				}
				
				ShallowMortonCodePtr mortonCode( new ShallowMortonCode() );
				mortonCode->build( i );
				
				ShallowLeafNodePtr< PointVector > node( new ShallowLeafNode< PointVector >() );
				PointPtr p0( new Point() );
				PointPtr p1( new Point() );
				node->setContents( PointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			size_t expectedFullSize = 	m_nNodes * sizeof( ShallowMortonCode ) + m_nPoints * sizeof( Point )
										+ m_nNodes * sizeof( ShallowLeafNode< PointVector > );
			ASSERT_EQ( manager.usedMemory(), expectedFullSize );
			
			map.clear();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
		}
		
		TEST_F( MemoryManagerTest, MediumExtendedPointVectorInnerNodes )
		{
			IMemoryManager& manager = MemoryManager::instance();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
			
			MediumOctreeMap map;
			
			for( unsigned int i = 0u; i < m_nNodes; ++i )
			{
				if( i == 0.5 * m_nNodes )
				{
					size_t expectedHalfSize = 	0.5 * m_nNodes * sizeof( MediumMortonCode )
												+ 0.5 * m_nPoints * sizeof( ExtendedPoint )
												+ 0.5 * m_nNodes * sizeof( MediumLeafNode< ExtendedPointVector > );
					ASSERT_EQ( manager.usedMemory(), expectedHalfSize );
				}
				
				MediumMortonCodePtr mortonCode( new MediumMortonCode() );
				mortonCode->build( i );
				
				MediumLeafNodePtr< ExtendedPointVector > node( new MediumLeafNode< ExtendedPointVector >() );
				ExtendedPointPtr p0( new ExtendedPoint() );
				ExtendedPointPtr p1( new ExtendedPoint() );
				node->setContents( ExtendedPointVector( { p0, p1 } ) );
				
				map[ mortonCode ] = node;
			}
			
			size_t expectedFullSize = 	m_nNodes * sizeof( MediumMortonCode ) + m_nPoints * sizeof( ExtendedPoint )
										+ m_nNodes * sizeof( MediumLeafNode< ExtendedPointVector > );
			ASSERT_EQ( manager.usedMemory(), expectedFullSize );
			
			map.clear();
			
			ASSERT_EQ( manager.usedMemory(), ( size_t ) 0 );
		}
	}
}