#include <Stream.h>

namespace model
{
	ostream& operator<<( ostream& out, const QPoint& point )
	{
		out << "( " << point.x() << ", " << point.y()  <<  " )";
		return out;
	}
	
	ostream& operator<<( ostream& out, const QSize& size )
	{
		out << "( " << size.width() << ", " << size.height()  <<  " )";
		return out;
	}
	
	ostream& operator<<(ostream& out, const QRect& rect)
	{
		out << "top left:" << rect.topLeft() << endl << "bottom right:" << rect.bottomRight()
			<< endl << "size:" << rect.size() << endl;
		return out;
	}
}