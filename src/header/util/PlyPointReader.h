#ifndef PLY_POINT_READER_H
#define PLY_POINT_READER_H

#include <locale.h>
#include <string>
#include <rply.h>
#include "Point.h"
#include "renderers/RenderingState.h"

using namespace std;
using namespace model;

namespace util
{
	class PlyPointWritter;

	/** Reader for a point .ply file. The file is opened at constructor and closed at destructor. */
	class PlyPointReader 
	{
	public:
		enum Precision
		{
			SINGLE = 0x1,
			DOUBLE = 0x2
		};
		
		friend PlyPointWritter;
		
		/** Checks if the file is valid, opens it, reads its header and discovers the number of points in it.
		 * @throws runtime_error if the file or its header cannot be read.
		 * @param onPointDone is a function that process a read point. */
		PlyPointReader( const string& fileName );
		
		~PlyPointReader() { ply_close( m_ply ); }
		
		/** Copies the header of the file managed by this reader to other file. */
		p_ply copyHeader( const string& outFilename );
		
		/** Reads a .ply file. . 
		 * @param precision specifies the desired precision for read points. It must agree with the template
		 * parameters of this method. */
		void read( const function< void( const Point& ) >& onPointDone, Precision precision = SINGLE );
	
		long getNumPoints() { return m_numPoints; }
	
	protected:
		void setupAdditionalCallbacks( p_ply ply, const Precision& precision,
									   pair< Point*, function< void( const Point& ) >* >& cbNeededData );
		
		/** Internal customizable reading method. Should setup reading needed data, callbacks, do the reading itself and free reading
		 * needed data.
		 * @returns ply_read() return code. */
		virtual int doRead( p_ply& ply, const Precision& precision );
		
		/** Copies the property to out's header. */
		void copyProperty( p_ply_property property, p_ply out );
		
		/** Internal method that calculates the property flag for each invocation of the reading vertex callback. */
		static unsigned int getPropFlag( const unsigned int& propIndex, const Precision& precision );
		
		/** Method used as RPly vertex callback. */
		static int vertexCB( p_ply_argument argument );
		
		long m_numPoints;
		
		function< void( const Point& ) > m_onPointDone;
		
		p_ply m_ply;
		
		string m_filename;
	};
	
	inline PlyPointReader::PlyPointReader( const string& fileName )
	: m_filename( fileName )
	{
		cout << "Setup read of " << m_filename << endl << endl;
		
		// Open ply.
		m_ply = ply_open( m_filename.c_str(), NULL, 0, NULL );
		if( !m_ply )
		{
			throw runtime_error( m_filename + ": cannot open .ply point file." );
		}
		
		// Read header.
		if( !ply_read_header( m_ply ) )
		{
			ply_close( m_ply );
			throw runtime_error( "Cannot read point file header." );
		}
		
		// Verify the properties of the vertex element in the .ply file in order to set the normal flag.
		p_ply_element element = ply_get_next_element( m_ply, NULL );
		ply_get_element_info( element, NULL, &m_numPoints );
		
		cout << "Vertices in file: " << m_numPoints << endl << endl << "=== Elements in header ===" << endl << endl;
		
		while( element != NULL )
		{
			p_ply_property  property = ply_get_next_property( element, NULL );
			long numElements = 0;
			const char* elementName;
			ply_get_element_info( element, &elementName, &numElements );
			
			cout << elementName << ": " << numElements << " instances" << endl;
			
			while( property != NULL )
			{
				const char* name;
				ply_get_property_info( property, &name, NULL, NULL, NULL );
				cout << "Prop name: " << name << endl;
				property = ply_get_next_property( element, property );
			}
			
			cout << endl;
			
			element = ply_get_next_element( m_ply, element );
		}
	}
	
	inline void PlyPointReader::read( const function< void( const Point& ) >& onPointDone, Precision precision )
	{
		m_onPointDone = onPointDone;
		
		/* Save application locale */
// 		const char *old_locale = setlocale( LC_NUMERIC, NULL );
		/* Change to PLY standard */
// 		setlocale( LC_NUMERIC, "C" );
		
		int resultCode = doRead( m_ply, precision );
		
		if( !resultCode )
		{
			ply_close( m_ply );
// 			setlocale( LC_NUMERIC, old_locale );
			throw runtime_error( "Problem while reading points." );
		}
		
		/* Restore application locale when done */
// 		setlocale( LC_NUMERIC, old_locale );
	}
	
	inline int PlyPointReader::doRead( p_ply& ply, const Precision& precision )
	{
		/** Temp point used to hold intermediary incomplete data before sending it to its final destiny. */
		Point tempPoint;
		pair< Point*, function< void( const Point& ) >* > cbNeededData( &tempPoint, &m_onPointDone );
		
		ply_set_read_cb( ply, "vertex", "x", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 0, precision ) );
		ply_set_read_cb( ply, "vertex", "y", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 1, precision ) );
		ply_set_read_cb( ply, "vertex", "z", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 2, precision ) );
		
		ply_set_read_cb( ply, "vertex", "nx", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 3, precision ) );
		ply_set_read_cb( ply, "vertex", "ny", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 4, precision ) );
		ply_set_read_cb( ply, "vertex", "nz", PlyPointReader::vertexCB, &cbNeededData, getPropFlag( 5, precision ) );
		
		return ply_read( ply );
	}
	
	inline unsigned int PlyPointReader::getPropFlag( const unsigned int& propIndex, const Precision& precision )
	{
		return propIndex | ( precision << 4 );
	}
	
	/** Struct to handle callback data. Agnostic to point type. */
	struct CBDataHandler
	{
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
					tempPoint->getNormal()[ index % 3 ] = value;
					break;
				}
				case 5:
				{
					// Last point component. Send complete point to vector.
					tempPoint->getNormal()[ index % 3 ] = ( float ) value;
					( *readingData->second )( Point( tempPoint->getNormal(), tempPoint->getPos() ) );
					break;
				}
				case 6: case 7:
				{
					// Flat color case.
					tempPoint->getNormal()[ index % 3 ] = ( float ) value / 255;
					break;
				}
				case 8:
				{
					// Last point component. Send complete point to vector.
					tempPoint->getNormal()[ index % 3 ] = ( float ) value / 255;
					( *readingData->second )( Point( tempPoint->getNormal(), tempPoint->getPos() ) );
					break;
				}
			}
		}
	};
	
	inline int PlyPointReader::vertexCB( p_ply_argument argument )
	{
		long propFlag;
		void *rawReadingData;
		ply_get_argument_user_data( argument, &rawReadingData, &propFlag );
		
		auto readingData = ( pair< Point*, function< void( const Point& ) >* >* ) rawReadingData;
		
		float value = ply_get_argument_value( argument );
		
		unsigned int index = propFlag & 0xF;
		
		CBDataHandler dataHandler;
		dataHandler( index, value, readingData );
		
		return 1;
	}
}

#endif