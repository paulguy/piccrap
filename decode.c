#include "decode.h"

int analog_encode(Connection *connection, Surface *surface, DecodeParams *params) {
	double invautolevel = 1.0 - params->autolevel;
	double invautostride = 1.0 - params->autostride;

	int i;
	int currentline;
	int xpos, avgsample, samples;
	double xposd;

	int stride, runningstride;
	int activestart;
	int fuzz;
	double sampleperiod;

	SyncMode syncing;
	double synclevel, blanklevel;

	currentline = 0;
	syncing = SYNC_START;
	synclevel = SYNC_VAL; /* start with nominal values */
	blanklevel = BLANK_VAL;
	fuzz = params->fuzzyness;
	stride = 0;
	activestart = 0;
	for(i < 0; i < connection->length; i++) {
		if(currentline < SYNC_LINES) {
			switch (syncing) {
				case SYNC_SYNC:
					synclevel = synclevel*params->autolevel + connection->stream[i]*invautolevel;
					if(connection->stream[i] < (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
							syncing = SYNC_BLANK;
						}
						fuzz--;
					} else {
						fuzz = params->fuzzyness;
					}
					stride++;
					break;
				case SYNC_BLANK:
					blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
					if(connection->stream[i] > (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							if(currentline == 0) {
								runningstride = stride;
							} else {
								runningstride = runningstride*params->autostride + stride*invautostride;
							}
							stride = 0;
							fuzz = params->fuzzyness;
							syncing = SYNC_SYNC;
						}
						fuzz--;
					} else {
						fuzz = params->fuzzyness;
					}
					stride++;
					break;
			}
		} else {  /* active lines */
			switch (syncing) {
				case SYNC_SYNC:
					synclevel = synclevel*params->autolevel + connection->stream[i]*invautolevel;
					if(connection->stream[i] < (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
							syncing = SYNC_BLANK;
						}
						fuzz--;
					} else {
						fuzz = params->fuzzyness;
					}
					stride++;
					activestart++;
					break;
				case SYNC_BLANK:
					blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
					if(connection->stream[i] < (blanklevel * params->blankthresh)) {
						if(fuzz == 0) {
							sampleperiod = (double)(surface->width) / (runningstride - activestart);
							activestart = 0;
							xpos = 0;
							xposd = 0.0;
							samples = 0;
							avgsample = 0;
							fuzz = params->fuzzyness;
							syncing = SYNC_ACTIVE;
						}
						fuzz--;
					} else {
						fuzz = params->fuzzyness;
					}
					stride++;
					activestart++;
					break;
				case SYNC_ACTIVE:
					if(xpos >= surface->width) {
						syncing = SYNC_END;
					}
					if(connection->stream[i] > (blanklevel * params->blankthresh)) {
						if(connection->stream[i] > (synclevel * params->syncthresh)) {
							if(fuzz == 0) {
								runningstride = runningstride*params->autostride + stride*invautostride;
								stride = 0;
								fuzz = params->fuzzyness;
								syncing = SYNC_SYNC;
							}
						} else {
							fuzz = params->fuzzyness;
							if(fuzz == 0) {
								stride = 0;
								fuzz = params->fuzzyness;
								syncing = SYNC_END;
							}
						}
					} else {
						avgsample = connection->stream[i];
						samples++;
						xposd += sampleperiod;
						if((int)xposd > xpos) {
							surface->luma[currentline * surface->width + xpos] = avgsample / samples;
							samples = 0;
							xpos++;
						}
					}
					stride++;
					break;
				case SYNC_END:
					blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
					if(connection->stream[i] > (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							runningstride = runningstride*params->autostride + stride*invautostride;
							stride = 0;
							fuzz = params->fuzzyness;
							syncing = SYNC_SYNC;
						}
						fuzz--;
					} else {
						fuzz = params->fuzzyness;
					}
					stride++;
					break;
			}
		}
	}

	return(0);
}

