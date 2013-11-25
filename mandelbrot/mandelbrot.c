/*
	Copyright (c) 2013 Simon Otter

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

/*
	This code uses C99-stuff and is therefore not compatible with C89 and such.

	Use GCC or Clang with std=c99, or something else that supports C99.
*/

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "complex.h"

#define RERROR(reason, code) \
	{ \
		puts("Error: " #reason); \
		return (code); \
	} 
#define ERROR_DIMENSIONS_TO_SMALL 	200
#define ERROR_FILE_LOCKED 			201
#define ERROR_TGA_CREATION			202
#define ERROR_TGA_WRITE				203

/*
	TGA METHODS.
*/
typedef struct tga_data_s
{
	/* 
		The image data is arranged from top left to bottom right,
		BGRA (32 bit) or BGR (24 bit).

		Thus data[0] is the blue byte of the topmost left corner.
	*/
	uint8_t  *data;
	uint16_t width;
	uint16_t height;
	/* 24 or 32 (32 is with alpha mask.) */
	uint8_t  bitdepth;
} tga_data;

uint64_t tga_len(uint16_t w, uint16_t h, uint8_t bitdepth)
{
	uint64_t bytespp = bitdepth == 24? 3 : 4;
	uint64_t len = ((uint32_t) w) * ((uint32_t) h) * bytespp;
	return len;
}

// bithdepth 24 or 32
tga_data *tga_create(uint32_t w, uint32_t h, uint8_t bitdepth)
{
	if (bitdepth != 24 && bitdepth != 32)
		return NULL;

	tga_data *tga = malloc(sizeof(tga_data));
	if (tga != NULL)
	{
		tga->width = w;
		tga->height = h;
		tga->bitdepth = bitdepth;

		uint64_t len = tga_len(w, h, bitdepth);
		tga->data = malloc(len);

		if (tga->data == NULL)
		{
			free(tga);
			tga = NULL;
		}
	}
	return tga;
}

void tga_free(tga_data *tga)
{
	if (tga != NULL)
		free(tga->data);
	free(tga);
}

void tga_write(tga_data *data, FILE *f)
{
	/* Write the header. */

	/* Identification field. */
	/* We don't support the image identification field. */
	putc(0, f);

	/* Color map type. */
	/* Always zero because we don't support color mapped images. */
	putc(0, f);

	/* Image type code. */
	/* Always 2 since we only support uncompressed RGB. */
	putc(2, f);

	/*  Color map specification, not used. */
	for (int i = 0; i < 5; i++)
		putc(0, f);

	/* X-origin. lo-hi 2 byte integer. */
	putc(0, f); putc(0, f);

	/* Y-origin. lo-hi 2 byte integer. */
	putc(0, f); putc(0, f);

	/* Image width. lo-hi 2 byte integer. */
	putc(data->width & 0x00FF, f);
	putc((data->width & 0xFF00) >> 8, f);

	/* Image height. lo-hi 2 byte integer. */
	putc(data->height & 0x00FF, f);
	putc((data->height & 0xFF00) >> 8, f);

	/* Image Pixel Size (amount of bits per pixel.) */
	putc(data->bitdepth, f);

	/* Image Descriptor Byte */
	putc(0x20 | (data->bitdepth == 32? 0x08 : 0x00), f);

	/* Write the image data. */
	uint64_t len = tga_len(data->width, data->height, data->bitdepth);

	for (uint64_t i = 0; i < len; ++i)
	{
		putc(data->data[i], f);
	}
}

int draw_pixel(double complex c, int x, int y, tga_data *tga)
{	
	int const iterations = 500;

	/* For debugging.
	printf("x: %d, y: %d\n", x, y);
	printf("%f + i%f\n", creal(c), cimag(c));
	*/

	double complex z = 0.0 + 0.0 * I;
	int i = 0;
	for (; i < iterations; i++)
	{	
		if (cabs(z) > 2.0)
			break;
		z = (z * z) + c;
	}

	int in_set = cabs(z) < 2.0;

	int r = pow(1.0 - (i / (double) iterations), 36) * 220.9;

	int stride = (x + (y * tga->width)) * 3;
	/* TGA is stored BGR. */
	tga->data[stride + 2] = 255 - r;
	tga->data[stride + 1] = 255 - r;
	tga->data[stride + 0] = r;

	return 0;
}

int draw_picture(unsigned int width, unsigned int height, char const *filename)
{
	if (width < 2 || height < 2)
		RERROR("Error: Dimensions may not be less than 2x2 pixels.", 
			    ERROR_DIMENSIONS_TO_SMALL);

	FILE *f = fopen(filename, "wb+");
	if (!f)
		RERROR("Error: Unable to open file for reading.", ERROR_FILE_LOCKED);

	tga_data *tga = tga_create(width, height, 24);
	if (tga == 0)
	{
		fclose(f);
		RERROR("Error: Could not create TGA data, memory error perhaps.", ERROR_TGA_CREATION);
	}

	/*
		Determine the size of the unit disk.
	*/
	double unit_length = width < height? width : height;

	double world_scale = 2.5;
	double x_offset = 0.5;

	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			double xd = x; xd = ((xd - (width * 0.5)) / unit_length) * world_scale;
			double yd = y; yd = ((yd - (height * 0.5)) / unit_length) * world_scale;

			double complex c = (xd - x_offset) + (yd * I);

			draw_pixel(c, x, y, tga);
		}
	}

	tga_write(tga, f);

	tga_free(tga);
	fclose(f);

	return 0;
}

int main(int argc, char *argv[])
{
	int w = 1920;
	int h = 1080;
	int mult = 2;
	return draw_picture(w*mult, h*mult, "mandelbrot.tga");
}