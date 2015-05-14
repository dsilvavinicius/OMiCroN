#ifndef OUT_OF_CORE_PLY_POINT_READER_H
#define OUT_OF_CORE_PLY_POINT_READER_H

#include <sqlite3.h>
#include "PlyPointReader.h"
#include "SQLiteHelper.h"

namespace util
{
	/** RPly vertex callback. */
	template< typename Float, typename Vec3, typename Point >
	int vertexOutOfCoreCB( p_ply_argument argument );
	
	template< typename Float, typename Vec3, typename Point >
	class OutOfCorePlyPointReader
	: public PlyPointReader< Float, Vec3, Point >
	{
		using PlyPointReader = util::PlyPointReader< Float, Vec3, Point >;
	public:
		/** @param database is the SQLite database where the points table will be created and the points will be added. */
		OutOfCorePlyPointReader( const string& fileName, Precision precision, Attributes attribs, sqlite3* database );
	
	protected:
		void setVertexCB( p_ply ply, string propName, const unsigned int propIndex, Precision precision ) override;
		
	private:
		/** Temp point used to hold intermediary incomplete data before sending it to database. */
		Point m_tempPoint;
		/** Point insertion statement. */
		sqlite3_stmt* m_pointInsertion;
		/** Data needed while reading points, in order to fill the database. */
		pair< Point*, sqlite3_stmt* > m_readingNeededData;
	};
	
	template< typename Float, typename Vec3, typename Point >
	OutOfCorePlyPointReader::OutOfCorePlyPointReader( const string& fileName, Precision precision, Attributes attribs,
													  sqlite3* database )
	: PlyPointReader( fileName, precision, attribs )
	{
		sqlite3_stmt* creationStmt;
		SQLiteHelper::safeCall(
			[ & ] ()
			{
			return sqlite3_prepare_v2( 	database,
										"CREATE TABLE Points ("
											"Id INT NOT NULL PRIMARY KEY AUTOINCREMENT,"
											"Point BLOB"
										");\0",
										-1, &creationStmt, NULL
									);
			}
		);
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_step( creationStmt ); } );
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_finalize( creationStmt ); } );
										
		SQLiteHelper::safeCall(
			[ & ] ()
			{
				return sqlite3_prepare_v2( database, "INSERT INTO Points( Point ) VALUES ( ? )\0", -1, &m_pointInsertion,
										   NULL );
			}
		);
		
		m_readingNeededData.first = &m_tempPoint;
		m_readingNeededData.second = m_pointInsertion;
	}
	
	template< typename Float, typename Vec3, typename Point >
	void OutOfCorePlyPointReader::setVertexCB( p_ply ply, string propName, const unsigned int propIndex,
											   Precision precision )
	{
		unsigned int propFlag = propIndex | ( precision << 4 );
		pair< Point*, sqlite3_stmt* > readingNeededData( m_tempPoint, m_pointInsertion );
		ply_set_read_cb( ply, "vertex", propName.c_str(), vertexOutOfCoreCB< Float, Vec3, Point >, &m_readingNeededData,
						 propFlag );
	}
	
	template< typename Float, typename Vec3, typename Point >
	struct OutOfCoreCBDataHandler
	{
		void operator()( const unsigned int& index, const float& value, pair< Point*, sqlite3_stmt* >* neededData )
		{
			switch( index )
			{
				case 0: case 1: case 2:
				{
					Point* point = neededData->first;
					( *point->getPos()[ index ] ) = value;
					break;
				}
				case 3: case 4:
				{
					// Flat color case.
					Point* point = neededData->first;
					( *point->getColor() )[ index % 3 ] = ( float ) value / 255;
					break;
				}
				case 5:
				{
					// Last point component. Send complete point to database.
					Point* point = neededData->first;
					( *point->getColor() )[ index % 3 ] = ( float ) value / 255;
					
					sqlite3_stmt* insertionStmt = neededData->second;
					
					// START HERE ON NEXT DAY! //
					void* blob = point;
					sqlite3_bind_blob( insertionStmt, 1, blob, sizeof( Point ), SQLITE_STATIC );
					sqlite3_step( insertionStmt );
					
					break;
				}
				case 6: case 7:
				{
					// Normal case.
					Point* point = neededData->first;
					( *point->getColor() )[ index % 3 ] = value;
					break;
				}
				case 8:
				{
					// Last point component. Send complete point to database.
					Point* point = neededData->first;
					( *point->getColor() )[ index % 3 ] = value;
					
					
					break;
				}
			}
		}
	};
	
	template< typename Float, typename Vec3 >
	struct OutOfCoreCBDataHandler< Float, Vec3, ExtendedPoint< Float,Vec3 > >
	{
		void operator()( const unsigned int& index, const float& value, pair< Point*, sqlite3_stmt* >* neededData )
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
	int vertexOutOfCoreCB( p_ply_argument argument )
	{
		long propFlag;
		void *rawNeededData;
		ply_get_argument_user_data( argument, &rawNeededData, &propFlag );
		pair< Point*, sqlite3_stmt* >* readingNeededData = ( pair< Point*, sqlite3_stmt* >* ) rawNeededData;
		
		float value = ply_get_argument_value( argument );
		
		unsigned int index = propFlag & 0xF;
		
		OutOfCoreCBDataHandler< Float, Vec3, Point > dataHandler;
		dataHandler( index, value, readingNeededData );
		
		return 1;
	}
}

#endif