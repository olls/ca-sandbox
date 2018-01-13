#ifndef COLOUR_H_DEF
#define COLOUR_H_DEF

#include "vectors.h"


vec4
lighten_colour(vec4 colour, r32 amount = 0.2);


vec4
darken_colour(vec4 colour, r32 amount = 0.2);


#endif