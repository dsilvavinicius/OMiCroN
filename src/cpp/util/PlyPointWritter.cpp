#include "PlyPointWritter.h"

namespace util
{
	template<>
	void PlyPointWritter< Point >::write( const Point& p )
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
	
	template<>
	void PlyPointWritter< ExtendedPoint >::write( const ExtendedPoint& p )
	{
		const Vec3& pos = p.getPos();
		ply_write( m_ply, pos.x() );
		ply_write( m_ply, pos.y() );
		ply_write( m_ply, pos.z() );
		
		const Vec3& normal = p.getNormal();
		ply_write( m_ply, normal.x() );
		ply_write( m_ply, normal.y() );
		ply_write( m_ply, normal.z() );
		
		const Vec3& color = p.getNormal();
		ply_write( m_ply, color.x() );
		ply_write( m_ply, color.y() );
		ply_write( m_ply, color.z() );
	}
	
	template<>
	void PlyPointWritter< Point >::copyProperty( p_ply_property property )
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
	
	template<>
	void PlyPointWritter< ExtendedPoint >::copyProperty( p_ply_property property )
	{
		const char *property_name;
		e_ply_type type, length_type, value_type;
		ply_get_property_info( property, &property_name, &type, &length_type, &value_type );
		
		if( !strcmp( property_name, "x" ) || !strcmp( property_name, "y" ) || !strcmp( property_name, "z" ) ||
			!strcmp( property_name, "nx" ) || !strcmp( property_name, "ny" ) || !strcmp( property_name, "nz" ) ||
			!strcmp( property_name, "red" ) || !strcmp( property_name, "green" ) || !strcmp( property_name, "blue" ) )
		{
			// Debug
			{
				cout << "Copy prop: " << property_name << endl << endl;
			}
			
			/* add this property to output file */
			if( !ply_add_property( m_ply, property_name, type, length_type, value_type ) )
			{
				throw runtime_error( "Cannot copy property to .ply header." );
			}
		}
	}
}