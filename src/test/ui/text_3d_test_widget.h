#ifndef TEXT_3D_TEST_WIDGET
#define TEXT_3D_TEST_WIDGET

#include <utils/qtfreecamerawidget.hpp>
#include "omicron/renderer/text_effect.h"

namespace omicron::test::ui
{
    using namespace Tucano;
    using namespace omicron::renderer;
    
    class Text3DTestWidget
    : public QtFreecameraWidget
    {
        Q_OBJECT
        
    public:
        Text3DTestWidget( QWidget *parent = 0 );
        
        void initializeGL();
        
        void paintGL();
    
    private:
        TextEffect m_textEffect;
    };
}

#endif
