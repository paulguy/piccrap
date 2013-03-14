#include "encode.h"

/* structure of an encoded frame
 * --- Frame ---
 * 9 vertical sync lines (find approx length of a line)
 * X Active Lines
 *
 * --- vsync line ---
 * 48 sync region (find lengths of lines) (-40)
 * 56 + active region (blanking level) (0)
 *
 * --- Active Line ---
 * 104 units sync
 * 48 sync region (find lengths of lines) (-40)
 * 44 blanking level (reference for next line, prepare start of line) (0)
 * XX active region (7.5 - 100)
 * 12 blanking level (end line) (0)
 */

int analog_encode(Connection **connection, Surface *surface, int multiplier) {
	int i, j;
	int offset, lineoffset;

	int synclength = SYNC_LENGTH * multiplier;
	int frontporchlength = FRONT_PORCH * multiplier;
	int totalsynclength = synclength + frontporchlength;
	int activelength = surface->width * multiplier;
	int backporchlength = BACK_PORCH * multiplier;
	int stride = totalsynclength + activelength + backporchlength;
	int backporchstart = stride - backporchlength;
	int lines = surface->height + SYNC_LINES;

	if(*connection == NULL) {
		*connection = (Connection *)malloc(sizeof(Connection));
		if(*connection == NULL) {
			return(-1);
		}
		*connection->length = stride * lines;
		*connection->stream = (double *)malloc(sizeof(double) * *connection->length);
		if(*connection->stream == NULL) {
			free(*connection);
			return(-1);
		}
	}

	offset = 0;
	/* sync lines */
	for(i = 0; i < lines; i++) {
		if(i < SYNC_LINES) {
			for(j = 0; j < stride; j++) {
				if(j < synclength) {
					*connection->stream[offset] = SYNC_VAL;
				} else {
					*connection->stream[offset] = BLANK_VAL;
				}
				offset++;
			}
		} else {
			lineoffset = 0;
			for(j = 0; j < stride; j++) {
				if(j < totalsynclength) {
					if(j < synclength) {
						*connection->stream[offset] = SYNC_VAL;
					} else {
						*connection->stream[offset] = BLANK_VAL;
					}
				} else if (j > backporchstart) {
					*connection->stream[offset] = BLANK_VAL;
				} else {
					*connection->stream[offset] = ((255.0 - (double)(surface->luma[(i - SYNC_LINES) * surface->width + (lineoffset / multiplier)])) * LUMA_NORM_MUL + WHITE_VAL;
				}
				lineoffset++;
				offset++;
			}
		}
	}

	return(0);
}
