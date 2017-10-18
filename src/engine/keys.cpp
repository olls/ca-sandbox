#include "keys.h"


void
reset_key_events(Keys *keys)
{
  for (u32 i = 0; i < 127; ++i)
  {
    Key *key = keys->ascii + i;

    key->down_event = false;
    key->up_event = false;
  }

  keys->right_arrow.down_event = false;
  keys->right_arrow.up_event = false;

  keys->left_arrow.down_event = false;
  keys->left_arrow.up_event = false;

  keys->up_arrow.down_event = false;
  keys->up_arrow.up_event = false;

  keys->down_arrow.down_event = false;
  keys->down_arrow.up_event = false;
}


void
process_key_event(Keys *keys, SDL_KeyboardEvent *event)
{
  Key *key = 0;

  SDL_Keycode sdl_key = event->keysym.sym;

  if (sdl_key >= SDLK_UNKNOWN && sdl_key <= SDLK_DELETE)
  {
    // Within ASCII range

    key = keys->ascii + (sdl_key - SDLK_UNKNOWN);
  }
  else if (sdl_key == SDL_SCANCODE_RIGHT)
  {
    key = &keys->right_arrow;
  }
  else if (sdl_key == SDL_SCANCODE_LEFT)
  {
    key = &keys->left_arrow;
  }
  else if (sdl_key == SDL_SCANCODE_UP)
  {
    key = &keys->up_arrow;
  }
  else if (sdl_key == SDL_SCANCODE_DOWN)
  {
    key = &keys->down_arrow;
  }

  if (key != 0)
  {
    if (event->type == SDL_KEYDOWN)
    {
      key->down_event = true;
      key->up_event = false;
      key->pressed = true;
      key->modifier = event->keysym.mod;
    }
    else if (event->type == SDL_KEYUP)
    {
      key->down_event = false;
      key->up_event = true;
      key->pressed = false;
      key->modifier = KMOD_NONE;
    }
  }
}


b32
key_pressed_this_frame(Keys *keys, char key)
{
  return false;
}


b32
key_down_event_this_frame(Keys *keys, char key)
{
  return false;
}


b32
key_up_event_this_frame(Keys *keys, char key)
{
  return false;
}
