#include <stdlib.h>

typedef struct {
	double *stream;
	int length;
} Connection;

Connection *connection_init(int length);
