typedef struct {
	int width, height;
	char *luma;
} Surface;

Surface *surface_init(int width, int height) {
	Surface *surface;

	surface = (Surface *)malloc(sizeof(Surface));
	if(surface == NULL) {
		return(NULL);
	}
	surface->width = width;
	surface->height = height;
	surface->luma = (char *)malloc(sizeof(char) * width * height);
	if(surface->luma == NULL) {
		free(surface);
		return(NULL);
	}

	return(surface);
}
