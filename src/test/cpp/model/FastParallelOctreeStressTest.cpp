#include <gtest/gtest.h>
#include "FastParallelOctree.h"
#include "FastParallelOctreeStressParam.h"

extern "C" FastParallelOctreeStressParam g_fastParallelStressParam;

namespace model
{
	namespace test
	{
		TEST( FastParallelOctreeStressTest, Stress )
		{
			using Morton = MediumMortonCode;
			using Octree = FastParallelOctree< Morton, Point >;
			using OctreeDim = Octree::Dim;
			using Sql = SQLiteManager< Point, Morton, Octree::Node >;
			using NodeArray = typename Sql::NodeArray;
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
			
			{
				ofstream m_log( "FastParallelOctreeStressTest.log", ios_base::app );
				
				FastParallelOctreeStressParam param = g_fastParallelStressParam;
				m_log << "Starting building octree. Params:" << param << endl;
				auto start = Profiler::now( m_log );
				
				Octree octree;
				octree.buildFromFile( param.m_plyFilename, param.m_hierarchyLvl, param.m_workItemSize,
									  param.m_memoryQuota, param.m_nThreads );
				
				m_log << "Time to build octree (ms): " << Profiler::elapsedTime( start, m_log ) << endl << endl;
				
				#ifdef HIERARCHY_STATS
					m_log << "Processed nodes: " <<octree.m_processedNodes << endl << endl;
				#endif
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}