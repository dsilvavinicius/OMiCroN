#ifndef OCTREE_STATS_H
#define OCTREE_STATS_H

#include <ostream>

using namespace std;

namespace model
{
	class OctreeStats
	{
	public:
		OctreeStats( const float traversalTime, const float renderingTime, const unsigned int numRenderedPoints )
		: m_traversalTime( traversalTime ),
		m_renderingTime( renderingTime ),
		m_numRenderedPoints( numRenderedPoints ) {}
		
		OctreeStats( OctreeStats&& other )
		: m_traversalTime( other.m_traversalTime ),
		m_renderingTime( other.m_renderingTime ),
		m_numRenderedPoints( other.m_numRenderedPoints ) {}
		
		friend ostream& operator<<( ostream& out, const OctreeStats& octreeStats )
		{
			out << "Traversal time: " << octreeStats.m_traversalTime << endl << endl
				<< "Rendering time: " << octreeStats.m_renderingTime << endl << endl
				<< "Rendered points: " << octreeStats.m_numRenderedPoints << endl << endl;
			return out;
		}
		
	public:
		const float m_traversalTime;
		const float m_renderingTime;
		const unsigned int m_numRenderedPoints;
	};
	
	class FrontOctreeStats
	: OctreeStats
	{
	public:
		FrontOctreeStats( const float traversalTime, const float renderingTime, const unsigned int numRenderedPoints,
						  const unsigned int numFrontNodes )
		: OctreeStats::OctreeStats( traversalTime, renderingTime, numRenderedPoints ),
		m_numFrontNodes( numFrontNodes ) {}
		
		FrontOctreeStats( OctreeStats& stats, unsigned int numFrontNodes )
		: FrontOctreeStats::FrontOctreeStats( stats.m_traversalTime, stats.m_renderingTime, stats.m_numRenderedPoints,
											  numFrontNodes ) {}
		
		FrontOctreeStats( FrontOctreeStats&& other )
		: OctreeStats( std::move( other ) ),
		m_numFrontNodes( other.m_numFrontNodes ) {}
		
		friend ostream& operator<<( ostream& out, const FrontOctreeStats& frontOctreeStats )
		{
			
			out << ( OctreeStats& ) frontOctreeStats;
			out << "Front total nodes: " << frontOctreeStats.m_numFrontNodes << endl << endl;
			return out;
		}
		
	public:
		const unsigned int m_numFrontNodes;
	};
}

#endif