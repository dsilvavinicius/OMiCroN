#ifndef OUT_OF_CORE_PLY_POINT_READER_H
#define OUT_OF_CORE_PLY_POINT_READER_H

#include <sqlite3.h>
#include "PlyPointReader.h"
#include "SQLiteHelper.h"

namespace util
{
	template< typename Float, typename Vec3, typename Point >
	class OutOfCorePlyPointReader
	: public PlyPointReader< Float, Vec3, Point >
	{
		using PlyPointReader = util::PlyPointReader< Float, Vec3, Point >;
	public:
		/** @param database is the SQLite database where the points table will be created and the points will be added. */
		OutOfCorePlyPointReader( sqlite3* database );
		~OutOfCorePlyPointReader();
	
	protected:
		int doRead( p_ply& ply, const typename PlyPointReader::Precision& precision, const bool& colorsNeeded,
					const bool& normalsNeeded ) override;
		
		static int vertexCB( p_ply_argument argument );
		
		sqlite3_stmt* m_pointInsertionStmt;
	};
	
	template< typename Float, typename Vec3, typename Point >
	OutOfCorePlyPointReader< Float, Vec3, Point >::OutOfCorePlyPointReader( sqlite3* database )
	: PlyPointReader()
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
				return sqlite3_prepare_v2( database, "INSERT INTO Points( Point ) VALUES ( ? )\0", -1,
										   &m_pointInsertionStmt, NULL );
			}
		);
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_finalize( creationStmt ); } );
	}
	
	template< typename Float, typename Vec3, typename Point >
	OutOfCorePlyPointReader< Float, Vec3, Point >::~OutOfCorePlyPointReader()
	{
		SQLiteHelper::safeCall( [ & ] () { return sqlite3_finalize( m_pointInsertionStmt ); } );
	}
	
	template< typename Float, typename Vec3, typename Point >
	int OutOfCorePlyPointReader< Float, Vec3, Point >::doRead( p_ply& ply, const typename PlyPointReader::Precision& precision,
															   const bool& colorsNeeded, const bool& normalsNeeded )
	{
		/** Temp point used to hold intermediary incomplete data before sending it to its final destiny. */
		Point tempPoint;
		pair< Point*, sqlite3_stmt* > cbNeededData( &tempPoint, m_pointInsertionStmt );
		
		ply_set_read_cb( ply, "vertex", "x", OutOfCorePlyPointReader::vertexCB, &cbNeededData, PlyPointReader::getPropFlag( 0, precision ) );
		ply_set_read_cb( ply, "vertex", "y", OutOfCorePlyPointReader::vertexCB, &cbNeededData, PlyPointReader::getPropFlag( 1, precision ) );
		ply_set_read_cb( ply, "vertex", "z", OutOfCorePlyPointReader::vertexCB, &cbNeededData, PlyPointReader::getPropFlag( 2, precision ) );
		
		if( colorsNeeded )
		{
			ply_set_read_cb( ply, "vertex", "red", OutOfCorePlyPointReader::vertexCB, &cbNeededData, PlyPointReader::getPropFlag( 3, precision ) );
			ply_set_read_cb( ply, "vertex", "green", OutOfCorePlyPointReader::vertexCB, &cbNeededData, PlyPointReader::getPropFlag( 4, precision ) );
			ply_set_read_cb( ply, "vertex", "blue", OutOfCorePlyPointReader::vertexCB, &cbNeededData, PlyPointReader::getPropFlag( 5, precision ) );
		}
		
		if( normalsNeeded )
		{
			ply_set_read_cb( ply, "vertex", "nx", OutOfCorePlyPointReader::vertexCB, &cbNeededData, PlyPointReader::getPropFlag( 6, precision ) );
			ply_set_read_cb( ply, "vertex", "ny", OutOfCorePlyPointReader::vertexCB, &cbNeededData, PlyPointReader::getPropFlag( 7, precision ) );
			ply_set_read_cb( ply, "vertex", "nz", OutOfCorePlyPointReader::vertexCB, &cbNeededData, PlyPointReader::getPropFlag( 8, precision ) );
		}
		
		return ply_read( ply );
	}
	
	/** Base to structs that handle callback data. Defines final destination in a database. */
	template< typename Float, typename Vec3, typename Point >
	struct OutOfCoreCBDataHandlerBase
	{
		/** Inserts the point into its final destination. */
		void insertPoint( sqlite3_stmt* insertionStmt, const Point& point )
		{
			const void* blob = &point;
			SQLiteHelper::safeCall(
				[ & ]
				{
					return sqlite3_bind_blob( insertionStmt, 1, blob, sizeof( Point ), SQLITE_STATIC );
				}
			);
			SQLiteHelper::safeCall(
				[ & ]
				{	
					return sqlite3_step( insertionStmt );
				}
			);
		}
	};
	
	template< typename Float, typename Vec3, typename Point >
	struct OutOfCoreCBDataHandler
	: public OutOfCoreCBDataHandlerBase< Float, Vec3, Point >
	{
		using CBDataHandlerBase = util::OutOfCoreCBDataHandlerBase< Float, Vec3, Point >;
		
		void operator()( const unsigned int& index, const float& value, pair< Point*, sqlite3_stmt* >* neededData )
		{
			Point* tempPoint = neededData->first;
			
			switch( index )
			{
				case 0: case 1: case 2:
				{
					( *tempPoint->getPos() )[ index ] = value;
					break;
				}
				case 3: case 4:
				{
					// Flat color case.
					( *tempPoint->getColor() )[ index % 3 ] = ( float ) value / 255;
					break;
				}
				case 5:
				{
					// Last point component. Send complete point to database.
					( *tempPoint->getColor() )[ index % 3 ] = ( float ) value / 255;
					CBDataHandlerBase::insertPoint( neededData->second, *tempPoint );
					break;
				}
				case 6: case 7:
				{
					// Normal case.
					( *tempPoint->getColor() )[ index % 3 ] = value;
					break;
				}
				case 8:
				{
					// Last point component. Send complete point to database.
					( *tempPoint->getColor() )[ index % 3 ] = value;
					CBDataHandlerBase::insertPoint( neededData->second, *tempPoint );
					break;
				}
			}
		}
	};
	
	template< typename Float, typename Vec3 >
	struct OutOfCoreCBDataHandler< Float, Vec3, ExtendedPoint< Float,Vec3 > >
	{
		using Point = ExtendedPoint< Float, Vec3 >;
		using CBDataHandlerBase = util::OutOfCoreCBDataHandlerBase< Float, Vec3, Point >;
		
		void operator()( const unsigned int& index, const float& value, pair< Point*, sqlite3_stmt* >* neededData )
		{
			Point* tempPoint = neededData->first;
			
			switch( index )
			{
				case 0: case 1: case 2:
				{
					( *tempPoint->getPos() )[ index ] = value;
					break;
				}
				case 3: case 4: case 5:
				{
					// Flat color case.
					( *tempPoint->getColor() )[ index % 3 ] = ( float ) value / 255;
					break;
				}
				case 6: case 7:
				{
					// Normal case.
					( *tempPoint->getNormal() )[ index % 3 ] = value;
					break;
				}
				case 8:
				{
					// Last point component. Send complete point to vector.
					( *tempPoint->getNormal() )[ index % 3 ] = ( float ) value;
					CBDataHandlerBase::insertPoint( neededData->second, *tempPoint );
					break;
				}
			}
		}
	};
	
	/** RPly vertex callback. */
	template< typename Float, typename Vec3, typename Point >
	int OutOfCorePlyPointReader< Float, Vec3, Point >::vertexCB( p_ply_argument argument )
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