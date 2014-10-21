#ifndef STREAM_H
#define STREAM_H

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <QPoint>
#include <QRect>
#include <QSize>

using namespace std;
using namespace glm;

namespace model
{
	/*template <typename T>
	ostream& operator<<(ostream& out, const vector<T>& v)
	{
		out << "{";
		bool first = true;
		for (T element : v)
		{
			if (first)
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
	}*/
	
	template <typename T>
	ostream& operator<<(ostream& out, const vector<T>& v)
	{
		out << "{";
		bool first = true;
		for (T element : v)
		{
			if (first)
			{
				out << glm::to_string(element);
				first = false;
			}
			else
			{
				out << ", " << glm::to_string(element);
			}
		}
		out << "}";
		
		return out;
	}
	
	ostream& operator<<( ostream& out, const QSize& size );
	
	ostream& operator<<( ostream& out, const QPoint& point );
	
	ostream& operator<<( ostream& out, const QRect& rect );
}

#endif