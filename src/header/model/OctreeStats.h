#ifndef OCTREE_STATS_H
#define OCTREE_STATS_H

#include <ostream>

using namespace std;

namespace model
{
	class OctreeStats
	{
	public:
		OctreeStats( const unsigned int numRenderedPoints )
		: m_numRenderedPoints( numRenderedPoints ) {}
		
		OctreeStats( OctreeStats&& other )
		: m_numRenderedPoints( other.m_numRenderedPoints ) {}
		
		friend ostream& operator<<( ostream& out, const OctreeStats& octreeStats )
		{
			out << "Rendered points: " << octreeStats.m_numRenderedPoints << endl;
			return out;
		}
		
	public:
		const unsigned int m_numRenderedPoints;
	};
	
	class FrontOctreeStats
	: OctreeStats
	{
	public:
		FrontOctreeStats( const unsigned int numRenderedPoints, const unsigned int numFrontNodes,
						  const unsigned int numInvalidNodes )
		: OctreeStats::OctreeStats( numRenderedPoints ),
		m_numFrontNodes( numFrontNodes ),
		m_numInvalidNodes( numInvalidNodes ) {}
		
		FrontOctreeStats( OctreeStats& stats, unsigned int numFrontNodes, unsigned int numInvalidNodes )
		: FrontOctreeStats::FrontOctreeStats( stats.m_numRenderedPoints, numFrontNodes, numInvalidNodes ) {}
		
		FrontOctreeStats( FrontOctreeStats&& other )
		: OctreeStats( std::move( other ) ),
		m_numFrontNodes( other.m_numFrontNodes ),
		m_numInvalidNodes( other.m_numInvalidNodes ) {}
		
		friend ostream& operator<<( ostream& out, const FrontOctreeStats& frontOctreeStats )
		{
			
			out << ( OctreeStats& ) frontOctreeStats;
			out << "Front total nodes: " << frontOctreeStats.m_numFrontNodes << endl
				<< "Front invalid nodes: " << frontOctreeStats.m_numInvalidNodes << endl;
			return out;
		}
		
	public:
		const unsigned int m_numFrontNodes;
		const unsigned int m_numInvalidNodes;
	};
}

#endif