#include "PlyPointReader.h"

#include <stdexcept>

namespace util
{
	int PlyPointReader::vertexCB(p_ply_argument argument)
	{
		long propFlag;
		void *rawPoints;
		ply_get_argument_user_data(argument, &rawPoints, &propFlag);
		PointVector< float, vec3 >* points = (PointVector< float, vec3 >*) rawPoints;
		
		float value = ply_get_argument_value(argument);
		//cout << "Prop value: " << value << endl;
		
		unsigned int index = propFlag & 0x7;		
		switch (index)
		{
			case 0:
			{
				auto point = make_shared< Point< float, vec3 > >(vec3(0.f, 0.f, 0.f), vec3(value, 0.f, 0.f));
				points->push_back(point);
				break;
			}
			case 1: case 2:
			{
				PointPtr< float, vec3 > point = points->back();
				(*point->getPos())[index] = value;
				break;
			}
			case 3: case 4: case 5:
			{
				PointPtr< float, vec3 > point = points->back();
				(*point->getColor())[index - 3] = (float) value / 255;
				break;
			}
		}
		
		return 1;
	}
}