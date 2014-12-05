#include <QtGui/QGuiApplication>
#include "PointRendererWindow.h"

using namespace ui;

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
	
	QSurfaceFormat format;
	format.setSamples(16);

	PointRendererWindow window(format);
	window.resize(640, 480);
	window.show();

    return app.exec();
}
