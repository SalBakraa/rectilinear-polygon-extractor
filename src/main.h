#ifndef RECTILIEARIZE_H_
#define RECTILIEARIZE_H_

#include <stddef.h>

/**
 * @brief Converts an image represented by an array of RGBA values to rectilinear polygon.
 *
 * This function takes the input image data and creates a rectilinear polygon representing the contents of the
 * image. The image *MUST* have a fully transperent background.
 *
 * @param data A pointer to the array of RGBA values representing the input image.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param points A pointer to an array of XY values representing the vertices of the rectilinear polygon. This array
 *               will be allocated by the function.
 * @param point_count A pointer to a size_t variable that will be set to the length of the points array.
 *
 * @note The 'data' array is expected to be in RGBA format, where each pixel is represented by four consecutive
 *       unsigned char values: red, green, blue, and alpha channels.
 * @note The 'points' array is an even length array where every two elements is a pair of X-Y coordinates.
 */
void rectilinearize_image(unsigned char *data, int width, int height, int **points, size_t *point_count);

/**
 * @brief Converts an image represented by an array of RGBA values to rectilinear polygon.
 *
 * This function takes the path to png image and creates a rectilinear polygon representing the contents of the
 * image. The image *MUST* have a fully transperent background.
 *
 * @param filename Path to a png file.
 * @param points A pointer to an array of XY values representing the vertices of the rectilinear polygon. This array
 *               will be allocated by the function.
 * @param point_count A pointer to a size_t variable that will be set to the length of the points array.
 *
 * @note The 'filename' string must point to a valid png file with an alpha channel
 * @note The 'points' array is an even length array where every two elements is a pair of X-Y coordinates.
 */
void rectilinearize_file(const char *filename, int **points, size_t *point_count);

#endif  // RECTILIEARIZE_H_
