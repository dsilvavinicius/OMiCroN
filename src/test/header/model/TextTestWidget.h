#ifndef TEXT_TEST_WIDGET
#define TEXT_TEST_WIDGET

#include <utils/qtplainwidget.hpp>
#include "TextEffect.h"

namespace model
{
	namespace test
	{
		class TextTestWidget
		: public QtPlainWidget
		{
			Q_OBJECT
			
		public:
			TextTestWidget( QWidget *parent = 0 );
			
			void paintGL();
		};
	}
}

#endif