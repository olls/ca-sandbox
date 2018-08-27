#include "loader.h"

#include "engine/engine.h"

#include "ca-sandbox/ca-sandbox.h"

#include <stdio.h>
#include <dlfcn.h>


Export_VTable *
load_library(void **library_handle, const char *library_path)
{
  Export_VTable *result = 0;

  if (*library_handle != 0)
  {
    dlclose(*library_handle);
    *library_handle = 0;
  }

  *library_handle = dlopen(library_path, RTLD_NOW);
  if (!*library_handle)
  {
    printf("%s\n", dlerror());
  }
  else
  {
    result = (Export_VTable *)dlsym(*library_handle, "exports");

    char *error = dlerror();
    if (error != 0)
    {
      result = 0;
      printf("%s\n", error);
    }
  }

  return result;
}


int main(int argc, char const *argv[])
{
  s32 result = 0;

  void *library_handle = 0;
  Export_VTable *ca_sandbox_exports;

  const char *library_path = "lib" APP_NAME ".so";

  Engine engine;
  b32 engine_started = start_engine(argc, argv, APP_NAME " - Version " APP_VERSION, &engine);

  CA_SandboxState *state = 0;
  LoaderReturnStatus status = {.reload = true, .success = true};

  b32 quit = !engine_started;
  while (!quit)
  {
    if (!status.success)
    {
      // Quit with error
      result = 1;
      quit = true;
    }
    else
    {
      if (!status.reload)
      {
        // Quit successfully
        quit = true;
      }
      else
      {
        printf("\nReloading " APP_NAME " library\n");
        ca_sandbox_exports = load_library(&library_handle, library_path);
        printf("\n");

        if (ca_sandbox_exports == 0)
        {
          quit = true;
        }
      }
    }

    if (!quit)
    {
      status = ca_sandbox_exports->main_loop(argc, argv, &engine, &state);
    }
  }

  stop_engine(&engine);

  return result;
}