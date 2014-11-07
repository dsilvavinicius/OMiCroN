#ifndef STREAM_H
#define STREAM_H

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <QPoint>
#include <QRect>
#include <QSize>
#include "Point.h"
#include <ExtendedPoint.h>

using namespace std;
using namespace glm;

namespace model
{
	template< typename T >
	ostream& operator<<( ostream& out, const vector< T >& v )
	{
		out << "{";
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
				out << ", " << element;
			}
		}
		out << "}";
		
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
	}
	
	template<>
	ostream& operator<<( ostream& out, const vector< vec3 >& v );
	
	ostream& operator<<( ostream& out, const QSize& size );
	
	ostream& operator<<( ostream& out, const QPoint& point );
	
	ostream& operator<<( ostream& out, const QRect& rect );
}

#endif