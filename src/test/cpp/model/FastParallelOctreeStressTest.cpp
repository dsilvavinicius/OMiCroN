#include <gtest/gtest.h>
#include "FastParallelOctree.h"

namespace model
{
	namespace test
	{
		typedef struct TestParam
		{
			TestParam( const string& plyFilename, const int nThreads, const int hierarchyLvl, const int workItemSize,
					   const ulong memoryQuota )
			: m_nThreads( nThreads ),
			m_plyFilename( plyFilename ),
			m_hierarchyLvl( hierarchyLvl ),
			m_workItemSize( workItemSize ),
			m_memoryQuota( memoryQuota )
			{};
			
			friend ostream& operator<<( ostream &out, const TestParam &param );
			
			int m_nThreads;
			string m_plyFilename;
			int m_hierarchyLvl;
			int m_workItemSize;
			ulong m_memoryQuota;
		} TestParam;
		
		ostream& operator<<( ostream &out, const TestParam &param )
		{
			out << param.m_plyFilename << endl << "Threads: " << param.m_nThreads << endl << "Max lvl: "
				<< param.m_hierarchyLvl << endl << "Workitem size:" << param.m_workItemSize << endl
				<< "Mem quota (in bytes):" << param.m_memoryQuota << endl;
			return out;
		}
		
		/** Creates the test parameters.
		 * @param startFrom is the test index from which the tests will start. This parameter provides a way to resume
		 * tests after a crash or something. */
		vector< TestParam > createTestParams( int startFromIdx = 0 )
		{
			vector< TestParam > param;
			int currentTestIdx = 0;
			
			for( int nThreads = 8; nThreads >= 1; --nThreads )
			{
				for( int maxLvl = 20; maxLvl >= 5; maxLvl -= 5 )
				{
					for( float worklistSize = 2048.f; worklistSize >= 24.f; worklistSize *= 0.5f )
					{
						for( ulong memQuota = 1024ul * 1024ul * 1024ul * 5ul; memQuota >= 1024ul * 1024ul * 1024ul * 2ul;
							memQuota -= 1024ul * 1024ul * 1024ul * 2ul )
						{
							if( currentTestIdx++ >= startFromIdx )
							{
								param.push_back( TestParam( "../data/example/staypuff.ply", nThreads, maxLvl, worklistSize, memQuota ) );
							}
							if( currentTestIdx++ >= startFromIdx )
							{
								param.push_back( TestParam( "../../../src/data/real/tempietto_all.ply", nThreads, maxLvl, worklistSize, memQuota ) );
							}
							if( currentTestIdx++ >= startFromIdx )
							{
								param.push_back( TestParam( "../../../src/data/real/tempietto_sub_tot.ply", nThreads, maxLvl, worklistSize, memQuota ) );
							}
						}
					}
				}
			}
			
			return param;
		}
		
		class FastParallelOctreeStressTest
		: public ::testing::TestWithParam< TestParam >
		{};
		
		INSTANTIATE_TEST_CASE_P( Stress, FastParallelOctreeStressTest,
                        ::testing::ValuesIn( createTestParams( 32 ) ) );
		
		TEST_P( FastParallelOctreeStressTest, Stress )
		{
			using Morton = MediumMortonCode;
			using Octree = FastParallelOctree< Morton, Point >;
			using OctreeDim = Octree::Dim;
			using Sql = SQLiteManager< Point, Morton, Octree::Node >;
			using NodeArray = typename Sql::NodeArray;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				ofstream m_log( "FastParallelOctreeStressTest.log", ios_base::app );
				
				TestParam param = GetParam();
				m_log << "Starting building octree. Params:" << param << endl;
				auto start = Profiler::now( m_log );
				
				Octree octree;
				octree.buildFromFile( param.m_plyFilename, param.m_hierarchyLvl, param.m_workItemSize,
									  param.m_memoryQuota, param.m_nThreads );
				
				m_log << "Time to build octree (ms): " << Profiler::elapsedTime( start, m_log ) << endl << endl;
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}