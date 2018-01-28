#ifndef FLAGS_ENUM
#define FLAGS_ENUM


#define MAKE_FLAGS_ENUM_OPS(Name) \
inline Name \
operator|(Name a, Name b) \
{ \
  return static_cast<Name>( static_cast<int>(a) | static_cast<int>(b) ); \
} \
 \
inline Name \
operator|=(Name& a, Name b) \
{ \
  return (a = a | b); \
}


#endif