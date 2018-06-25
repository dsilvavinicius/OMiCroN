#ifndef OGL_UTILS_H
#define OGL_UTILS_H

#include <iostream>
#include <stdexcept>
#include <GL/glew.h>
#include <sstream>

using namespace std;

namespace omicron::renderer
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
		
		static void checkFramebuffer()
		{
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				std::string status_string;

				switch (status)
				{
					case GL_FRAMEBUFFER_COMPLETE:
						status_string = "GL_FRAMEBUFFER_COMPLETE";
						break;

					case GL_FRAMEBUFFER_UNDEFINED:
						status_string = "GL_FRAMEBUFFER_UNDEFINED";
						break;

					case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
						status_string = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
						break;

					case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
						status_string = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
						break;

					case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
						status_string = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
						break;

					case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
						status_string = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
						break;

					case GL_FRAMEBUFFER_UNSUPPORTED:
						status_string = "GL_FRAMEBUFFER_UNSUPPORTED";
						break;

					case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
						status_string = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
						break;

					case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
						status_string = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
						break;

					default:
						status_string = "UNKNOWN";
				}
				
				stringstream ss; ss << "Framebuffer status: " << status_string << endl << endl;
				
				throw runtime_error( ss.str() );
			}
		}
	};
}

#endif
