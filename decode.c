#include "decode.h"

int analog_decode(Connection *connection, Surface *surface, DecodeParams *params) {
	double invautolevel = 1.0 - params->autolevel;

	int i;
	int currentline, linestart;
	int xpos, avgsample, samples;
	double xposd;

	int runningstride;
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
	activestart = 0;
	linestart = 0;
	for(i = 0; i < connection->length; i++) {
/*		fprintf(stderr, "%f %f %f\n", connection->stream[i], synclevel, synclevel * params->syncthresh);
*/		if(currentline < SYNC_LINES) {
			switch (syncing) {
				case SYNC_SYNC:
					if(connection->stream[i] < (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							i -= params->fuzzyness;
							fuzz = params->fuzzyness;
							syncing = SYNC_BLANK;
						} else {
							fuzz--;
						}
					} else {
						synclevel = synclevel*params->autolevel + connection->stream[i]*invautolevel;
						if(fuzz < params->fuzzyness) {
							fuzz = params->fuzzyness;
						}
					}
					break;
				case SYNC_BLANK:
					if(connection->stream[i] > (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							i -= params->fuzzyness;
							if(currentline == 0) {
								runningstride = i - linestart;
							} else {
								runningstride += i - linestart;
							}
fprintf(stderr, "%d %d %d %d %f %f\n", currentline, i - linestart, runningstride, runningstride / (currentline + 1), synclevel, blanklevel);
							linestart = i;
							currentline++;
							fuzz = params->fuzzyness;
							syncing = SYNC_SYNC;
						} else {
							fuzz--;
						}
					} else {
						blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
						if(fuzz < params->fuzzyness) {
							fuzz = params->fuzzyness;
						}
					}
					break;
				default:
					return(-1);
			}
		} else {  /* active lines */
			switch (syncing) {
				case SYNC_SYNC:
					if(connection->stream[i] < synclevel) {
						if(fuzz == 0) {
							i -= params->fuzzyness;
							fuzz = params->fuzzyness;
							syncing = SYNC_BLANK;
						} else {
							fuzz--;
						}
					} else {
						synclevel = synclevel*params->autolevel + connection->stream[i]*invautolevel;
						if(fuzz < params->fuzzyness) {
							activestart += params->fuzzyness - fuzz;
							fuzz = params->fuzzyness;
						}
					}
					break;
				case SYNC_BLANK:
					if(connection->stream[i] < blanklevel) {
						if(fuzz == 0) {
							i -= params->fuzzyness;
							activestart = i - linestart;
							sampleperiod =	(double)(surface->width) / ((double)(runningstride / currentline) -
											(double)activestart - /* sync beginning */
											((BACK_PORCH / (FRONT_PORCH + SYNC_LENGTH)) * (double)activestart)); /* nominal end */
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
						} else {
							fuzz--;
						}
					} else {
						blanklevel = blanklevel*params->autolevel + connection->stream[i]*invautolevel;
						if(fuzz < params->fuzzyness) {
							activestart += params->fuzzyness - fuzz;
							fuzz = params->fuzzyness;
						}
					}
					break;
				case SYNC_ACTIVE:
					if(connection->stream[i] > (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							i -= params->fuzzyness;
							runningstride += i - linestart;
							linestart = i;
							xpos = 0;
							currentline++;
							fuzz = params->fuzzyness;
							syncing = SYNC_SYNC;
fprintf(stderr, "%d %d %d %f %f\n", currentline, i - linestart, runningstride, synclevel, blanklevel);
fprintf(stderr, "Line %d ended at %d due to hsync reached.\n", currentline, i);
						} else {
							fuzz--;
						}
					} else {
						fuzz = params->fuzzyness;
					}
					if(connection->stream[i] < whitelevel) {
						avgsample += whitelevel;
					} else if(connection->stream[i] > blacklevel) {
						avgsample += blacklevel;
					} else {
						avgsample += connection->stream[i];
					}
					samples++;
					xposd += sampleperiod;
					if((int)xposd > xpos) {
						surface->luma[(currentline - SYNC_LINES) * surface->width + xpos] = (int)(255.0 - ((avgsample / (double)samples) - whitelevel) * (255.0 / (blacklevel - whitelevel)));
						avgsample = 0;
						samples = 0;
						xpos++;
						if(xpos >= surface->width) {
fprintf(stderr, "Ran out of framebuffer on line %d.\n", currentline);
							syncing = SYNC_END;
						}
					}
					break;
				case SYNC_END:
					if(connection->stream[i] > (synclevel * params->syncthresh)) {
						if(fuzz == 0) {
							i -= params->fuzzyness;
							runningstride += i - linestart;
							currentline++;
fprintf(stderr, "%d %d %d %d %f %d %f\n", currentline, i - linestart, runningstride, runningstride / currentline, xposd, xpos, synclevel);
							linestart = i;
							xpos = 0;
							fuzz = params->fuzzyness;
							syncing = SYNC_SYNC;
						} else {
							fuzz--;
						}
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

