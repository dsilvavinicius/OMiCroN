#ifndef EXTERNAL_SORT_READER_H
#define EXTERNAL_SORT_READER_H

#include <stxxl/stream>
#include "omicron/util/profiler.h"
#include "omicron/disk/ply_point_reader.h"
#include "omicron/hierarchy/octree_dim_calculator.h"

namespace omicron::disk
{
    using namespace std;
    using namespace basic;
    using namespace hierarchy;
    using namespace util;
    using namespace stxxl::stream;
    
    /** A point reader which sorts datasets bigger than memory in morton order. It performs a k-way merge algorithm using the STXXL library. The algorithm can be divided in two phases: input (the first run phase) and output (the merge phase). The input phase is the pre-processing and the output phase serves as a stream of sorted points. The output starts as early as the first merged point is found. */
    template< typename Morton >
    class ExternalSortReader
    : public PointReader
    {
    public:
        using OctreeDim = OctreeDimensions< Morton >;
        using OctreeDimCalc = OctreeDimCalculator< Morton >;
        using DimOriginScale = hierarchy::DimOriginScale< Morton >;
        
        /** Ctor. Initializes the sort and performs the input phase. */
        ExternalSortReader( const string& inputFilename, uint maxLevel );
        
        /** Performs the second phase, calling onPointSorted for each point merged. */
        void read( const function< void( const Point& ) >& onPointSorted ) override;
    
        const OctreeDim& dimensions() const { return *m_comp; }
    
    private:
        static constexpr ulong RUNS_CREATOR_MEMORY = 10ul * 1024ul * 1024ul * 1024ul;
        static constexpr ulong RUNS_MERGER_MEMORY = 1ul * 1024ul * 1024ul * 1024ul;

        using RunsCreator = runs_creator< use_push< Point >, OctreeDim >;
        
        unique_ptr< RunsCreator > m_runsCreator;
        unique_ptr< OctreeDim > m_comp;
    };
    
    template< typename Morton >
    ExternalSortReader< Morton >::ExternalSortReader( const string& inputFilename, uint maxLevel )
    {
        // Calculates the octree dimensions.
        OctreeDimCalc dimCalc( []( const Point& p ){} );
        
        {
            PlyPointReader dimCalcReader( inputFilename );
            dimCalcReader.read(
                [ & ]( const Point& p )
                {
                    dimCalc.insertPoint( p );
                }
            );
        }
        
        DimOriginScale dimOriginScale = dimCalc.dimensions( maxLevel );
        
        m_comp = unique_ptr< OctreeDim >( new OctreeDim( dimOriginScale.dimensions() ) );
        
        auto start = Profiler::now( "STXXL::runs_creator." );
        
        m_runsCreator = unique_ptr< RunsCreator >( new RunsCreator( *m_comp, RUNS_CREATOR_MEMORY ) );
        
        PlyPointReader reader( inputFilename );
        reader.read(
            [ & ]( const Point& p )
            {
                Point copy( p );
                m_runsCreator->push( dimOriginScale.scale( copy ) );
            }
        );
        
        m_inputTime = Profiler::elapsedTime( start, "STXXL::runs_creator." );
    }
    
    template< typename Morton >
    void ExternalSortReader< Morton >::read( const function< void( const Point& ) >& onPointSorted )
    {
        using RunsMerger = runs_merger< typename RunsCreator::sorted_runs_type >;
        
        auto start = Profiler::now( "STXXL::runs_merger init." );
        
        RunsMerger runsMerger( m_runsCreator->result(), *m_comp, RUNS_MERGER_MEMORY );
        
        m_initTime = Profiler::elapsedTime( start, "STXXL::runs_merger init." );
        
        start = Profiler::now( "STXXL::runs_merger output." );
        
        while( !runsMerger.empty() )
        {
            onPointSorted( *runsMerger );
            ++runsMerger;
        }
    
        m_readTime = Profiler::elapsedTime( start, "STXXL::runs_merger output." );
    }
}

#endif
