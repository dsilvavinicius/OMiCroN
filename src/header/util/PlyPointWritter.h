#ifndef PLY_POINT_WRITTER_H
#define PLY_POINT_WRITTER_H

#include "PlyPointReader.h"

namespace util
{
	template< typename Point >
	class PlyPointWritter
	{
		using Reader = PlyPointReader< Point >;
		
	public:
		PlyPointWritter( const Reader& reader, const string& filename );
		~PlyPointWritter() { ply_close( m_ply ); }
		void write( const Point& p );
		const string& filename() { return m_filename; }
		
	private:
		void copyProperty( p_ply_property property );
		
		string m_filename;
		p_ply m_ply;
	};
	
	template< typename Point >
	inline PlyPointWritter< Point >::PlyPointWritter( const Reader& reader, const string& filename )
	: m_filename( filename ),
	m_ply( ply_create( m_filename.c_str(), PLY_LITTLE_ENDIAN, NULL, 0, NULL ) )
	{
		if( !m_ply )
		{
			throw runtime_error( m_filename + ": cannot open .ply file to write." );
		}
		p_ply_element element = NULL;
		
		/* iterate over all elements in input file */
		while( ( element = ply_get_next_element( reader.m_ply, element ) ) )
		{
			p_ply_property property = NULL;
			long ninstances = 0;
			const char *element_name;
			ply_get_element_info( element, &element_name, &ninstances );
			
			/* add this element to output file */
			if( !ply_add_element( m_ply, element_name, ninstances ) )
			{
				throw runtime_error( "Cannot copy element to .ply header." );
			}
			
			/* iterate over all properties of current element */
			while( ( property = ply_get_next_property( element, property ) ) )
			{
				copyProperty( property );
			}
		}
		
		if( !ply_write_header( m_ply ) )
		{
			throw runtime_error( "Cannot write .ply header." );
		}
	}
}

#endif