#include <stdlib.h>
#include <stdio.h>

#include "connection.h"
#include "surface.h"
#include "encoder.h"
#include "decoder.h"

int main(int argc, char **argv) {
	Connection *working;
	Surface *insurface;
	Surface *outsurface;
	DecodeParams decodeparams;
	int ret;
	FILE *out;

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

	decodeparams->syncthresh = 0.9;
	decodeparams->fuzzyness = 8;
	decodeparams->autolevel = 0.1;
	decodeparams->autostride = 0.1;

	working = NULL;
	ret = encode(&working, insurface, 10);
	if(ret < 0) {
		fprintf(stderr, "Encode failed.\n");
		return(-2);
	}
	ret = decode(working, outsurface);
	if(ret < 0) {
		fprintf(stderr, "Decode failed.\n");
		return(-2);
	}

	out = fopen("out.bin", "wb");
	fwrite(outsurface->luma, outsurface->width * outsurface->height, 1, out);
	fclose(out);

	return(0);
}
