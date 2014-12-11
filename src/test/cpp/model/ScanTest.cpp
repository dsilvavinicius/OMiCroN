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
			QGuiApplication app( g_argc, g_argv );
	
			QSurfaceFormat format;
			format.setVersion( 4, 3 );
			format.setRenderableType( QSurfaceFormat::OpenGL );
			format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
			format.setSamples( 16 );

			ScanQGLView window( format );
			window.resize(640, 480);
			window.show();

			app.exec();
			
			vector< unsigned int > scanResults = window.m_scanResults;
			
			cout << "Final Scan." << endl;
			for( unsigned int value : scanResults )
			{
				cout << value << endl;
			}
			cout << endl;
		}
	}
}