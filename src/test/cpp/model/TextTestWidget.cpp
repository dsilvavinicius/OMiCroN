#include "TextTestWidget.h"
#include <QGuiApplication>

namespace model
{
	namespace test
	{
		TextTestWidget::TextTestWidget( QWidget *parent )
		: QtPlainWidget( parent )
		{}
		
		void TextTestWidget::paintGL()
		{
			cout << "=== Starting paint ===" << endl << endl;
			
			float sx = 2.0 / width();
			float sy = 2.0 / height();

			cout << "Initializing effect" << endl << endl;
			
			TextEffect effect( QGuiApplication::applicationDirPath().toStdString() + "/../shaders/" );
			
			effect.initialize( QGuiApplication::applicationDirPath().toStdString() + "/../shaders/Inconsolata.otf" );

			cout << "Effect initialized" << endl << endl;
			
			/* White background */
			glClearColor( 1, 1, 1, 1 );
			glClear( GL_COLOR_BUFFER_BIT );

			/* Enable blending, necessary for our alpha texture */
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

			Vector4f black( 0.f, 0.f, 0.f, 1.f );
			Vector4f red( 1.f, 0.f, 0.f, 1.f );
			Vector4f transparent_green( 0.f, 1.f, 0.f, 0.5f );

			/* Set color to black */
			effect.setColor( black );

			cout << "Rendering texts" << endl << endl;
			
			/* Effects of alignment */
			effect.render("The Quick Brown Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 50 * sy, sx, sy);
			effect.render("The Misaligned Fox Jumps Over The Lazy Dog", -1 + 8.5 * sx, 1 - 100.5 * sy, sx, sy);

			/* Scaling the texture versus changing the font size */
			effect.render("The Small Texture Scaled Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 175 * sy, sx * 0.5, sy * 0.5);
			effect.render("The Small Font Sized Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 200 * sy, sx, sy);
			effect.render("The Tiny Texture Scaled Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 235 * sy, sx * 0.25, sy * 0.25);
			effect.render("The Tiny Font Sized Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 250 * sy, sx, sy);

			/* Colors and transparency */
			effect.render("The Solid Black Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 430 * sy, sx, sy);

			effect.setColor( red );
			effect.render("The Solid Red Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 330 * sy, sx, sy);
			effect.render("The Solid Red Fox Jumps Over The Lazy Dog", -1 + 28 * sx, 1 - 450 * sy, sx, sy);

			effect.setColor( transparent_green );
			effect.render("The Transparent Green Fox Jumps Over The Lazy Dog", -1 + 8 * sx, 1 - 380 * sy, sx, sy);
			effect.render("The Transparent Green Fox Jumps Over The Lazy Dog", -1 + 18 * sx, 1 - 440 * sy, sx, sy);
			
			cout << "=== Ending paint ===" << endl << endl;
		}
	}
}