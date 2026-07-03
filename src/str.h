// str.h: simple string utilities
#ifndef STR_H
#define STR_H
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "types.h"

// Init memory arena for all strs and returns if it succeeded
bool StrInit(size n_bytes);

// Deinitializes memory arena for all strs
void StrDeinit(void);

// Create a string buffer and returns it (allocates 1 extra for '\0')
char *Str(size n_bytes);

// Concatenates multiple strings and returns result (NULL on failure)
char *StrConcatenate(int n_strs, ...);

#endif // STR_H
