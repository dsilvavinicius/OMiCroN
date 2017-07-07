#ifndef PLY_POINT_AND_FACE_WRITER
#define PLY_POINT_AND_FACE_WRITER

#include <rply.h>
#include "BasicTypes.h"

namespace model
{
	class PlyPointAndFaceWriter
	{
	public:
		PlyPointAndFaceWriter( const string& filename, ulong nPoints, ulong nFaces );
		~PlyPointAndFaceWriter();
		
		void writePos( const Vec3& pos );
		void writeTri( const Vec3& tri );
		
	private:
		p_ply m_ply;
	};
	
	inline PlyPointAndFaceWriter::PlyPointAndFaceWriter( const string& filename, ulong nPoints, ulong nFaces )
	: m_ply( ply_create( filename.c_str(), PLY_LITTLE_ENDIAN, NULL, 0, NULL ) )
	{
		ply_add_element( m_ply, "vertex", nPoints);
		ply_add_property( m_ply, "x", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32 );
		ply_add_property( m_ply, "y", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32 );
		ply_add_property( m_ply, "z", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32 );
		
		ply_add_element( m_ply, "face", nFaces);
		ply_add_list_property( m_ply, "vertex_indices", PLY_UCHAR, PLY_INT );
		
		ply_write_header( m_ply );
	}
	
	inline PlyPointAndFaceWriter::~PlyPointAndFaceWriter()
	{
		ply_close( m_ply );
	}
	
	inline void PlyPointAndFaceWriter::writePos( const Vec3& pos )
	{
		ply_write( m_ply, pos.x() );
		ply_write( m_ply, pos.y() );
		ply_write( m_ply, pos.z() );
	}
	
	inline void PlyPointAndFaceWriter::writeTri( const Vec3& tri )
	{
		ply_write( m_ply, 3.f );
		ply_write( m_ply, tri.x() );
		ply_write( m_ply, tri.y() );
		ply_write( m_ply, tri.z() );
	}
}

#endif