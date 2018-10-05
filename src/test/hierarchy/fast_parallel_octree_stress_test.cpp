#include <gtest/gtest.h>
#include "omicron/hierarchy/fast_parallel_octree.h"
#include "omicron/ui/gl_hidden_widget.h"
#include "hierarchy/fast_parallel_octree_test_param.h"

extern "C" FastParallelOctreeTestParam g_fastParallelStressParam;

namespace omicron::test::hierarchy
{
    TEST( FastParallelOctreeStressTest, DISABLED_Stress )
    {
        using Morton = MediumMortonCode;
        using Octree = FastParallelOctree< Morton >;
        using OctreeDim = Octree::Dim;
// 			using Sql = SQLiteManager< Point, Morton, Octree::Node >;
// 			using NodeArray = typename Sql::NodeArray;
        
        ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
        
        {
            ofstream m_log( "FastParallelOctreeStressTest.log", ios_base::app );
            
            FastParallelOctreeTestParam param = g_fastParallelStressParam;
            m_log << "Params: " << param << endl;
            auto start = Profiler::now( "Octree construction", m_log );
            
            GLHiddenWidget hiddenWidget;
            
            Octree octree( param.m_plyFilename, param.m_hierarchyLvl, RuntimeSetup( param.m_nThreads, param.m_workItemSize, param.m_memoryQuota ) );
            
            Profiler::elapsedTime( start, "Octree construction", m_log );
        }
        
        ASSERT_EQ( 0, AllocStatistics::totalAllocated() );
    }
}
