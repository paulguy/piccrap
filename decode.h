#include "connection.h"
#include "surface.h"
#include "signalvalues.h"

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

int analog_encode(Connection *connection, Surface *surface, DecodeParams *params);
