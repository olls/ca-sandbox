#include <GL/glew.h>

#include "types.h"
#include "print.h"
#include "opengl-util.h"


/// @file
/// @brief Utility functions for debugging OpenGL


/// Convert an OpenGL error code into a string, and print it.
void
opengl_print_error(GLenum error_code, const char *file, u32 line)
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
  print("OpenGL error: %s at %s:%d\n", error, file, line);
}


/// Get all errors from OpenGL, and print them.
b32
_opengl_print_errors(const char *file, u32 line)
{
  b32 success = true;

  GLenum error = glGetError();
  while (error != GL_NO_ERROR)
  {
    success &= false;
    opengl_print_error(error, file, line);

    error = glGetError();
  }

  return success;
}


/// @brief A callback function used by OpenGL for debug output.
///
/// This is enabled in init_sdl() with DEBUG defined.
///
void
opengl_debug_output_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *user_param)
{
  const char *source_str = 0;
  const char *type_str = 0;
  const char *severity_str = 0;

  switch (source)
  {
      case GL_DEBUG_SOURCE_API:
      {
        source_str = "GL_DEBUG_SOURCE_API";
      } break;
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      {
        source_str = "GL_DEBUG_SOURCE_WINDOW_SYSTEM";
      } break;
      case GL_DEBUG_SOURCE_SHADER_COMPILER:
      {
        source_str = "GL_DEBUG_SOURCE_SHADER_COMPILER";
      } break;
      case GL_DEBUG_SOURCE_THIRD_PARTY:
      {
        source_str = "GL_DEBUG_SOURCE_THIRD_PARTY";
      } break;
      case GL_DEBUG_SOURCE_APPLICATION:
      {
        source_str = "GL_DEBUG_SOURCE_APPLICATION";
      } break;
      case GL_DEBUG_SOURCE_OTHER:
      {
        source_str = "GL_DEBUG_SOURCE_OTHER";
      } break;
  }

  switch (type)
  {
      case GL_DEBUG_TYPE_ERROR:
      {
        type_str = "GL_DEBUG_TYPE_ERROR";
      } break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      {
        type_str = "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR";
      } break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      {
        type_str = "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR";
      } break;
      case GL_DEBUG_TYPE_PORTABILITY:
      {
        type_str = "GL_DEBUG_TYPE_PORTABILITY";
      } break;
      case GL_DEBUG_TYPE_PERFORMANCE:
      {
        type_str = "GL_DEBUG_TYPE_PERFORMANCE";
      } break;
      case GL_DEBUG_TYPE_MARKER:
      {
        type_str = "GL_DEBUG_TYPE_MARKER";
      } break;
      case GL_DEBUG_TYPE_PUSH_GROUP:
      {
        type_str = "GL_DEBUG_TYPE_PUSH_GROUP";
      } break;
      case GL_DEBUG_TYPE_POP_GROUP:
      {
        type_str = "GL_DEBUG_TYPE_POP_GROUP";
      } break;
      case GL_DEBUG_TYPE_OTHER:
      {
        type_str = "GL_DEBUG_TYPE_OTHER";
      } break;
  }

  switch (severity)
  {
      case GL_DEBUG_SEVERITY_HIGH:
      {
        severity_str = "GL_DEBUG_SEVERITY_HIGH";
      } break;
      case GL_DEBUG_SEVERITY_MEDIUM:
      {
        severity_str = "GL_DEBUG_SEVERITY_MEDIUM";
      } break;
      case GL_DEBUG_SEVERITY_LOW:
      {
        severity_str = "GL_DEBUG_SEVERITY_LOW";
      } break;
      case GL_DEBUG_SEVERITY_NOTIFICATION:
      {
        severity_str = "GL_DEBUG_SEVERITY_NOTIFICATION";
      } break;
  }

  print("Source: %s, Type: %s, ID: %d, Severity: %s, \"%.*s\"\n", source_str, type_str, id, severity_str, length, message);
}


/// Function alias for alternate OpenGL versions.
void
opengl_debug_output_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, void *user_param)
{
  opengl_debug_output_callback(source, type, id, severity, length, message, (const void *)user_param);
}