#ifndef PLY_POINT_READER_H
#define PLY_POINT_READER_H

#include <string>
#include "rply/rply.h"
#include "Point.h"

using namespace std;
using namespace model;

namespace util
{	
	/** Reader for a point .ply file. */
	class PlyPointReader 
	{
	public:
		enum Precision
		{
			DOUBLE = 0x8,
			SINGLE = 0x0
		};
		
		/** Reads a .ply file, returning a vector of points.
		 * @param precision specifies the desired precision for read points. It must be agreed with the template
		 * parameters of this method. */
		template< typename Float, typename Vec3 >
		static PointVector< Float, Vec3 > read(const string& fileName, Precision precision);
		
	private:
		PlyPointReader();
		
		/** Sets the RPly callback for vertex data, given a property index and a precision flag. */
		template< typename Float, typename Vec3 >
		static void setVertexCB(p_ply ply, string propName, const unsigned int propIndex,
								PointVector< Float, Vec3 >* points, Precision precision);
		
		static int vertexCB(p_ply_argument argument);
	};
	
	template< typename Float, typename Vec3 >
	void PlyPointReader::setVertexCB(p_ply ply, string propName, const unsigned int propIndex,
									 PointVector< Float, Vec3 >* points, Precision precision)
	{
		unsigned int propFlag = propIndex | (precision << 3);
		ply_set_read_cb(ply, "vertex", propName.c_str(), vertexCB, points, propFlag);
	}
	
	template< typename Float, typename Vec3 >
	PointVector< Float, Vec3 > PlyPointReader::read(const string& fileName,
		PlyPointReader::Precision precision)
	{
		p_ply ply = ply_open(fileName.c_str(), NULL, 0, NULL);
		if (!ply)
		{
			throw runtime_error("Cannot open .ply point file");
		}
		if (!ply_read_header(ply))
		{
			throw runtime_error("Cannot read point file header.");
		}
		
		PointVector< Float, Vec3 > points;
		setVertexCB< Float, Vec3 >(ply, "x", 0, &points, precision);
		setVertexCB< Float, Vec3 >(ply, "y", 1, &points, precision);
		setVertexCB< Float, Vec3 >(ply, "z", 2, &points, precision);
		setVertexCB< Float, Vec3 >(ply, "red", 3, &points, precision);
		setVertexCB< Float, Vec3 >(ply, "green", 4, &points, precision);
		setVertexCB< Float, Vec3 >(ply, "blue", 5, &points, precision);
		
		if (!ply_read(ply))
		{
			throw runtime_error("Problem while reading points.");
		}
		
		return points;
	}
}

#endif