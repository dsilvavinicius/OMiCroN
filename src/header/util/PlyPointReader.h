#ifndef PLY_POINT_READER_H
#define PLY_POINT_READER_H

#include <locale.h>
#include <string>
#include <rply.h>
#include "ExtendedPoint.h"
#include "RenderingState.h"

using namespace std;
using namespace model;

namespace util
{
	/** Reader for a point .ply file. To access the read points, use getPoints(). */
	// TODO: Specialize this class so the precision is infered automatically by template parameters.
	template< typename Point >
	class PlyPointReader 
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
	public:
		enum Precision
		{
			SINGLE = 0x1,
			DOUBLE = 0x2
		};
		
		/** @param onPointDone is a function that process a read point. */
		PlyPointReader( const function< void( const Point& ) >& onPointDone );
		
		/** Reads a .ply file. . 
		 * @param precision specifies the desired precision for read points. It must agree with the template
		 * parameters of this method. */
		void read( const string& fileName, Precision precision, Attributes attribs );
		
		Attributes getAttributes(){ return m_attribs; }
	
	protected:
		/** Internal customizable reading method. Should setup reading needed data, callbacks, do the reading itself and free reading
		 * needed data.
		 * @returns ply_read() return code. */
		virtual int doRead( p_ply& ply, const Precision& precision, const bool& colorsNeeded, const bool& normalsNeeded );
		
		/** Internal method that calculates the property flag for each invocation of the reading vertex callback. */
		static unsigned int getPropFlag( const unsigned int& propIndex, const Precision& precision );
		
		/** Method used as RPly vertex callback. */
		static int vertexCB( p_ply_argument argument );
		
		Attributes m_attribs;
		
		function< void( const Point& ) > m_onPointDone;
	};
	
	template< typename Point >
	PlyPointReader< Point >::PlyPointReader( const function< void( const Point& ) >& onPointDone )
	: m_onPointDone( onPointDone )
	{}
	
	template< typename Point >
	void PlyPointReader< Point >::read( const string& fileName, Precision precision, Attributes attribs )
	{
		/* Save application locale */
		const char *old_locale = setlocale( LC_NUMERIC, NULL );
		/* Change to PLY standard */
		setlocale( LC_NUMERIC, "C" );

		p_ply ply = ply_open( fileName.c_str(), NULL, 0, NULL );
		if( !ply )
		{
			setlocale( LC_NUMERIC, old_locale );
			throw runtime_error( fileName + ": cannot open .ply point file." );
		}
		if( !ply_read_header( ply ) )
		{
			ply_close( ply );
			setlocale( LC_NUMERIC, old_locale );
			throw runtime_error( "Cannot read point file header." );
		}
		
		// Verify the properties of the vertex element in the .ply file in order to set the normal flag.
		p_ply_element vertexElement = ply_get_next_element( ply, NULL );
		p_ply_property  property = ply_get_next_property( vertexElement, NULL );
		
		bool colorsNeeded = attribs & COLORS;
		bool normalsNeeded = attribs & NORMALS;
		int rawAttribs = 0x0;
		
		{
			long nPoints;
			ply_get_element_info( vertexElement, NULL, &nPoints );
			cout << "Points in file: " << nPoints << endl;
		}
		
		while( property != NULL )
		{
			char* name;
			ply_get_property_info( property, const_cast< const char** >( &name ), NULL, NULL, NULL );
			
			if( colorsNeeded && !strcmp( name, "red\0" ) )
			{
				rawAttribs |= model::COLORS;
				m_attribs = Attributes( rawAttribs );
			}
			
			if( normalsNeeded && !strcmp( name, "nx\0" ) )
			{
				rawAttribs |= model::NORMALS;
				m_attribs = Attributes( rawAttribs );
			}
			
			cout << "Prop name: " << name << endl;
			property = ply_get_next_property( vertexElement, property );
		}
		cout << endl;
		
		int resultCode = doRead( ply, precision, colorsNeeded, normalsNeeded );
		
		if( !resultCode )
		{
			ply_close( ply );
			setlocale( LC_NUMERIC, old_locale );
			throw runtime_error( "Problem while reading points." );
		}
		
		ply_close( ply );
		
		/* Restore application locale when done */
		setlocale( LC_NUMERIC, old_locale );
	}
	
	template< typename Point >
	int PlyPointReader< Point >::doRead( p_ply& ply, const Precision& precision, const bool& colorsNeeded,
													   const bool& normalsNeeded )
	{
		/** Temp point used to hold intermediary incomplete data before sending it to its final destiny. */
		Point tempPoint;
		pair< Point*, function< void( const Point& ) >* > cbNeededData( &tempPoint, &m_onPointDone );
		
		ply_set_read_cb( ply, "vertex", "x", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 0, precision ) );
		ply_set_read_cb( ply, "vertex", "y", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 1, precision ) );
		ply_set_read_cb( ply, "vertex", "z", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 2, precision ) );
		
		if( normalsNeeded )
		{
			ply_set_read_cb( ply, "vertex", "nx", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 3, precision ) );
			ply_set_read_cb( ply, "vertex", "ny", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 4, precision ) );
			ply_set_read_cb( ply, "vertex", "nz", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 5, precision ) );
		}
		
		if( colorsNeeded )
		{
			ply_set_read_cb( ply, "vertex", "red", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 6, precision ) );
			ply_set_read_cb( ply, "vertex", "green", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 7, precision ) );
			ply_set_read_cb( ply, "vertex", "blue", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 8, precision ) );
		}
		
		return ply_read( ply );
	}
	
	template< typename Point >
	inline unsigned int PlyPointReader< Point >::getPropFlag( const unsigned int& propIndex,
																		   const Precision& precision )
	{
		return propIndex | ( precision << 4 );
	}
	
	/** Struct to handle callback data. Agnostic to point type. */
	template< typename Point >
	struct CBDataHandler
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		
		void operator()( const unsigned int& index, const float& value,
						 pair< Point*, function< void( const Point& ) >* >* readingData )
		{
			Point* tempPoint = readingData->first;
			
			switch( index )
			{
				case 0: case 1: case 2:
				{
					tempPoint->getPos()[ index ] = value;
					break;
				}
				case 3: case 4:
				{
					// Normal case.
					tempPoint->getColor()[ index % 3 ] = value;
					break;
				}
				case 5:
				{
					// Last point component. Send complete point to vector.
					tempPoint->getColor()[ index % 3 ] = ( float ) value;
					( *readingData->second )( Point( tempPoint->getColor(), tempPoint->getPos() ) );
					break;
				}
				case 6: case 7:
				{
					// Flat color case.
					tempPoint->getColor()[ index % 3 ] = ( float ) value / 255;
					break;
				}
				case 8:
				{
					// Last point component. Send complete point to vector.
					tempPoint->getColor()[ index % 3 ] = ( float ) value / 255;
					( *readingData->second )( Point( tempPoint->getColor(), tempPoint->getPos() ) );
					break;
				}
			}
		}
	};
	
	/** Struct to handle callback data. Specialization for points with both colors and normals. */
	template<>
	struct CBDataHandler< ExtendedPoint >
	{
		using Point = ExtendedPoint;
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		
		void operator()( const unsigned int& index, const float& value,
						 pair< Point*, function< void( const Point& ) >* >* readingData )
		{
			Point* tempPoint = readingData->first;
			
			switch( index )
			{
				case 0: case 1: case 2:
				{
					tempPoint->getPos()[ index ] = value;
					break;
				}
				case 3: case 4: case 5:
				{
					// Normal case.
					tempPoint->getNormal()[ index % 3 ] = value;
					break;
				}
				case 6: case 7:
				{
					// Flat color case.
					tempPoint->getColor()[ index % 3 ] = ( float ) value / 255;
					break;
				}
				case 8:
				{
					// Last point component. Send complete point to vector.
					// Flat color case.
					tempPoint->getColor()[ index % 3 ] = ( float ) value / 255;
					( *readingData->second )( Point( tempPoint->getColor(), tempPoint->getNormal(), tempPoint->getPos() ) );
					break;
				}
			}
		}
	};
	
	template< typename Point >
	int PlyPointReader< Point >::vertexCB( p_ply_argument argument )
	{
		using PointPtr = shared_ptr< Point >;
		using PointVector = vector< PointPtr, ManagedAllocator< PointPtr > >;
		
		long propFlag;
		void *rawReadingData;
		ply_get_argument_user_data( argument, &rawReadingData, &propFlag );
		
		auto readingData = ( pair< Point*, function< void( const Point& ) >* >* ) rawReadingData;
		
		float value = ply_get_argument_value( argument );
		
		unsigned int index = propFlag & 0xF;
		
		CBDataHandler< Point > dataHandler;
		dataHandler( index, value, readingData );
		
		return 1;
	}
	
	// ====================== Type Sugar ================================ /
	
	/** Reader for points with position and color or normal. */
	using SimplePointReader = PlyPointReader< Point >;
	
	/** Reader for points with position, color and normal. */
	using ExtendedPointReader = PlyPointReader< ExtendedPoint >;
}

#endif