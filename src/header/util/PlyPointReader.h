#ifndef PLY_POINT_READER_H
#define PLY_POINT_READER_H

#include <locale.h>
#include <string>
#include "rply/rply.h"
#include "ExtendedPoint.h"

using namespace std;
using namespace model;

namespace util
{
	template< typename Float, typename Vec3, typename Point >
	struct CBDataHandler
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		void operator()( const unsigned int& index, const float& value, PointVector* points )
		{
			switch( index )
			{
				case 0:
				{
					auto point = make_shared< Point >( vec3( 1.f, 1.f, 1.f ), vec3( value, 0.f, 0.f ) );
					points->push_back( point );
					break;
				}
				case 1: case 2:
				{
					shared_ptr< Point > point = points->back();
					( *point->getPos() )[ index ] = value;
					break;
				}
				case 3: case 4: case 5:
				{
					// Flat color case.
					shared_ptr< Point > point = points->back();
					( *point->getColor() )[ index % 3 ] = ( float ) value / 255;
					break;
				}
				case 6: case 7: case 8:
				{
					// Normal case.
					shared_ptr< Point > point = points->back();
					( *point->getColor() )[ index % 3 ] = value;
					break;
				}
			}
		}
	};
	
	template< typename Float, typename Vec3 >
	struct CBDataHandler< Float, Vec3, ExtendedPoint< Float,Vec3 > >
	{
		using Point = ExtendedPoint< Float, Vec3 >;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		
		void operator()( const unsigned int& index, const float& value, PointVector* points )
		{
			switch( index )
			{
				case 0:
				{
					auto point = make_shared< Point >( vec3( 1.f, 1.f, 1.f ), vec3( 1.f, 1.f, 1.f ), vec3( value, 0.f, 0.f ) );
					points->push_back( point );
					break;
				}
				case 1: case 2:
				{
					shared_ptr< Point > point = points->back();
					( *point->getPos() )[ index ] = value;
					break;
				}
				case 3: case 4: case 5:
				{
					// Flat color.
					shared_ptr< Point > point = points->back();
					( *point->getColor() )[ index % 3 ] = ( float ) value / 255;
					break;
				}
				case 6: case 7: case 8:
				{
					// Normal.
					shared_ptr< Point > point = points->back();
					( *point->getNormal() )[ index % 3 ] = value;
					break;
				}
			}
		}
	};
	
	/** RPly vertex callback. */
	template< typename Float, typename Vec3, typename Point >
	int vertexCB( p_ply_argument argument )
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
		
		long propFlag;
		void *rawPoints;
		ply_get_argument_user_data( argument, &rawPoints, &propFlag );
		PointVector* points = ( PointVector* ) rawPoints;
		
		float value = ply_get_argument_value( argument );
		
		unsigned int index = propFlag & 0xF;
		
		CBDataHandler< Float, Vec3, Point > dataHandler;
		dataHandler( index, value, points );
		
		return 1;
	}
	
	/** RPly vertex callback for models with color and normal. */
	//template< >
	//void handleCBData< ExtendedPoint >( p_ply_argument argument );
	
	/** Reader for a point .ply file. To access the read points, use getPoints(). To verify if the model vertices have
	 * normals or just float colors, use hasNormals(). */
	template< typename Float, typename Vec3, typename Point >
	class PlyPointReader 
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr >;
	public:
		enum Precision
		{
			SINGLE = 0x1,
			DOUBLE = 0x2
		};
		
		enum Attributes
		{
			COLORS = 0x1,
			NORMALS = 0x2,
			COLORS_AND_NORMALS = 0x3
		};
		
		// TODO: Specialize this class so the precision is infered automatically by template parameters.
		/** Reads a .ply file. . 
		 * @param precision specifies the desired precision for read points. It must be agreed with the template
		 * parameters of this method. */
		PlyPointReader( const string& fileName, Precision precision, Attributes attribs );
		
		PointVector getPoints(){ return m_points; }
		bool hasColors(){ return m_colorsFlag; }
		bool hasNormals(){ return m_normalsFlag; }
	private:
		/** Sets the RPly callback for vertex data, given a property index and a precision flag. */
		void setVertexCB( p_ply ply, string propName, const unsigned int propIndex, Precision precision );
		
		PointVector m_points;
		bool m_colorsFlag;
		bool m_normalsFlag;
	};
	
	template< typename Float, typename Vec3, typename Point >
	void PlyPointReader< Float, Vec3, Point >::setVertexCB( p_ply ply, string propName, const unsigned int propIndex,
													 Precision precision )
	{
		unsigned int propFlag = propIndex | ( precision << 4 );
		ply_set_read_cb( ply, "vertex", propName.c_str(), vertexCB< Float, Vec3, Point >, &m_points, propFlag );
	}
	
	template< typename Float, typename Vec3, typename Point >
	PlyPointReader< Float, Vec3, Point >::PlyPointReader( const string& fileName, Precision precision, Attributes attribs )
	: m_normalsFlag( false ),
	m_colorsFlag( false )
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
		
		// Verify the properties of the vertex element in the .ply file in order to set the normal flag.
		p_ply_element vertexElement = ply_get_next_element( ply, NULL );
		p_ply_property  property = ply_get_next_property( vertexElement, property );
		
		bool colorsNeeded = attribs & COLORS;
		bool normalsNeeded = attribs & NORMALS;
		
		while( property != NULL )
		{
			char* name;
			ply_get_property_info( property, const_cast< const char** >( &name ), NULL, NULL, NULL );
			
			if( colorsNeeded && !strcmp( name, "red\0" ) )
			{
				m_colorsFlag = true;
			}
			
			if( normalsNeeded && !strcmp( name, "nx\0" ) )
			{
				m_normalsFlag = true;
			}
			
			cout << "Prop name: " << name << endl;
			property = ply_get_next_property( vertexElement, property );
		}
		cout << endl;
		
		// Set callbacks for reading.
		setVertexCB( ply, "x", 0, precision );
		setVertexCB( ply, "y", 1, precision );
		setVertexCB( ply, "z", 2, precision );
		
		if( colorsNeeded )
		{
			setVertexCB( ply, "red", 3, precision );
			setVertexCB( ply, "green", 4, precision );
			setVertexCB( ply, "blue", 5, precision );
		}
		
		if( normalsNeeded )
		{
			setVertexCB( ply, "nx", 6, precision );
			setVertexCB( ply, "ny", 7, precision );
			setVertexCB( ply, "nz", 8, precision );
		}
		
		if( !ply_read( ply ) )
		{
			ply_close( ply );
			setlocale( LC_NUMERIC, old_locale );
			throw runtime_error( "Problem while reading points." );
		}
		
		ply_close( ply );
		
		/* Restore application locale when done */
		setlocale( LC_NUMERIC, old_locale );
	}
	
	/** Reader for points with position and color or normal. */
	using SimplePointReader = PlyPointReader< float, vec3, Point< float, vec3 > >;
	
	/** Reader for points with position, color and normal. */
	using ExtendedPointReader = PlyPointReader< float, vec3, ExtendedPoint< float, vec3 > >;
}

#endif