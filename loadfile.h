#ifndef LOADFILE_H
#define LOADFILE_H

#include "threadlife.h"

#define MAX_FILENAME_LENGTH 128

/* Loads a plaintext format life file to a world buffer.
Returns 1 on success, 0 otherwise. */
char LoadLifeWorld(LifeWorldBuffer_t *dest, const char *file, char bCenter);

#endif
