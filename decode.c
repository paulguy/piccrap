#include "decode.h"

int analog_decode(Connection *connection, Surface *surface, DecodeParams *params) {
	double invautolevel = 1.0 - params->autolevel;
	double invautostride = 1.0 - params->autostride;

	int i;
	int currentline;
	int xpos, avgsample, samples;
	double xposd;

	double stride, runningstride;
	int activestart;
	int fuzz;
	double sampleperiod;

	SyncMode syncing;
	double synclevel, blanklevel;
	double whitelevel, blacklevel;

	currentline = 0;
	syncing = SYNC_SYNC;
	synclevel = SYNC_VAL; /* start with nominal values */
	blanklevel = BLANK_VAL;
	fuzz = params->fuzzyness;
	stride = 0;
	activestart = 0;
	for(i = 0; i < connection->length; i++) {
/*		fprintf(stderr, "%f %f %f\n", connection->stream[i], synclevel, synclevel * params->syncthresh);
*/		if(currentline < SYNC_LINES) {
			switch (syncing) {
				case SYNC_SYNC:
					if(connection->stream[i] < (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
							syncing = SYNC_BLANK;
						}
						fuzz--;
					} else {
						synclevel = synclevel*params->autolevel + connection->stream[i]*invautolevel;
						fuzz = params->fuzzyness;
					}
					stride++;
					break;
				case SYNC_BLANK:
					if(connection->stream[i] > (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
fprintf(stderr, "%d %f %f %f %f\n", currentline, stride, runningstride, synclevel, blanklevel);
							if(currentline == 0) {
								runningstride = stride;
							} else {
								runningstride = runningstride*params->autostride + stride*invautostride;
							}
							stride = 0;
							currentline++;
							fuzz = params->fuzzyness;
							syncing = SYNC_SYNC;
						}
						fuzz--;
					} else {
						blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
						fuzz = params->fuzzyness;
					}
					stride++;
					break;
				default:
					return(-1);
			}
		} else {  /* active lines */
			switch (syncing) {
				case SYNC_SYNC:
					if(connection->stream[i] < synclevel) {
						if(fuzz == 0) {
							blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
							syncing = SYNC_BLANK;
						}
						fuzz--;
					} else {
						synclevel = synclevel*params->autolevel + connection->stream[i]*invautolevel;
						if(fuzz < params->fuzzyness) {
							activestart += params->fuzzyness - fuzz;
							fuzz = params->fuzzyness;
						}
						activestart++;
					}
					stride++;
					break;
				case SYNC_BLANK:
					blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
					if(connection->stream[i] < blanklevel) {
						if(fuzz == 0) {
							sampleperiod = (double)(surface->width) / (runningstride - (double)activestart);
fprintf(stderr, "%d %f ", activestart, sampleperiod);
							activestart = 0;
							xpos = 0;
							xposd = 0.0;
							samples = 0;
							avgsample = 0;
							whitelevel = blanklevel * (WHITE_VAL / BLANK_VAL);
							blacklevel = blanklevel * (BLACK_VAL / BLANK_VAL);
fprintf(stderr, "%f %f %f\n", blanklevel, blacklevel, whitelevel);
							fuzz = params->fuzzyness;
							syncing = SYNC_ACTIVE;
						}
						fuzz--;
					} else {
						if(fuzz < params->fuzzyness) {
							activestart += params->fuzzyness - fuzz;
							fuzz = params->fuzzyness;
						}
						activestart++;
					}
					stride++;
					break;
				case SYNC_ACTIVE:
					if(connection->stream[i] > (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							/*runningstride = runningstride*params->autostride + stride*invautostride;*/
							stride = 0;
							xpos = 0;
							currentline++;
							fuzz = params->fuzzyness;
							syncing = SYNC_SYNC;
fprintf(stderr, "%d %f %f %f %f\n", currentline, stride, runningstride, synclevel, blanklevel);
fprintf(stderr, "Line %d ended at %d due to hsync reached.\n", currentline, i);
						}
					}
					avgsample += connection->stream[i];
					samples++;
					xposd += sampleperiod;
					if((int)xposd > xpos) {
						samples = 1;
						avgsample = connection->stream[i];
						if(avgsample < whitelevel) {
							avgsample = whitelevel;
						} else if(avgsample > blacklevel) {
							avgsample = blacklevel;
						}
						surface->luma[(currentline - SYNC_LINES) * surface->width + xpos] = (int)(255.0 - ((avgsample / samples) - whitelevel) * (255.0 / (blacklevel - whitelevel)));
						avgsample = 0;
						samples = 0;
						xpos++;
						if(xpos >= surface->width) {
fprintf(stderr, "%d %f %f %f %d %f\n", currentline, stride, runningstride, xposd, xpos, synclevel);
fprintf(stderr, "Ran out of framebuffer on line %d.\n", currentline);
							syncing = SYNC_END;
						}
					}
					stride++;
					break;
				case SYNC_END:
					if(connection->stream[i] > (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							/*runningstride = runningstride*params->autostride + stride*invautostride;*/
							stride = 0;
							xpos = 0;
							currentline++;
							fuzz = params->fuzzyness;
							syncing = SYNC_SYNC;
						}
						fuzz--;
					} else {
						fuzz = params->fuzzyness;
					}
					break;
			}
		}
	}

fprintf(stderr, "Read ended at %d, %d.\n", xpos, currentline);
	return(0);
}

