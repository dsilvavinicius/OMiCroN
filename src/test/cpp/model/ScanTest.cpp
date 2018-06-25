#include <gtest/gtest.h>
#include <iostream>

#include <QApplication>

#include "Scan.h"
#include "ScanQGLView.h"

using namespace std;

extern "C" int g_argc;
extern "C" char** g_argv;

namespace omicron
{
	namespace test
	{
        class ScanTest : public ::testing::Test {};

		TEST_F( ScanTest, Scan )
		{
			QGuiApplication app( g_argc, g_argv );
	
			QSurfaceFormat format;
			format.setVersion( 4, 3 );
			format.setRenderableType( QSurfaceFormat::OpenGL );
			format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
			format.setSamples( 16 );

			vector< unsigned int  > values( 3000 );
			for( int i = 0; i < values.size(); ++i )
			{
				values[ i ] = i;
			}
			
			ScanQGLView window( values, format );
			window.resize(640, 480);
			window.show();

			app.exec();
			
			vector< unsigned int > scanResults = window.m_values;
			unsigned int reduction = window.m_reduction;
			
			unsigned int currentSum = 0;
			for( int i = 0; i < scanResults.size(); ++i )
			{
				ASSERT_EQ( scanResults[ i ], currentSum );
				currentSum += i;
			}
			
			ASSERT_EQ( reduction, currentSum );
		}
	}
}