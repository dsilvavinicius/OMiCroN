#include "PlyPointReader.h"

namespace util
{
	template<>
	void PlyPointReader< Point >
	::setupAdditionalCallbacks( p_ply ply, const Precision& precision,
								pair< Point*, function< void( const Point& ) >* >& cbNeededData ) {}
	
	template<>
	void PlyPointReader< ExtendedPoint >
	::setupAdditionalCallbacks( p_ply ply, const Precision& precision,
								pair< ExtendedPoint*, function< void( const ExtendedPoint& ) >* >& cbNeededData )
	{
		ply_set_read_cb( ply, "vertex", "red", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 6, precision ) );
		ply_set_read_cb( ply, "vertex", "green", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 7, precision ) );
		ply_set_read_cb( ply, "vertex", "blue", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 8, precision ) );
	}
}