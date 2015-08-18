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
	
	template< typename T >
	ostream& operator<<( ostream& out, const vector< T* >& v )
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
	
	template<>
	ostream& operator<<( ostream& out, const vector< vec3 >& v );
	
	template<>
	ostream& operator<<( ostream& out, const vector< vec4 >& v );
	
	template<>
	ostream& operator<<( ostream& out, const vector< PointPtr >& v );
	
	template<>
	ostream& operator<<( ostream& out, const vector< ExtendedPointPtr >& v );
	
	ostream& operator<<( ostream& out, const QSize& size );
	
	ostream& operator<<( ostream& out, const QPoint& point );
	
	ostream& operator<<( ostream& out, const QRect& rect );
	
	ostream& operator<<( ostream& out, const vec3& vec );
	
	ostream& operator<<( ostream& out, const vec4& vec );
}

#endif