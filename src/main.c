#include <stdlib.h>
#include <stdbool.h>

#include <stb_image.h>
#include <stb_ds.h>

#include <nobuild/nobuild_log.h>
#include <nobuild/nobuild_cstr.h>
#include <nobuild/nobuild_path.h>

#include "main.h"

typedef struct {
	unsigned char *data; // Pointer to the RGBA array
	int width;           // Width of the image
	int height;          // Height of the image
	int channels;        // Number of channels in the image
} Image;

#define INDEX_IMG(i, x, y) (i.width * y + x) * i.channels
#define INDEX_IMGP(i, x, y) (i->width * y + x) * i->channels

static bool is_corner_pixel(const Image *img, int x, int y) {
	int w = img->width;
	int h = img->height;

	// Calculate ahead of time to simplify the conditions
	bool is_trans_top          = (y > 0)              ? (img->data[INDEX_IMGP(img,     x, (y-1)) + 3] == 0) : true;
	bool is_trans_bottom       = (y < h-1)            ? (img->data[INDEX_IMGP(img,     x, (y+1)) + 3] == 0) : true;

	bool is_trans_left         = (x > 0)              ? (img->data[INDEX_IMGP(img, (x-1),     y) + 3] == 0) : true;
	bool is_trans_right        = (x < w-1)            ? (img->data[INDEX_IMGP(img, (x+1),     y) + 3] == 0) : true;

	bool is_trans_top_left     = (x > 0 && y > 0)     ? (img->data[INDEX_IMGP(img, (x-1), (y-1)) + 3] == 0) : true;
	bool is_trans_top_right    = (x < w-1 && y > 0)   ? (img->data[INDEX_IMGP(img, (x+1), (y-1)) + 3] == 0) : true;

	bool is_trans_bottom_left  = (x > 0 && y < h-1)   ? (img->data[INDEX_IMGP(img, (x-1), (y+1)) + 3] == 0) : true;
	bool is_trans_bottom_right = (x < w-1 && y < h-1) ? (img->data[INDEX_IMGP(img, (x+1), (y+1)) + 3] == 0) : true;

	// Edge case 1: The Pixel is protruding from the image. That means that the pixel is only connected on main side with
	// the 2 diagnoal sides being non transparent. i.e tetris T block eg:
	//
	//     +---+---+---+
	//     |   |   |   |
	//     +---+---+---+
	//     | 0 | * | 0 |
	//     +---+---+---+
	//     | 0 | 0 | 0 |
	//     +---+---+---+
	//
	if ((!is_trans_top_left && !is_trans_top && !is_trans_top_right && is_trans_right && is_trans_bottom_right && is_trans_bottom && is_trans_bottom_left && is_trans_left)
		|| (!is_trans_top_right && !is_trans_right && !is_trans_bottom_right && is_trans_bottom && is_trans_bottom_left && is_trans_left && is_trans_top_left && is_trans_top)
		|| (!is_trans_bottom_right && !is_trans_bottom && !is_trans_bottom_left && is_trans_left && is_trans_top_left && is_trans_top && is_trans_top_right && is_trans_right)
		|| (!is_trans_bottom_left && !is_trans_left && !is_trans_top_left && is_trans_top && is_trans_top_right && is_trans_right && is_trans_bottom_right && is_trans_bottom)) {
		TODO("Fucking deal with this edge case: { x: %d, y: %d }\n", x, y);
	}

	// Check for convex corner pixels. A convex corner pixel must have atleast 3 neighbors be transparent and the
	// neighbors must be adjacent to one another and one of them must be a diagonal pixel. eg:
	//
	//     +---+---+---+
	//     | 0 | 0 |   |
	//     +---+---+---+
	//     | 0 | * |   |
	//     +---+---+---+
	//     |   |   |   |
	//     +---+---+---+
	//
	if ((is_trans_left && is_trans_top_left && is_trans_top)
		|| (is_trans_top && is_trans_top_right && is_trans_right)
		|| (is_trans_right && is_trans_bottom_right && is_trans_bottom)
		|| (is_trans_bottom && is_trans_bottom_left && is_trans_left)) {
		return true;
	}

	// Edge Case 2: Do convex corners are infront of each another
	//
	//  +---+---+---+---+
	//  |   |   |   |   |
	//  +---+---+---+---+
	//  |   | * | 0 |   |
	//  +---+---+---+---+
	//  |   | 0 | * |   |
	//  +---+---+---+---+
	//  |   |   |   |   |
	//  +---+---+---+---+
	//
	if ((is_trans_left && !is_trans_top_left && is_trans_top)
		|| (is_trans_top && !is_trans_top_right && is_trans_right)
		|| (is_trans_right && !is_trans_bottom_right && is_trans_bottom)
		|| (is_trans_bottom && !is_trans_bottom_left && is_trans_left)) {
		return true;
	}

	// Check for concave corner pixels. A concave corner pixel must have exactly one diagonal neighbor be
	// transparent. eg:
	//
	//     +---+---+---+
	//     | 0 |   |   |
	//     +---+---+---+
	//     |   | * |   |
	//     +---+---+---+
	//     |   |   |   |
	//     +---+---+---+
	//
	if ((is_trans_top_left && !is_trans_top && !is_trans_top_right && !is_trans_right && !is_trans_bottom_right && !is_trans_bottom && !is_trans_bottom_left && !is_trans_left)
		|| (is_trans_top_right && !is_trans_right && !is_trans_bottom_right && !is_trans_bottom && !is_trans_bottom_left && !is_trans_left && !is_trans_top_left && !is_trans_top)
		|| (is_trans_bottom_right && !is_trans_bottom && !is_trans_bottom_left && !is_trans_left && !is_trans_top_left && !is_trans_top && !is_trans_top_right && !is_trans_right)
		|| (is_trans_bottom_left && !is_trans_left && !is_trans_top_left && !is_trans_top && !is_trans_top_right && !is_trans_right && !is_trans_bottom_right && !is_trans_bottom)) {
		return true;
	}

	return false;
}

typedef struct RectilinearPoint {
	int x, y;
} RectilinearPoint;

static void extract_polygon(Image *img, RectilinearPoint **points) {
	for(int y = 0; y < img->height; y++) {
		for(int x = 0; x < img->width; x++) {
			// Check if the pixel is transparent
			if (img->data[INDEX_IMGP(img, x, y) + 3] == 0) {
				continue;
			}

			if (is_corner_pixel(img, x, y)) {
				RectilinearPoint point = { .x = x, .y = y };
				arrput(*points, point);
			}
		}
	}
}

typedef struct Edge {
	RectilinearPoint key, value;
} Edge;

static Edge *get_edges(RectilinearPoint *points, size_t point_count, bool cmp_by_x) {
	Edge *map = NULL;
	for (size_t i = 0; i < point_count;) {
		size_t last_idx = point_count;
		for (size_t j = i; j < point_count; ++j) {
			if ((cmp_by_x && points[j].x != points[i].x) || (!cmp_by_x && points[j].y != points[i].y)) {
				last_idx = j;
				break;
			}
		}

		bool in_edge = false;
		RectilinearPoint *first_point = NULL;
		for (size_t j = i; j < last_idx; ++j) {
			if (in_edge) {
				hmput(map, *first_point, points[j]);
				hmput(map, points[j], *first_point);
				in_edge = false;
			} else {
				first_point = points + j;
				in_edge = true;
			}
		}

		i = last_idx;
	}

	return map;
}

static bool sort_by_x = true;
static int is_less_by_axis(const void *_a, const void *_b) {
	RectilinearPoint a = *((RectilinearPoint *) _a); RectilinearPoint b = *((RectilinearPoint *) _b);
	if (sort_by_x)
		return a.x == b.x ? a.y - b.y : a.x - b.x;
	else
		return a.y == b.y ? a.x - b.x : a.y - b.y;
}

void rectilinearize_image(unsigned char *data, int width, int height, int **points, size_t *point_count) {
	RectilinearPoint *rect_points = NULL;

	Image img = {
		.data = data, .width = width, .height = height, .channels = 4
	};
	extract_polygon(&img, &rect_points);

	if (rect_points == NULL) {
		return;
	}

	// Final sorted points
	RectilinearPoint *sorted_points = NULL;
	arrput(sorted_points, rect_points[0]);

	sort_by_x = false;
	qsort(rect_points, arrlenu(rect_points), sizeof *rect_points, is_less_by_axis);
	Edge *h_edges = get_edges(rect_points, arrlenu(rect_points), false);
	if (h_edges == NULL) {
		return;
	}

	sort_by_x = true;
	qsort(rect_points, arrlenu(rect_points), sizeof *rect_points, is_less_by_axis);
	Edge *v_edges = get_edges(rect_points, arrlenu(rect_points), true);
	if (v_edges == NULL) {
		return;
	}

	bool is_y_axis = true;
	while (true) {
		RectilinearPoint next_point;
		if (is_y_axis) {
			next_point = hmget(h_edges, arrlast(sorted_points));
		} else {
			next_point = hmget(v_edges, arrlast(sorted_points));
		}

		if (next_point.x == sorted_points[0].x && next_point.y == sorted_points[0].y) {
			break;
		}

		arrput(sorted_points, next_point);
		is_y_axis = !is_y_axis;
	}

	if (points && point_count) {
		size_t p_count = arrlenu(sorted_points);
		int *p = malloc(sizeof *p * (p_count << 1));
		if (p == NULL) {
			return;
		}
		for (size_t i = 0; i < p_count; ++i) {
			p[(i << 1)]     = sorted_points[i].x;
			p[(i << 1) + 1] = sorted_points[i].y;
		}

		*points = p;
		*point_count = p_count;
	}
}

void rectilinearize_file(const char *filename, int **points, size_t *point_count) {
	Image img = {0};
	img.data = stbi_load(filename, &img.width, &img.height, &img.channels, 4);
	if (img.channels != 4) {
		return;
	}

	rectilinearize_image(img.data, img.width, img.height, points, point_count);
}

#ifdef BINARY
int main(int argc, char **argv) {
	const char *filename = NULL;
	bool svg_output = false;
	for (int i = 1; i < argc; ++i) {
		if (STARTS_WITH(argv[i], "--output-as-svg")) {
			svg_output = true;
			continue;
		}

		filename = argv[i];
	}

	if (filename == NULL) {
		PANIC("Missing file argument.");
	}

	int *points = NULL;
	size_t point_count = 0;
	rectilinearize_file(filename, &points, &point_count);

	int width, height = width = 0;
	for (size_t i = 0; i < point_count; i+=2) {
		width  = points[i]   > width  ? points[i]   : width;
		height = points[i+1] > height ? points[i+1] : height;
	}

	if (svg_output) {
		printf(
			"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
			"<!-- Created with Inkscape (http://www.inkscape.org/) -->\n"
			"\n"
			"<svg\n"
			"	width=\"%d\"\n"
			"	height=\"%d\"\n"
			"	viewBox=\"0 0 %d %d\"\n"
			"	version=\"1.1\"\n"
			"	id=\"svg5\"\n"
			"	inkscape:version=\"1.2.2 (b0a8486541, 2022-12-01)\"\n"
			"	sodipodi:docname=\"drawing.svg\"\n"
			"	xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\"\n"
			"	xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n"
			"	xmlns=\"http://www.w3.org/2000/svg\"\n"
			"	xmlns:svg=\"http://www.w3.org/2000/svg\">\n"
			"	<sodipodi:namedview\n"
			"		id=\"namedview7\"\n"
			"		pagecolor=\"#ffffff\"\n"
			"		bordercolor=\"#000000\"\n"
			"		borderopacity=\"0.25\"\n"
			"		inkscape:showpageshadow=\"2\"\n"
			"		inkscape:pageopacity=\"0.0\"\n"
			"		inkscape:pagecheckerboard=\"0\"\n"
			"		inkscape:deskcolor=\"#d1d1d1\"\n"
			"		inkscape:document-units=\"mm\"\n"
			"		showgrid=\"false\"\n"
			"		inkscape:zoom=\"0.77593294\"\n"
			"		inkscape:cx=\"396.94152\"\n"
			"		inkscape:cy=\"561.90423\"\n"
			"		inkscape:window-width=\"1916\"\n"
			"		inkscape:window-height=\"1049\"\n"
			"		inkscape:window-x=\"1920\"\n"
			"		inkscape:window-y=\"27\"\n"
			"		inkscape:window-maximized=\"1\"\n"
			"		inkscape:current-layer=\"layer1\" />\n"
			"	<defs\n"
			"		id=\"defs2\" />\n"
			"	<g\n"
			"		inkscape:label=\"Layer 1\"\n"
			"		inkscape:groupmode=\"layer\"\n"
			"		id=\"layer1\" />\n"
			"\n"
			"	<g\n"
			"		stroke=\"black\"\n"
			"		stroke-width=\"1px\">\n", width, height, width, height
		);

		for (size_t i = 0; i < point_count - 2; i+=2) {
			printf("		<line x1=\"%dpx\" y1=\"%dpx\" x2=\"%dpx\" y2=\"%dpx\"/>\n",
					points[i], points[i+1], points[i+2], points[i+3]);
		}

		printf(
			"	</g>\n"
			"</svg>\n"
		 );
	} else {
		printf("[\n");
		for (size_t i = 0; i < point_count - 2; i+=2) {
			printf("\t{ \"x\": %d, \"y\": %d },\n", points[i], points[i+1]);
		}
		printf("\t{ \"x\": %d, \"y\": %d }\n", points[point_count - 2], points[point_count - 1]);
		printf("]\n");
	}
}
#endif //BINARY
