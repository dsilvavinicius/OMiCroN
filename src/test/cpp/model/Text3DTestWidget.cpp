#include "Text3DTestWidget.h"

#include <QGuiApplication>

namespace model
{
	namespace test
	{
		Text3DTestWidget::Text3DTestWidget( QWidget *parent )
		: QtFreecameraWidget( parent )
		{}
		
		void Text3DTestWidget::paintGL()
		{
			Vector4f viewPort = camera->getViewport();
			glViewport( viewPort[ 0 ], viewPort[ 1 ], viewPort[ 2 ], viewPort[ 3 ] );
			
			TextEffect effect( QGuiApplication::applicationDirPath().toStdString() + "/../shaders/" );
			
			effect.initialize( QGuiApplication::applicationDirPath().toStdString() + "/../shaders/Inconsolata.otf" );
			
			/* White background */
			glClearColor( 1, 1, 1, 1 );
			glClear( GL_COLOR_BUFFER_BIT );

			/* Enable blending, necessary for our alpha texture */
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			Vector4f black( 0.f, 0.f, 0.f, 1.f );

			/* Set color to black */
			effect.setColor( black );
			
			/* Effects of alignment */
			effect.render( "Origin", Vector4f( 0.f, 0.f, 0.f, 1.f ), *camera );
			effect.render( "XZ=20", Vector4f( 20.f, 0.f, 20.f, 1.f ), *camera );
			effect.render( "YZ=20", Vector4f( 0.f, 20.f, 20.f, 1.f ), *camera );
			effect.render( "XYZ=20", Vector4f( 20.f, 20.f, 20.f, 1.f ), *camera );
			effect.render( "Z=80", Vector4f( 0.f, 0.f, 80.f, 1.f ), *camera );
			effect.render( "Z=-80", Vector4f( 0.f, 0.f, -80.f, 1.f ), *camera );
			effect.render( "X=80", Vector4f( 80.f, 0.f, 0.f, 1.f ), *camera );
			effect.render( "Y=80", Vector4f( 0.f, 80.f, 0.f, 1.f ), *camera );
			
			stringstream ss;
			ss << "Camera pos: " << endl << camera->getTranslationMatrix();
			Vector4f viewport = camera->getViewport();
			effect.render( ss.str(), TextEffect::MEDIUM, -0.95f, -0.75f, 2.0 / viewport[ 2 ], 2.0 / viewport[ 3 ] );
		}
	}
}