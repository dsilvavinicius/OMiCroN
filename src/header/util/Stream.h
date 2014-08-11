#ifndef STREAM_H
#define STREAM_H

#include <vector>
#include <iostream>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

namespace model
{
	template <typename T>
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
	}
}

#endif