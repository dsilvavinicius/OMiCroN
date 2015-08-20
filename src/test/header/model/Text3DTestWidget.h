#ifndef TEXT_3D_TEST_WIDGET
#define TEXT_3D_TEST_WIDGET

#include <utils/qtfreecamerawidget.hpp>
#include "TextEffect.h"

using namespace Tucano;

namespace model
{
	namespace test
	{
		class Text3DTestWidget
		: public QtFreecameraWidget
		{
			Q_OBJECT
			
		public:
			Text3DTestWidget( QWidget *parent = 0 );
			
			void paintGL();
		};
	}
}

#endif