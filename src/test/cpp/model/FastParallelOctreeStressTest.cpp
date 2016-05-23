#include <gtest/gtest.h>
#include "FastParallelOctree.h"
#include "FastParallelOctreeTestParam.h"

extern "C" FastParallelOctreeTestParam g_fastParallelStressParam;

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
				
				FastParallelOctreeTestParam param = g_fastParallelStressParam;
				m_log << "Params: " << param << endl;
				auto start = Profiler::now( "Octree construction", m_log );
				
				Octree octree( param.m_plyFilename, param.m_hierarchyLvl,
							   Octree::RuntimeSetup( param.m_nThreads, param.m_workItemSize, param.m_memoryQuota ) );
				
				Profiler::elapsedTime( start, "Octree construction", m_log );
				
				#ifdef HIERARCHY_STATS
					m_log << "Processed nodes: " << octree.m_processedNodes << endl << endl;
				#endif
			}
			
			ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
		}
	}
}