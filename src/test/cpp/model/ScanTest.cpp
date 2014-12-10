#include <gtest/gtest.h>
#include <iostream>

#include <QApplication>

#include "Scan.h"
#include "ScanQGLView.h"

using namespace std;

extern "C" int g_argc;
extern "C" char** g_argv;

namespace model
{
	namespace test
	{
        class ScanTest : public ::testing::Test {};

		TEST_F( ScanTest, Scan )
		{
			vector< unsigned int > values( 3000, 1 );
			
			QGuiApplication app( g_argc, g_argv );
	
			QSurfaceFormat format;
			format.setVersion( 4, 3 );
			format.setRenderableType( QSurfaceFormat::OpenGL );
			format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
			format.setSamples( 16 );

			ScanQGLView window( values, format );
			window.resize(640, 480);
			window.show();

			app.exec();
			
			shared_ptr< vector< unsigned int > > scanResults = window.m_scanResults;
			
			for( unsigned int value : *scanResults )
			{
				cout << value << ", " << endl;
			}
		}
	}
}