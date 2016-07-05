#ifndef OGL_UTILS_H
#define OGL_UTILS_H

#include <iostream>
#include <stdexcept>
#include <GL/glu.h>

using namespace std;

namespace util
{
	class OglUtils
	{
	public:
		static void checkOglErrors()
		{
			GLenum err = GL_NO_ERROR;
			bool hasErrors = false;
			stringstream ss;
			while( ( err = glGetError() ) != GL_NO_ERROR )
			{
				hasErrors = true;
				ss  << "OpenGL error 0x" << hex << err << ": " << gluErrorString( err ) << endl << endl;
			}
			
			if( hasErrors )
			{
				throw runtime_error( ss.str() );
			}
		}
	};
}

#endif