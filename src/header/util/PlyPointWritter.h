#ifndef PLY_POINT_WRITTER_H
#define PLY_POINT_WRITTER_H

#include "PlyPointReader.h"

namespace util
{
	class PlyPointWritter
	{
		using Reader = PlyPointReader;
		
	public:
		PlyPointWritter( const Reader& reader, const string& filename, ulong nPoints );
		~PlyPointWritter() { ply_close( m_ply ); }
		void write( const Point& p );
		const string& filename() { return m_filename; }
		
	private:
		void copyProperty( p_ply_property property );
		
		string m_filename;
		p_ply m_ply;
	};
	
	inline PlyPointWritter::PlyPointWritter( const Reader& reader, const string& filename, ulong nPoints )
	: m_filename( filename ),
	m_ply( ply_create( m_filename.c_str(), PLY_LITTLE_ENDIAN, NULL, 0, NULL ) )
	{
		if( !m_ply )
		{
			throw runtime_error( m_filename + ": cannot open .ply file to write." );
		}
		p_ply_element element = ply_get_next_element( reader.m_ply, NULL );
		p_ply_property property = NULL;
		const char *element_name;
		ply_get_element_info( element, &element_name, NULL );
		
		/* add this element to output file */
		if( !ply_add_element( m_ply, element_name, nPoints ) )
		{
			throw runtime_error( "Cannot copy element to .ply header." );
		}
		
		/* iterate over all properties of current element */
		while( ( property = ply_get_next_property( element, property ) ) )
		{
			copyProperty( property );
		}
		
		if( !ply_write_header( m_ply ) )
		{
			throw runtime_error( "Cannot write .ply header." );
		}
	}
	
	inline void PlyPointWritter::write( const Point& p )
	{
		const Vec3& pos = p.getPos();
		ply_write( m_ply, pos.x() );
		ply_write( m_ply, pos.y() );
		ply_write( m_ply, pos.z() );
		
		const Vec3& normal = p.getNormal();
		ply_write( m_ply, normal.x() );
		ply_write( m_ply, normal.y() );
		ply_write( m_ply, normal.z() );
	}
	
	inline void PlyPointWritter::copyProperty( p_ply_property property )
	{
		const char *property_name;
		e_ply_type type, length_type, value_type;
		ply_get_property_info( property, &property_name, &type, &length_type, &value_type );
		
		if( !strcmp( property_name, "x" ) || !strcmp( property_name, "y" ) || !strcmp( property_name, "z" ) ||
			!strcmp( property_name, "nx" ) || !strcmp( property_name, "ny" ) || !strcmp( property_name, "nz" ) )
		{
			/* add this property to output file */
			if( !ply_add_property( m_ply, property_name, type, length_type, value_type ) )
			{
				throw runtime_error( "Cannot copy property to .ply header." );
			}
		}
	}
}

#endif