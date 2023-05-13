#ifndef RECTILIEARIZE_H_
#define RECTILIEARIZE_H_

#include <stddef.h>

typedef struct {
	int x, y;
} Point;

typedef struct {
	unsigned char *data; // Pointer to the RGBA array
	int width;           // Width of the image
	int height;          // Height of the image
	int channels;        // Number of channels in the image
} Image;

// The point array is allocated using the "stb_ds.h" library
const Point *rectilinearize_image(Image *img, size_t *point_count);
const Point *rectilinearize_file(const char *filename, size_t *point_count);

#endif  // RECTILIEARIZE_H_
