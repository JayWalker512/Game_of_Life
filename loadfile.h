#ifndef LOADFILE_H
#define LOADFILE_H

#include "threadlife.h"

#define MAX_FILENAME_LENGTH 128

/* Loads a plaintext format life file to a world buffer.
Returns 1 on success, 0 otherwise. */
char LoadLifeWorld(LifeWorldBuffer_t *dest, const char *file, char bCenter);

/* Saves the entire content of the world buffer to a plaintext file. 
Returns 1 on success, 0 otherwise. */
char SaveLifeWorld(const char *file, LifeWorldBuffer_t *source);

#endif
