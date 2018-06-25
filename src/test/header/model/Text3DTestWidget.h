#ifndef TEXT_3D_TEST_WIDGET
#define TEXT_3D_TEST_WIDGET

#include <utils/qtfreecamerawidget.hpp>
#include "omicron/hierarchy/text_effect.h"

namespace omicron::test
{
    using namespace Tucano;
    using namespace omicron::hierarchy;
    
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
