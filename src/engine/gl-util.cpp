#include <cstdio>
#include <GL/glew.h>

#include "gl-util.h"
#include "types.h"


void
gl_print_error(GLenum error_code, const char *file, u32 line)
{
  const char *error;
  switch (error_code)
  {
    case (GL_INVALID_ENUM):
    {
      error = "GL_INVALID_ENUM";
    } break;
    case (GL_INVALID_VALUE):
    {
      error = "GL_INVALID_VALUE";
    } break;
    case (GL_INVALID_OPERATION):
    {
      error = "GL_INVALID_OPERATION";
    } break;
    case (GL_STACK_OVERFLOW):
    {
      error = "GL_STACK_OVERFLOW";
    } break;
    case (GL_STACK_UNDERFLOW):
    {
      error = "GL_STACK_UNDERFLOW";
    } break;
    case (GL_OUT_OF_MEMORY):
    {
      error = "GL_OUT_OF_MEMORY";
    } break;
    case (GL_INVALID_FRAMEBUFFER_OPERATION):
    {
      error = "GL_INVALID_FRAMEBUFFER_OPERATION";
    } break;
    case (GL_TABLE_TOO_LARGE):
    {
      error = "GL_TABLE_TOO_LARGE";
    }
  }
  printf("OpenGL error: %s at %s:%d\n", error, file, line);
}


b32
_gl_print_errors(const char *file, u32 line)
{
  b32 success = true;

  GLenum error = glGetError();
  while (error != GL_NO_ERROR)
  {
    success &= false;
    gl_print_error(error, file, line);

    error = glGetError();
  }

  return success;
}
