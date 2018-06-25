#ifndef TEXT_EFFECT
#define TEXT_EFFECT

#include <effect.hpp>
#include <texture.hpp>
#include <mesh.hpp>
#include <camera.hpp>

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "omicron/basic/stream.h"

namespace omicron::renderer
{
    using namespace Tucano;
    using namespace Eigen;
    
	/**
	* The atlas struct holds a texture that contains the visible US-ASCII characters
	* of a certain font rendered with a certain character height.
	* It also contains an array that contains all the information necessary to
	* generate the appropriate vertex and texture coordinates for each character.
	*
	* After the constructor is run, you don't need to use any FreeType functions anymore.
	*/
	struct Atlas {
		Texture tex;	// texture object

		unsigned int w;	// width of texture in pixels
		unsigned int h;	// height of texture in pixels
		unsigned int lineh; // height of one line of text = height of the tallest char in the map.
		
		struct {
			float ax;	// advance.x
			float ay;	// advance.y

			float bw;	// bitmap.width;
			float bh;	// bitmap.height;

			float bl;	// bitmap_left;
			float bt;	// bitmap_top;

			float tx;	// x offset of glyph in texture coordinates
			float ty;	// y offset of glyph in texture coordinates
		} c[128];		// character information
		
		Atlas( FT_Face face, int height )
		{
			// Maximum texture width
			int MAXWIDTH = 1024;
			
			FT_Set_Pixel_Sizes( face, 0, height );
			FT_GlyphSlot g = face->glyph;

			unsigned int roww = 0;
			unsigned int rowh = 0;
			w = 0;
			h = 0;
			lineh = 0;
			
			memset( c, 0, sizeof c );

			/* Find minimum size for a texture holding all visible ASCII characters */
			for( int i = 32; i < 128; i++ )
			{
				if( FT_Load_Char( face, i, FT_LOAD_RENDER ) )
				{
					stringstream ss;
					ss << "Loading character " << i << " failed!";
					throw logic_error( ss.str() );
				}
				if( roww + g->bitmap.width + 1 >= MAXWIDTH )
				{
					w = std::max( w, roww );
					h += rowh;
					lineh = std::max( rowh, lineh );
					roww = 0;
					rowh = 0;
				}
				roww += g->bitmap.width + 1;
				rowh = std::max( rowh, ( unsigned int ) g->bitmap.rows );
			}

			w = std::max( w, roww );
			h += rowh;
			lineh = std::max( rowh, lineh );

			/* Create a texture that will be used to hold all ASCII glyphs */
			tex.create( GL_TEXTURE_2D, GL_ALPHA, w, h, GL_ALPHA, GL_UNSIGNED_BYTE );

			/* We require 1 byte alignment when uploading texture data */
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			/* Clamping to edges is important to prevent artifacts when scaling */
			/* Linear filtering usually looks best for text */
			tex.setTexParameters( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR );
			
			tex.bind();

			/* Paste all glyph bitmaps into the texture, remembering the offset */
			int ox = 0;
			int oy = 0;

			rowh = 0;

			for( int i = 32; i < 128; i++ )
			{
				if( FT_Load_Char( face, i, FT_LOAD_RENDER ) )
				{
					stringstream ss;
					ss << "Loading character " <<  i << " failed!";
					throw logic_error( ss.str() );
				}

				if( ox + g->bitmap.width + 1 >= MAXWIDTH )
				{
					oy += rowh;
					rowh = 0;
					ox = 0;
				}
				
				glTexSubImage2D( GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_ALPHA, GL_UNSIGNED_BYTE,
								 g->bitmap.buffer );
				c[i].ax = g->advance.x >> 6;
				c[i].ay = g->advance.y >> 6;

				c[i].bw = g->bitmap.width;
				c[i].bh = g->bitmap.rows;

				c[i].bl = g->bitmap_left;
				c[i].bt = g->bitmap_top;

				c[i].tx = ox / ( float ) w;
				c[i].ty = oy / ( float ) h;

				rowh = std::max( rowh, ( unsigned int ) g->bitmap.rows );
				ox += g->bitmap.width + 1;
			}

			cout << "Generated a " << w << " x " << h << " ( " << w * h / 1024 << " kb ) texture atlas" << endl;
		}
		
		~Atlas()
		{
			tex.unbind();
// 			tex.destroy();
		}
	};
	
	/** Effect for rendering text using character atlasses. Implementation of OpenGL Programming/Modern OpenGL Tutorial
	 * Text Rendering 02 available in
	 * https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_02 */
	class TextEffect
	: public Effect
	{
	public:
		enum ATLAS_SIZE
		{
			VERY_LARGE,
			LARGE,
			MEDIUM,
			SMALL,
			COUNT
		};
		
		TextEffect( string shadersDir = "shaders/" );
		
		~TextEffect();
		
		virtual void initialize();
		
		virtual void initialize( const string& fontFilename );
		
		/**
		* Render text in the position projCoords in normalized device coordinates using the currently loaded font and
		* currently set font size.
		* The pixel coordinates that the FreeType2 library uses are scaled by scale( first component for x and second for y).
		*/
		virtual void render( const string& text, const ATLAS_SIZE& size, const Vector3f& projCoords, const Vector2f& scale );
		
		/** Render text in the 3D coordinate, with a given Camera. */
		virtual void render( const string& text, const Vector4f& pos, Camera& cam );
		
		/** Sets the text color. Supports alpha channel. */
		void setColor( const Vector4f& color );
		
	protected:
		Atlas* m_atlases[ 4 ];
		
		Shader m_text2DShader;
	};
	
	inline TextEffect::TextEffect( string shadersDir )
	: Effect( shadersDir )
	{}
	
	inline TextEffect::~TextEffect()
	{
		for( int i = 0; i < ATLAS_SIZE::COUNT; ++i )
		{
			delete m_atlases[ i ];
		}
	}
	
	inline void TextEffect::initialize()
	{
		initialize( "shaders/Inconsolata.otf" );
	}
	
	inline void TextEffect::initialize( const string& fontFilename )
	{
		FT_Library ft;
		FT_Face face;
		
		/* Initialize the FreeType2 library */
		if( FT_Init_FreeType( &ft ) )
		{
			throw runtime_error( "Could not init freetype library." );
		}

		/* Load a font */
		if( FT_New_Face( ft, fontFilename.c_str(), 0, &face ) )
		{
			stringstream ss;
			ss << "Could not open font " << fontFilename;
			throw logic_error( ss.str() );
		}

		loadShader( m_text2DShader, "Text2D" );

		/* Create texture atlasses for several font sizes */
		m_atlases[ VERY_LARGE ] = new Atlas( face, 72 );
		m_atlases[ LARGE ] = new Atlas( face, 48 );
		m_atlases[ MEDIUM ] = new Atlas( face, 24 );
		m_atlases[ SMALL ] = new Atlas( face, 12 );
	}
	
	inline void TextEffect::render(  const string& text, const ATLAS_SIZE& size, const Vector3f& projCoords,
									 const Vector2f& scale )
	{
		const uint8_t *p;
		
		Atlas* a = m_atlases[ size ];

		vector< Vector4f > coords;
		vector< Vector2f > texCoords;
		vector< GLuint > indices;
		int c = 0;

		float x = projCoords[ 0 ];
		float y = projCoords[ 1 ];
		float z = projCoords[ 2 ];
		
		float sx = scale[ 0 ];
		float sy = scale[ 1 ];
		
		/* Loop through all characters */
		for( p = ( const uint8_t * )text.c_str(); *p; p++ )
		{
			/* Calculate the vertex and texture coordinates */
			float x2 = x + a->c[ *p ].bl * sx;
			float y2 = -y - a->c[ *p ].bt * sy;
			float w = a->c[ *p ].bw * sx;
			float h = a->c[ *p ].bh * sy;

			/* Advance the cursor to the start of the next character */
			x += a->c[ *p ].ax * sx;
			y += a->c[ *p ].ay * sy;

			/* Skip glyphs that have no pixels */
			if( !w || !h )
			{
				if( *( char* )p == '\n' )
				{
					x = projCoords[ 0 ];
					y -= a->lineh * sy;
				}
				continue;
			}

			indices.push_back( c++ );
			coords.push_back( Vector4f( x2, -y2, z, 1.f ) );
			texCoords.push_back( Vector2f( a->c[ *p ].tx, a->c[ *p ].ty ) );
			
			indices.push_back( c++ );
			coords.push_back( Vector4f( x2, -y2 - h, z, 1.f ) );
			texCoords.push_back( Vector2f( a->c[ *p ].tx, a->c[ *p ].ty + a->c[ *p ].bh / a->h ) );
			
			indices.push_back( c++ );
			coords.push_back( Vector4f( x2 + w, -y2, z, 1.f ) );
			texCoords.push_back( Vector2f( a->c[ *p ].tx + a->c[ *p ].bw / a->w, a->c[ *p ].ty ) );
			
			indices.push_back( c++ );
			coords.push_back( Vector4f( x2 + w, -y2, z, 1.f ) );
			texCoords.push_back( Vector2f( a->c[ *p ].tx + a->c[ *p ].bw / a->w,	a->c[ *p ].ty ) );
			
			indices.push_back( c++ );
			coords.push_back( Vector4f( x2, -y2 - h, z, 1.f ) );
			texCoords.push_back( Vector2f( a->c[ *p ].tx, a->c[ *p ].ty + a->c[ *p ].bh / a->h ) );
			
			indices.push_back( c++ );
			coords.push_back( Vector4f( x2 + w, -y2 - h, z, 1.f ) );
			texCoords.push_back( Vector2f( a->c[ *p ].tx + a->c[ *p ].bw / a->w, a->c[ *p ].ty + a->c[ *p ].bh / a->h ) );
			
		}
		
		m_text2DShader.bind();
		/* Use the texture containing the atlas */
		m_text2DShader.setUniform( "tex", a->tex.bind() );
		
		//
		/*GLuint buffer;
		glGenBuffers( 1, &buffer );
		glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, buffer );
		
		GLuint id;
		int bufferSize = coords.size() * sizeof( vec4 );
		glGenTransformFeedbacks( 1, &id );
		glBufferData( GL_TRANSFORM_FEEDBACK_BUFFER, bufferSize, NULL, GL_DYNAMIC_COPY );
		glBindBufferRange( GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer, 0, bufferSize );
		
		const char* vars[] = { "feedCoord" };
		glTransformFeedbackVaryings( m_textShader.getShaderProgram(), 1, vars, GL_INTERLEAVED_ATTRIBS );
		m_textShader.linkProgram();
		glBeginTransformFeedback( GL_TRIANGLES );*/
		//
		
		Mesh mesh;
		mesh.loadVertices( coords );
		mesh.loadTexCoords( texCoords );
		mesh.loadIndices( indices );
		mesh.setAttributeLocation( m_text2DShader );
		mesh.render();
		
		//
		/*vector< vec4 > feedbackData( coords.size() );
		glEndTransformFeedback();
		glFlush();
		glGetBufferSubData( GL_TRANSFORM_FEEDBACK_BUFFER, 0, bufferSize, feedbackData.data() );
		
		glInvalidateBufferData( buffer );
		glDeleteTransformFeedbacks( 1, &id );
		
		cout << "Feedback data:" << endl << endl << feedbackData << endl;*/
		//
		
		m_text2DShader.unbind();
		a->tex.unbind();
	}
	
	inline void TextEffect::render( const string& text, const Vector4f& pos, Camera& cam )
	{
		
		Matrix4f mvp = *cam.projectionMatrix() * cam.viewMatrix()->matrix();
		Vector4f viewport = cam.getViewport();
		
		Vector4f projPos = mvp * pos;
		projPos = projPos / projPos[ 3 ];
		
		if( projPos[ 2 ] >= -1.f && projPos[ 2 ] <= 1.f )
		{
			// The position is inside frustum.
			
			Vector2f scale( 2.0 / viewport[ 2 ], 2.0 / viewport[ 3 ] );
			
			render( text, MEDIUM, projPos.head< 3 >(), scale );
		}
	}
	
	inline void TextEffect::setColor( const Vector4f& color )
	{
		m_text2DShader.bind();
		m_text2DShader.setUniform( "color", color );
	}
}

#endif
