#include "header/model/Text3DTestWidget.h"

#include <QGuiApplication>

namespace omicron::test
{
    Text3DTestWidget::Text3DTestWidget( QWidget *parent )
    : QtFreecameraWidget( parent ),
    m_textEffect( "../shaders/" )
    {}
    
    void Text3DTestWidget::initializeGL()
    {
        QtFreecameraWidget::initializeGL();
        
        /* Enable blending, necessary for our alpha texture */
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        
        glCullFace( GL_BACK );
        glEnable( GL_CULL_FACE );
        
        glEnable( GL_DEPTH_TEST );
        
        m_textEffect.initialize( "../shaders/Inconsolata.otf" );
    }
    
    void Text3DTestWidget::paintGL()
    {
        Vector4f viewPort = camera->getViewport();
        glViewport( viewPort[ 0 ], viewPort[ 1 ], viewPort[ 2 ], viewPort[ 3 ] );
        
        /* White background */
        glClearColor( 1, 1, 1, 1 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        Vector4f black( 0.f, 0.f, 0.f, 1.f );

        /* Set color to black */
        m_textEffect.setColor( black );
        
        /* Effects of alignment */
        m_textEffect.render( "Origin", Vector4f( 0.f, 0.f, 0.f, 1.f ), *camera );
        m_textEffect.render( "XZ=20", Vector4f( 20.f, 0.f, 20.f, 1.f ), *camera );
        m_textEffect.render( "YZ=20", Vector4f( 0.f, 20.f, 20.f, 1.f ), *camera );
        m_textEffect.render( "XYZ=20", Vector4f( 20.f, 20.f, 20.f, 1.f ), *camera );
        m_textEffect.render( "Z=80", Vector4f( 0.f, 0.f, 80.f, 1.f ), *camera );
        m_textEffect.render( "Z=-80", Vector4f( 0.f, 0.f, -80.f, 1.f ), *camera );
        m_textEffect.render( "X=80", Vector4f( 80.f, 0.f, 0.f, 1.f ), *camera );
        m_textEffect.render( "Y=80", Vector4f( 0.f, 80.f, 0.f, 1.f ), *camera );
        
        stringstream ss;
        ss << "Camera pos: " << endl << camera->getTranslationMatrix();
        Vector4f viewport = camera->getViewport();
        m_textEffect.render( ss.str(), TextEffect::SMALL, Vector3f( -0.95f, -0.75f, 0.f ),
                                Vector2f( 2.0 / viewport[ 2 ], 2.0 / viewport[ 3 ] ) );
    }
}
