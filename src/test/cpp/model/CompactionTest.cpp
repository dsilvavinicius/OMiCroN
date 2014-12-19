#include <gtest/gtest.h>
#include <iostream>

#include <QApplication>
#include <glm/glm.hpp>

#include "CompactionQGLView.h"
#include "Stream.h"

using namespace std;
using namespace glm;

extern "C" int g_argc;
extern "C" char** g_argv;

namespace model
{
	namespace test
	{
        class CompactionTest : public ::testing::Test {};

		TEST_F( CompactionTest, Compaction )
		{
			QGuiApplication app( g_argc, g_argv );
	
			QSurfaceFormat format;
			format.setVersion( 4, 3 );
			format.setRenderableType( QSurfaceFormat::OpenGL );
			format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
			format.setSamples( 16 );

			unsigned int nElements = 3000;
			vector< unsigned int  > flags( nElements );
			vector< vec3 > pos( nElements );
			vector< vec3 > attrib0( nElements );
			
			for( int i = 0; i < nElements; ++i )
			{
				flags[ i ] = i % 2;
				pos[ i ] = vec3( i, i, i );
				attrib0[ i ] = vec3( i + nElements, i + nElements, i + nElements );
			}
			
			CompactionQGLView window( flags, pos, attrib0, format );
			window.resize(640, 480);
			window.show();

			app.exec();
			
			pos = window.m_compactedPos;
			attrib0 = window.m_compactedAttrib0;
			
			ASSERT_EQ( pos.size(), nElements * 0.5 );
			ASSERT_EQ( attrib0.size(), nElements * 0.5 );
			
			float expected = 1.;
			for( int i = 0; i < pos.size(); ++i, expected += 2 )
			{
				vec3 expectedVec( expected, expected, expected );
				ASSERT_EQ( pos[ i ], expectedVec );
				
				expectedVec = vec3( expected + nElements, expected + nElements, expected + nElements );
				ASSERT_EQ( attrib0[ i ], expectedVec );
			}
		}
	}
}