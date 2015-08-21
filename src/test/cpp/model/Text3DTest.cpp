#include <gtest/gtest.h>

#include "Text3DTestWidget.h"
#include <QApplication>

namespace model
{
	namespace test
	{
        class Text3DTest
        : public ::testing::Test
		{
		protected:
			void SetUp(){}
		};
		
		/** Renders text using TextEffect. Just opens a window displaying the texts rendered using OpenGL. When the
		 * window is closed, the test finishes. */
		TEST_F( Text3DTest, Rendering )
		{
			Text3DTestWidget widget;
			widget.resize( 640, 480 );
			widget.show();

			QApplication::exec();
		}
	}
}