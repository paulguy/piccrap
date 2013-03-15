#ifndef _SURFACE_H
#define _SURFACE_H

#include <stdlib.h>

typedef struct {
	int width, height;
	char *luma;
} Surface;

Surface *surface_init(int width, int height);

#endif
