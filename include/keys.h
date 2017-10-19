#ifndef KEY_LISTENER_H_DEF
#define KEY_LISTENER_H_DEF

#include <SDL2/SDL.h>

#include "types.h"


struct Key
{
  b32 down_event;
  b32 up_event;
  b32 pressed;

  u16 modifier;
};


enum AsciiKeyConstants
{
  KEY_SPACE = 0x20,
  KEY_ESCAPE = 0x1b,
  KEY_ENTER = 0x0d,
  KEY_BACKSPACE = 0x08,
  KEY_DELETE = 0x7f
};


struct Keys
{
  Key ascii[127];

  Key right_arrow;
  Key left_arrow;
  Key up_arrow;
  Key down_arrow;
};


void
reset_key_events(Keys *keys);

void
process_key_event(Keys *keys, SDL_KeyboardEvent *event);

#endif