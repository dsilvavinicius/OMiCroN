#ifndef EXTERNAL_SORT_READER_H
#define EXTERNAL_SORT_READER_H

#include <stxxl/stream>
#include "omicron/util/profiler.h"
#include "omicron/disk/ply_point_reader.h"

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
        
        /** Ctor. Initializes the sort and performs the input phase. */
        ExternalSortReader( const string& inputFilename, const OctreeDim& comp );
        
        /** Performs the second phase, calling onPointSorted for each point merged. */
        void read( const function< void( const Point& ) >& onPointSorted ) override;
    
        const OctreeDim& dimensions() const { return m_comp; }
    private:
        using RunsCreator = runs_creator< use_push< Point >, OctreeDim >;
        
        RunsCreator m_runsCreator;
        OctreeDim m_comp;
    };
    
    template< typename Morton >
    ExternalSortReader< Morton >::ExternalSortReader( const string& inputFilename, const OctreeDim& comp )
    : m_comp( comp ),
    m_runsCreator( comp, 10ul * 1024ul * 1024ul * 1024ul )
    {
        auto start = Profiler::now( "STXXL::runs_creator." );
        
        PlyPointReader reader( inputFilename );
        reader.read(
            [ & ]( const Point& p )
            {
                m_runsCreator.push( p );
            }
        );
        
        m_inputTime = Profiler::elapsedTime( start, "STXXL::runs_creator." );
    }
    
    template< typename Morton >
    void ExternalSortReader< Morton >::read( const function< void( const Point& ) >& onPointSorted )
    {
        using RunsMerger = runs_merger< typename RunsCreator::sorted_runs_type >;
        
        auto start = Profiler::now( "STXXL::runs_merger init." );
        
        RunsMerger runsMerger( m_runsCreator.result(), m_comp, 10ul * 1024ul * 1024ul * 1024ul );
        
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
