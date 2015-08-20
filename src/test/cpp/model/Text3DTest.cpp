#include <gtest/gtest.h>

#include "Text3DTestWidget.h"
#include <QApplication>

extern "C" int g_argc;
extern "C" char** g_argv;

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
			QApplication app( g_argc, g_argv );
			
			Text3DTestWidget widget;
			widget.resize( 640, 480 );
			widget.show();

			app.exec();
		}
	}
}