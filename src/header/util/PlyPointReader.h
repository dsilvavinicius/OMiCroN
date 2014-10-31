#ifndef PLY_POINT_READER_H
#define PLY_POINT_READER_H

#include <locale.h>
#include <string>
#include "rply/rply.h"
#include "Point.h"

using namespace std;
using namespace model;

namespace util
{
	/** RPly vertex callback. */
	int vertexCB( p_ply_argument argument );
	
	/** Reader for a point .ply file. To access the read points, use getPoints(). To verify if the model vertices have normals
	 * or just float colors, use hasNormals(). */
	template< typename Float, typename Vec3 >
	class PlyPointReader 
	{
	public:
		enum Precision
		{
			DOUBLE = 0x8,
			SINGLE = 0x0
		};
		
		// TODO: Specialize this class so the precision is infered automatically by template parameters.
		/** Reads a .ply file. . 
		 * @param precision specifies the desired precision for read points. It must be agreed with the template
		 * parameters of this method. */
		PlyPointReader( const string& fileName, Precision precision );
		
		PointVector< Float, Vec3 > getPoints();
		bool hasNormals();
	private:
		/** Sets the RPly callback for vertex data, given a property index and a precision flag. */
		void setVertexCB( p_ply ply, string propName, const unsigned int propIndex, Precision precision );
		
		PointVector< Float, Vec3 > m_points;
		bool m_normalsFlag;
	};
	
	template< typename Float, typename Vec3 >
	void PlyPointReader< Float, Vec3 >::setVertexCB( p_ply ply, string propName, const unsigned int propIndex,
													 Precision precision )
	{
		unsigned int propFlag = propIndex | ( precision << 4 );
		ply_set_read_cb( ply, "vertex", propName.c_str(), vertexCB, &m_points, propFlag );
	}
	
	template< typename Float, typename Vec3 >
	PlyPointReader< Float, Vec3 >::PlyPointReader( const string& fileName, PlyPointReader::Precision precision )
	{
		/* Save application locale */
		const char *old_locale = setlocale( LC_NUMERIC, NULL );
		/* Change to PLY standard */
		setlocale( LC_NUMERIC, "C" );

		p_ply ply = ply_open( fileName.c_str(), NULL, 0, NULL );
		if( !ply )
		{
			setlocale( LC_NUMERIC, old_locale );
			throw runtime_error( "Cannot open .ply point file" );
		}
		if( !ply_read_header( ply ) )
		{
			ply_close( ply );
			setlocale( LC_NUMERIC, old_locale );
			throw runtime_error( "Cannot read point file header." );
		}
		
		setVertexCB( ply, "x", 0, precision );
		setVertexCB( ply, "y", 1, precision );
		setVertexCB( ply, "z", 2, precision );
		setVertexCB( ply, "red", 3, precision );
		setVertexCB( ply, "green", 4, precision );
		setVertexCB( ply, "blue", 5, precision );
		setVertexCB( ply, "nx", 6, precision );
		setVertexCB( ply, "ny", 7, precision );
		setVertexCB( ply, "nz", 8, precision );
		
		if( !ply_read( ply ) )
		{
			ply_close( ply );
			setlocale( LC_NUMERIC, old_locale );
			throw runtime_error( "Problem while reading points." );
		}
		
		// Verify the properties of the vertex element in the .ply file in order to set the normal flag.
		p_ply_element vertexElement = ply_get_next_element( ply, NULL );
		p_ply_property  property = NULL;
		for( int i = 0; i < 4; ++i )
		{
			property = ply_get_next_property( vertexElement, property );
		}
		char* name;
		ply_get_property_info( property, const_cast< const char** >( &name ), NULL, NULL, NULL );
		m_normalsFlag =  strcmp( name, "nx\0" ) ? false : true;
		
		ply_close( ply );
		
		/* Restore application locale when done */
		setlocale( LC_NUMERIC, old_locale );
	}
	
	template< typename Float, typename Vec3 >
	PointVector< Float, Vec3 > PlyPointReader< Float, Vec3 >::getPoints() { return m_points; }
	
	template< typename Float, typename Vec3 >
	bool PlyPointReader< Float, Vec3 >::hasNormals() { return m_normalsFlag; }
}

#endif