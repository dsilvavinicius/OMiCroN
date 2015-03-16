#include <QtGui/QGuiApplication>
#include <QApplication>
#include "MainWindow.h"
#include "PointRendererWindow.h"

using namespace ui;

int main(int argc, char **argv)
{
	// Use this code to run with PointRendererWindow ( Qt3D )
    /*QGuiApplication app(argc, argv);
	
	QSurfaceFormat format;
	format.setVersion( 4, 3 );
	format.setRenderableType( QSurfaceFormat::OpenGL );
	format.setSwapBehavior( QSurfaceFormat::DoubleBuffer );
	format.setSamples( 16 );

	PointRendererWindow window(format);
	window.resize(640, 480);
	window.show();
	
	return app.exec();*/
	
	// Use this code to run with PointRendererWidget ( Legacy Qt and Tucano )
	QApplication app( argc, argv );

	MainWindow window;

    window.show();

    window.initialize();

    return app.exec();
}