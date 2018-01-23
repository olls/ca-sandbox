#ifndef PRINT_H_DEF
#define PRINT_H_DEF

/// @file
///

void
print(const char format[], ...)  __attribute__((format(printf, 1, 2)));


#endif