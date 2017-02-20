#ifndef OCTREE_STATS_H
#define OCTREE_STATS_H

#include <ostream>

using namespace std;

namespace model
{
	/** Statistics of a frame. */
	class FrameStats
	{
	public:
		FrameStats( const float traversalTime = 0.f, const float renderQueueTime = 0.f, const float nRenderedPoints = 0.f,
					const float frontInsertionDelay = 0.f, const float frontSize = 0.f, const float frontSegmentSize = 0.f )
		: m_traversalTime( traversalTime ),
		m_renderQueueTime( renderQueueTime ),
		m_cpuOverhead( traversalTime + renderQueueTime ),
		m_nRenderedPoints( nRenderedPoints ),
		m_frontInsertionDelay( frontInsertionDelay ),
		m_frontSize( frontSize ),
		m_frontSegmentSize( frontSegmentSize )
		{}
		
		friend ostream& operator<<( ostream& out, const FrameStats& frame )
		{
			out << "Traversal time: " << frame.m_traversalTime << endl
				<< "Render queue time: " << frame.m_renderQueueTime << endl
				<< "CPU overhead: " << frame.m_cpuOverhead << endl
				<< "Rendered points: " << frame.m_nRenderedPoints << endl
				<< "Front insertion delay:" << frame.m_frontInsertionDelay << endl
				<< "Front size: " << frame.m_frontSize << endl
				<< "Front segment size: " << frame.m_frontSegmentSize;
			return out;
		}
		
		float m_traversalTime;
		float m_renderQueueTime;
		float m_cpuOverhead;
		float m_nRenderedPoints;
		float m_frontInsertionDelay;
		float m_frontSize;
		float m_frontSegmentSize;
	};
	
	/** Statistics of an Octree. */
	class OctreeStats
	{
	public:
		OctreeStats()
		: m_nFrames( 0.f ),
		m_nFrontInsertions( 0.f )
		{
		}
		
		void addFrame( const FrameStats& frame )
		{
			++m_nFrames;
			
			m_currentStats = frame;
			
			float avgTraversalTime = calcIncrementalAvg( m_currentStats.m_traversalTime, m_avgStats.m_traversalTime, m_nFrames );
			float avgRenderQueueTime =  calcIncrementalAvg( m_currentStats.m_renderQueueTime, m_avgStats.m_renderQueueTime, m_nFrames );
			float avgRenderedPoints = calcIncrementalAvg( m_currentStats.m_nRenderedPoints, m_avgStats.m_nRenderedPoints, m_nFrames );
			float avgFrontInsertionDelay;
			if( m_currentStats.m_frontInsertionDelay > 0.f )
			{
				++m_nFrontInsertions;
				avgFrontInsertionDelay = calcIncrementalAvg( m_currentStats.m_frontInsertionDelay,
															 m_avgStats.m_frontInsertionDelay, m_nFrontInsertions );
			}
			else
			{
				avgFrontInsertionDelay = m_avgStats.m_frontInsertionDelay;
			}
			float avgFrontSize = calcIncrementalAvg( m_currentStats.m_frontSize, m_avgStats.m_frontSize, m_nFrames );
			float avgFrontSegmentSize = calcIncrementalAvg( m_currentStats.m_frontSegmentSize, m_avgStats.m_frontSegmentSize, m_nFrames );
			
			m_avgStats = FrameStats( avgTraversalTime, avgRenderQueueTime, avgRenderedPoints, avgFrontInsertionDelay, avgFrontSize,
									 avgFrontSegmentSize );
		}
		
		float calcIncrementalAvg( const float newValue, const float currentAvg, const float nFrames ) const
		{
			return currentAvg + ( newValue - currentAvg ) / nFrames;
		}
		
		friend ostream& operator<<( ostream& out, const OctreeStats& octreeStats )
		{
			out << "=== CURRENT FRAME STATS ===" << endl << octreeStats.m_currentStats << endl << endl
				<< "=== AVERAGE STATS === " << endl << octreeStats.m_avgStats;
			return out;
		}
		
	public:
		FrameStats m_currentStats;
		FrameStats m_avgStats;
		
		float m_nFrames;
		float m_nFrontInsertions;
	};
	
	/** Cumulus' statistics. */
	class CumulusStats
	{
	public:
		CumulusStats()
		: m_gpuOverhead( 0.f ),
		m_avgGpuOverhead( 0.f )
		{}
		
		void addFrame( const OctreeStats& octreeStats, const float gpuOverhead )
		{
			m_octreeStats = octreeStats;
			m_gpuOverhead = gpuOverhead;
			m_avgGpuOverhead = octreeStats.calcIncrementalAvg( m_gpuOverhead, m_avgGpuOverhead, octreeStats.m_nFrames );
		}
		
		friend ostream& operator<<( ostream& out, const CumulusStats& cumulusStats )
		{
			out << cumulusStats.m_octreeStats << endl << endl
				<< "=== GPU STATS === " << endl << "GPU Overhead: " << cumulusStats.m_gpuOverhead << endl
				<< "Average GPU Overhead: " << cumulusStats.m_avgGpuOverhead;
			return out;
		}
		
		OctreeStats m_octreeStats;
		float m_gpuOverhead;
		float m_avgGpuOverhead;
	};
}

#endif