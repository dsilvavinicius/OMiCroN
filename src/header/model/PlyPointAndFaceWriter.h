#ifndef PLY_POINT_AND_FACE_WRITER
#define PLY_POINT_AND_FACE_WRITER

#include <rply/rply.h>
#include "BasicTypes.h"
#include "Point.h"

namespace model
{
	class PlyPointAndFaceWriter
	{
	public:
		PlyPointAndFaceWriter( const string& filename, ulong nPoints, ulong nFaces );
		~PlyPointAndFaceWriter();
		
		void writeVertex( const Point& p );
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
		ply_add_property( m_ply, "nx", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32 );
		ply_add_property( m_ply, "ny", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32 );
		ply_add_property( m_ply, "nz", PLY_FLOAT32, PLY_FLOAT32, PLY_FLOAT32 );
		
		ply_add_element( m_ply, "face", nFaces);
		ply_add_list_property( m_ply, "vertex_indices", PLY_UCHAR, PLY_INT );
		
		ply_write_header( m_ply );
	}
	
	inline PlyPointAndFaceWriter::~PlyPointAndFaceWriter()
	{
		ply_close( m_ply );
	}
	
	inline void PlyPointAndFaceWriter::writeVertex( const Point& p )
	{
		ply_write( m_ply, p.getPos().x() );
		ply_write( m_ply, p.getPos().y() );
		ply_write( m_ply, p.getPos().z() );
		
		ply_write( m_ply, p.getNormal().x() );
		ply_write( m_ply, p.getNormal().y() );
		ply_write( m_ply, p.getNormal().z() );
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