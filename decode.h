#include "connection.h"
#include "surface.h"
#include "signalvalues.h"

#include <stdio.h>

typedef struct {
	double syncthresh;
	double blankthresh;
	double autolevel;
	double autostride;
	int fuzzyness;
} DecodeParams;

typedef enum {
	SYNC_SYNC,
	SYNC_BLANK,
	SYNC_ACTIVE,
	SYNC_END
} SyncMode;

int analog_decode(Connection *connection, Surface *surface, DecodeParams *params);
