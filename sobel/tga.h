/*
	The MIT License (MIT)

	Copyright (c) 2014 Simon Otter

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

#ifndef SOTGA_HPP
#define SOTGA_HPP

/*
	A small TGA image reader/writer. Does the barebones. It is MIT licensed so
	you can do what you want with it.

	Made by Simon Otter 2012-2014.
*/

#include "stdint.h"
#include "stdio.h"

/*
	24 bits of colour data in a BGR configuration.
	8 bits red, 8 bits green and 8 bits blue.
*/
#define TGA_24BPP	24
/*
	32 bits of colour data in a BGRA configuration.
	8 bits per channel.
*/
#define TGA_32BPP	32

#define TGA_BYTES_MAX						UINT64_MAX
#define TGA_PIXELS_MAX						UINT32_MAX
#define TGA_3_MEGAPIXELS					3 * (1000000)
#define TGA_6_MEGAPIXELS					6 * (1000000)
#define TGA_9_MEGAPIXELS					9 * (1000000)
#define TGA_12_MEGAPIXELS					12 * (1000000)

/*
	Undefined error. Shouldn't occur.
*/
#define TGA_READ_ERROR						0
#define TGA_READ_SUCCESS					1
/*
	General header error. Too short etc.
*/
#define TGA_ERROR_READ_MALFORMED_HEADER		2
/*
	Width or height is reported as 0.
*/
#define TGA_ERROR_READ_INVALID_DIMENSIONS	3
/* 
	Compression not supported at the moment. 
*/
#define TGA_ERROR_READ_COMPRESSED			4
/* 
	Color maps not supported at the moment. 
*/
#define TGA_ERROR_READ_COLOR_MAPPED			5
/*
	Unexpected EOF encountered.
*/
#define TGA_ERROR_READ_EOF					6
/*
	Unknown or unsupported bit depth.
*/
#define TGA_ERROR_READ_BITDEPTH				7
/*
	Out of memory.
*/
#define TGA_ERROR_READ_OOM 					8
#define TGA_ERROR_READ_TOO_FAR				9

#define TGA_READ_IS_SUCCESS(res)			((res).error == TGA_READ_SUCCESS)

/*
	Represents a TGA image.
*/
typedef struct tga_data_s
{
	uint16_t width;
	uint16_t height;
	/* 
		The image data is arranged from top left to bottom right,
		BGRA (32 bit) or BGR (24 bit).

		Thus data[0] is the blue byte of the topmost left corner.
	*/
	uint8_t  *data;
	/* 24 or 32 (32 is with alpha mask.) */
	uint8_t  bitdepth;
} tga_data;

typedef struct tga_read_result_s
{
	/*
		Holds the data, if any.

		If error != TGA_READ_SUCCESS, this will be a NULL ptr.
	*/
	tga_data *data;
	/*
		A descriptive message for the first error encountered.

		Never NULL.
	*/
	char const *msg;
	/*
		An error code. TGA_READ_SUCCESS if read was succesful.
	*/
	uint16_t error;
} tga_read_result;

/*
	Calculates the expected byte length of a piece of image data for a 
	TGA image. This is the size of the memory block in tga_data->data.

	It does not include headers and such.

	w and h must be > 0. bitdepth must be either 
	TGA_24BPP or TGA_32BPP.

	If either w or h is zero, and bitdepth is not TGA_24BPP or 
	TGA_32BPP the value returned will be incorrect.
*/
extern uint64_t tga_len(uint16_t w, uint16_t h, uint8_t bitdepth);

/*
	Creates a new TGA image in memory.

	Returns 0 if w or h is zero, or bitdepth is not TGA_24BPP nor TGA_32BPP.
	Furthermore, on out of memory conditions, also returns a zero
	pointer.
*/
extern tga_data *tga_create(uint32_t w, uint32_t h, uint8_t bitdepth);

/*
	Frees a tga_data struct. Must be called for proper clean-up.

	If tga equals 0, does nothing.
*/
extern void tga_free(tga_data *tga);

/*
	Writes out a properly formatted TGA datastream to the file handle.

	If data equals 0, does nothing.
*/
extern void tga_write(tga_data *data, FILE *f);

/*
	Tries to read an image from file handle f.

	f is assumed to be pointing at the start of the TGA data, which 
	means the first byte of the header. It will fail if the header
	can't be parsed.

	If more than maxbytes are read, the operation is cancelled and
	a failure (TGA_ERROR_READ_TOO_FAR) is returned. 

	If the image data has dimensions resulting in a pixel count
	larger than maxpixels, the operation is cancelled. 

	Example:

		// Read an image of less than exactly 6 megapixels.
		tga_read(f, TGA_BYTES_MAX, TGA_6_MEGAPIXELS);

	Both maxbytes and maxpixels can be bypassed in practice by
	passing TGA_BYTES_MAX and TGA_PIXELS_MAX.

	Example:

		// Read an image up to the maximum allowed by TGA.
		// (2^16 - 1) * (2^16 - 1) pixels.
		tga_read_result result = tga_read(f, TGA_BYTES_MAX, TGA_MAX_PIXELS);
		if (!TGA_READ_IS_SUCCESS(result))
		{
			puts("Oh noes.")
			abort();
		}

	Returns a tga_read_result, it describes the result from the 
	operation.
*/
extern tga_read_result tga_read(FILE *f, uint64_t maxbytes, uint64_t maxpixels);

/*
	Calculates the byte stride for a given TGA.

	Stride is the amount of bytes a scanline (row of pixels) 
	occupies in memory. It is the amount of bytes you need 
	to move in memory to get the next scanline.

	So after one stride, the row of pixels for y = 1 begins.

	Stride in some formats is larger than the width of the 
	image, but TGA does not put data after each scanline,
	so we can safely use the stride directly to get a 
	pixels position in memory.
*/
extern uint64_t tga_calc_stride(tga_data *tga);
#endif