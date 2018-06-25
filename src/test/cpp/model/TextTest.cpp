#include <gtest/gtest.h>

#include "header/model/TextTestWidget.h"
#include <QApplication>

namespace omicron::test
{
    class TextTest
    : public ::testing::Test
    {
    protected:
        void SetUp(){}
    };
    
    /** Renders text using TextEffect. Just opens a window displaying the texts rendered using OpenGL. When the
        * window is closed, the test finishes. */
    TEST_F( TextTest, Rendering )
    {
        TextTestWidget widget;
        widget.resize( 640, 480 );
        widget.show();

        QApplication::exec();
    }
}
