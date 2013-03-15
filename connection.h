#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <stdlib.h>

typedef struct {
	double *stream;
	int length;
} Connection;

Connection *connection_init(int length);

#endif
