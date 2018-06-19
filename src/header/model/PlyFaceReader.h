#ifndef PLY_FACE_READER_H
#define PLY_FACE_READER_H

#include <rply/rply.h>
#include <iostream>
#include "BasicTypes.h"


using namespace std;

namespace model
{
	class PlyFaceReader
	{
	public:
		PlyFaceReader( const string& filename, const function< void( const Vec3& ) >& onFaceDone );
		~PlyFaceReader();
		
		void read();
		
		static int callback( p_ply_argument argument );
		
		uint numFaces() { return m_nFaces; }
		
	private:
		const function< void( const Vec3& ) >& m_onFaceDone;
		Vec3 m_tempFace;
		ulong m_nCallbackCalls;
		p_ply m_ply;
		uint m_nFaces;
	};
	
	inline PlyFaceReader::PlyFaceReader( const string& filename, const function< void( const Vec3& ) >& onFaceDone )
	: m_onFaceDone( onFaceDone ),
	m_tempFace( 0.f, 0.f, 0.f ),
	m_nCallbackCalls( 0ul ),
	m_ply( NULL ),
	m_nFaces( 0u )
	{
		cout << "Setup read of " << filename << endl << endl;
		
		// Open ply.
		m_ply = ply_open( filename.c_str(), NULL, 0, NULL );
		if( !m_ply )
		{
			throw runtime_error( filename + ": cannot open .ply point file." );
		}
		
		// Read header.
		if( !ply_read_header( m_ply ) )
		{
			ply_close( m_ply );
			throw runtime_error( "Cannot read point file header." );
		}
		
		m_nFaces = ply_set_read_cb( m_ply, "face", "vertex_indices", PlyFaceReader::callback, this, 0 );
		
	}
	
	inline PlyFaceReader::~PlyFaceReader()
	{
		ply_close( m_ply );
	}
	
	inline void PlyFaceReader::read()
	{
		ply_read( m_ply );
	}
	
	inline int PlyFaceReader::callback( p_ply_argument argument )
	{
		void *rawReader;
		ply_get_argument_user_data( argument, &rawReader, NULL );
		auto reader = ( PlyFaceReader* ) rawReader;
		float value = ply_get_argument_value( argument );
		
		int listIndex = reader->m_nCallbackCalls++ % 4;
		
		if( listIndex > 0 )
		{
			reader->m_tempFace[ listIndex - 1 ] = value;
		}
		
		if( listIndex == 3 )
		{
			reader->m_onFaceDone( reader->m_tempFace );
		}
		
		return 1;
	}
}

#endif