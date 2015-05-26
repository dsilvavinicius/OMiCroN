#ifndef STREAM_H
#define STREAM_H

#include <vector>
#include <set>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <Eigen/Geometry>
#include "Point.h"
#include <ExtendedPoint.h>

using namespace std;
using namespace glm;
using namespace Eigen;

namespace model
{
	template< typename T >
	ostream& operator<<( ostream& out, const vector< T >& v )
	{
		out << "{" << endl;
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
	
	template< typename T >
	ostream& operator<<( ostream& out, const vector< T* >& v )
	{
		out << "{" << endl;
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
		out << "{" << endl;
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
	
	template< typename Float, typename Vec3 >
	ostream& operator<<( ostream& out, const vector< PointPtr< Float, Vec3> >& v )
	{
		out << "size: " << v.size() << endl << "{" << endl;
		bool first = true;
		for( PointPtr< Float, Vec3 > element : v )
		{
			if( first )
			{
				out << *element;
				first = false;
			}
			else
			{
				out << ", " << endl << *element;
			}
		}
		out << "}";
		
		return out;
	}
	
	template< typename Float, typename Vec3 >
	ostream& operator<<( ostream& out, const vector< ExtendedPointPtr< Float, Vec3> >& v )
	{
		out << "size: " << v.size() << endl << "{" << endl;
		bool first = true;
		for( ExtendedPointPtr< Float, Vec3 > element : v )
		{
			if( first )
			{
				out << *element;
				first = false;
			}
			else
			{
				out << ", " << endl << *element;
			}
		}
		out << "}";
		
		return out;
	}
	
	template<>
	ostream& operator<<( ostream& out, const vector< vec3 >& v );
	
	ostream& operator<<( ostream& out, const QSize& size );
	
	ostream& operator<<( ostream& out, const QPoint& point );
	
	ostream& operator<<( ostream& out, const QRect& rect );
	
	ostream& operator<<( ostream& out, const vec3& vec );
}

#endif