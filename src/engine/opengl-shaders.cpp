#include <GL/glew.h>
#include <stdlib.h>

#include "types.h"
#include "print.h"
#include "files.h"
#include "allocate.h"


b32
compile_shader(const char filename[], GLuint shader_type, GLuint *result)
{
  b32 success = true;

  File file;
  if (!open_file(filename, &file))
  {
    success = false;
  }
  else
  {
    *result = glCreateShader(shader_type);

    glShaderSource(*result, 1, &file.read_ptr, &file.size);
    glCompileShader(*result);

    GLint compile_success = 0;
    glGetShaderiv(*result, GL_COMPILE_STATUS, &compile_success);
    if (!compile_success)
    {
      success = false;

      GLint log_size = 0;
      glGetShaderiv(*result, GL_INFO_LOG_LENGTH, &log_size);

      GLchar *info_log = allocate(GLchar, log_size);
      glGetShaderInfoLog(*result, log_size, NULL, info_log);

      print("Shader compile error (%s): \"%s\"\n", filename, info_log);
      free(info_log);

      glDeleteShader(*result);
      *result = 0;
    }
  }

  return success;
}


b32
create_shader_program(const char *filenames[], GLenum types[], u32 n_shaders, GLuint *result)
{
  b32 success = true;

  *result = glCreateProgram();

  for (u32 shader_n = 0; shader_n < n_shaders; ++shader_n)
  {
    GLuint shader;
    b32 compile_success = compile_shader(filenames[shader_n], types[shader_n], &shader);
    if (compile_success == 0)
    {
      success = false;
    }
    else
    {
      glAttachShader(*result, shader);
    }
  }

  if (success)
  {
    glLinkProgram(*result);

    GLint link_success = 0;
    glGetProgramiv(*result, GL_LINK_STATUS, &link_success);
    if (link_success == GL_FALSE)
    {
      success = false;

      GLint log_size = 0;
      glGetProgramiv(*result, GL_INFO_LOG_LENGTH, &log_size);

      GLchar *info_log = allocate(GLchar, log_size);
      glGetShaderInfoLog(*result, log_size, NULL, info_log);

      print("Shader link error: \"%s\"\n", info_log);
      free(info_log);

      glDeleteShader(*result);
      *result = 0;
    }
  }

  return success;
}