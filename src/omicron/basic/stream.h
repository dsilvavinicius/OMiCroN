#ifndef STREAM_H
#define STREAM_H

#include <vector>
#include <set>
#include <iostream>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <eigen3/Eigen/Geometry>
#include "omicron/basic/point.h"

using namespace std;
using namespace Eigen;

namespace omicron::basic
{
	template< typename T, typename A >
	ostream& operator<<( ostream& out, const vector< T, A >& v )
	{
		out << "size: " << v.size() << endl << "{" << endl;
		bool first = true;
		for( T element : v )
		{
			if( first )
			{
				out << element;
				first = false;
			}
			else
			{
				out << endl << ", " << endl << element;
			}
		}
		out << "}" << endl;
		
		return out;
	}
	
	template< typename T, typename A >
	ostream& operator<<( ostream& out, const vector< T*, A >& v )
	{
		out << "size: " << v.size() << endl << "{" << endl;
		bool first = true;
		for( T* element : v )
		{
			if( first )
			{
				out << *element;
				first = false;
			}
			else
			{
				out << endl << ", " << endl << *element;
			}
		}
		out << "}" << endl;
		
		return out;
	}
	
	template< typename T >
	ostream& operator<<( ostream& out, const set< T >& s )
	{
		out << "size: " << s.size() << endl << "{" << endl;
		bool first = true;
		for( T element : s )
		{
			if( first )
			{
				out << element;
				first = false;
			}
			else
			{
				out << endl << ", " << endl << element;
			}
		}
		out << "}" << endl;
		
		return out;
	}
	
// 	template<>
// 	ostream& operator<<( ostream& out, const vector< vec3 >& v );
// 	
// 	template<>
// 	ostream& operator<<( ostream& out, const vector< vec4 >& v );
	
	template<>
	ostream& operator<<( ostream& out, const vector< PointPtr >& v );
	
	ostream& operator<<( ostream& out, const QSize& size );
	
	ostream& operator<<( ostream& out, const QPoint& point );
	
	ostream& operator<<( ostream& out, const QRect& rect );

	class Binary
	{
	public:
		template<typename T>
		static void read(istream& in, T& data)
		{
			in.read( reinterpret_cast< char* >( &data ), sizeof( T ) );
		}

		template<typename T>
		static void write(ostream& out, const T& data)
		{
			out.write( reinterpret_cast< const char* >( &data ), sizeof( T ) );
		}
	};
}

#endif
