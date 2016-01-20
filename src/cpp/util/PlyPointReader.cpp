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
	
	template<>
	void PlyPointReader< Point >::copyProperty( p_ply_property property, p_ply out )
	{
		const char *property_name;
		e_ply_type type, length_type, value_type;
		ply_get_property_info( property, &property_name, &type, &length_type, &value_type );
		
		if( !strcmp( property_name, "x" ) || !strcmp( property_name, "y" ) || !strcmp( property_name, "z" ) ||
			!strcmp( property_name, "nx" ) || !strcmp( property_name, "ny" ) || !strcmp( property_name, "nz" ) )
		{
			/* add this property to output file */
			if( !ply_add_property( out, property_name, type, length_type, value_type ) )
			{
				throw runtime_error( "Cannot copy property to .ply header." );
			}
		}
	}
	
	template<>
	void PlyPointReader< ExtendedPoint >::copyProperty( p_ply_property property, p_ply out )
	{
		const char *property_name;
		e_ply_type type, length_type, value_type;
		ply_get_property_info( property, &property_name, &type, &length_type, &value_type );
		
		if( !strcmp( property_name, "x" ) || !strcmp( property_name, "y" ) || !strcmp( property_name, "z" ) ||
			!strcmp( property_name, "nx" ) || !strcmp( property_name, "ny" ) || !strcmp( property_name, "nz" ) ||
			!strcmp( property_name, "red" ) || !strcmp( property_name, "green" ) || !strcmp( property_name, "blue" ) )
		{
			// Debug
			{
				cout << "Copy prop: " << property_name << endl << endl;
			}
			
			/* add this property to output file */
			if( !ply_add_property( out, property_name, type, length_type, value_type ) )
			{
				throw runtime_error( "Cannot copy property to .ply header." );
			}
		}
	}
}