#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "connection.h"
#include "surface.h"
#include "encode.h"
#include "decode.h"

int main() {
	Connection *working;
	Surface *insurface;
	Surface *outsurface;
	DecodeParams decodeparams;
	int ret;
	FILE *out;
	int i;

	insurface = surface_init(640, 480);
	if(insurface == NULL) {
		fprintf(stderr, "Couldn't allocate memory.\n");
		return(-1);
	}
	outsurface = surface_init(640, 480);
	if(outsurface == NULL) {
		fprintf(stderr, "Couldn't allocate memory.\n");
		return(-1);
	}

	for(i = 0; i < 640 * 480; i++) {
		insurface->luma[i] = 127;
	}
	for(i = 0; i < 640 * 480; i++) {
		outsurface->luma[i] = 0;
	}

	decodeparams.syncthresh = 0.9;
	decodeparams.fuzzyness = 2;
	decodeparams.autolevel = 0.9;
	decodeparams.autostride = 0.9;

	working = NULL;
	ret = analog_encode(&working, insurface, 10);
	if(ret < 0) {
		fprintf(stderr, "Encode failed.\n");
		return(-2);
	}

/*	srand(time(NULL));
	for(i = 0; i < working->length; i++) {
		working->stream[i] += (double)(rand() % 2000) / 100 - 10;
	}
*/
	ret = analog_decode(working, outsurface, &decodeparams);
	if(ret < 0) {
		fprintf(stderr, "Decode failed.\n");
		return(-2);
	}

	out = fopen("out.bin", "wb");
	fwrite(outsurface->luma, outsurface->width * outsurface->height, 1, out);
	fclose(out);

	return(0);
}
