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

	This reader only concerns itself with Version 1 TGAs. Version 2 TGAs are
	probably readable but it will not use the data stored in the footer and
	extension areas. 

	Currently the library only supports 24-bit and 32-bit uncompressed 
	non-colormapped TGAs.

	Made by Simon Otter 2012-2014.
*/

#include "stdint.h"
#include "stdio.h"

#define TGA_BYTES_MAX						UINT64_MAX
#define TGA_PIXELS_MAX						UINT32_MAX

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
	HEADER UTILITIES AND STRUCTS.
*/

#define TGA_UNMAPPPED_COLOR										0
#define TGA_MAPPED_COLOR 										1

#define TGA_IMAGE_TYPE_NONE										0
#define TGA_IMAGE_TYPE_UNCOMPRESSED_COLOR_MAPPED				1
#define TGA_IMAGE_TYPE_UNCOMPRESSED_RGB							2
#define TGA_IMAGE_TYPE_UNCOMPRESSED_BW							3
#define TGA_IMAGE_TYPE_RLE_COLOR_MAPPED							9
#define TGA_IMAGE_TYPE_RLE_RGB         							10
#define TGA_IMAGE_TYPE_COMPRESSED_BW   							11
#define TGA_IMAGE_TYPE_COMPRESSED_COLOR_MAPPED_HUFFMAN_RLE		33
#define TGA_IMAGE_TYPE_COMPRESSED_COLOR_MAPPED_HUFFMAN_QTREE 	34

#define TGA_IMAGE_DESCRIPTOR_ATTR_BIT_MASK						0x0F
#define TGA_IMAGE_DESCRIPTOR_SCREEN_ORIGIN_BIT_MASK				0x20
#define TGA_IMAGE_DESCRIPTOR_INTERLEAVING_FLAG					0xC0

#define TGA_16BPP	16
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

/*
	This structure mimics the data in the TGA version 1 headers.

	Use the associated functions to read a header from a file stream
	if needed to.

	In actual TGA files, data is explicitly little-endian, so two-byte
	fields will be stored (lo, hi). In this structure they are stored
	natively, the methods in the lib will convert to the right endianess.
*/
typedef struct tga_version1_header_s
{
	/*
		Number of (1 byte) characters stored in the identification
		field.
	*/
	uint8_t identification_field_length;

	/*
		Color Map Type.
	*/
	uint8_t color_map_type;

	/*
		Image Type Code.
	*/
	uint8_t image_type_code;

	/*
		Color Map Spec.

		If color_mapped == TGA_UNMAPPPED_COLOR then the values in these
		fields don't matter.
	*/
	uint16_t color_map_origin;
	uint16_t color_map_length;
	uint8_t  color_map_entry_size;

	/* 
		Image specification. 
	*/

	/*
		X and Y origin of the lower left corner of the image.
	*/
	uint16_t x_origin;
	uint16_t y_origin;
	uint16_t width;
	uint16_t height;
	/*
		Number of bits per pixel.
	*/
	uint8_t  image_pixel_size;
	/*
		First four (0 to 3) bits, number of "attribute bits" per pixel.
		That means, how many bits for the alpha channel.

			Targa 16, either 0 or 1.
			Targa 24, 0, no alpha map.
			Targa 32, 8, 8 bit alpha map.

		Bit 4 is always zero.
		Bit 5 is a screen origin bit.

			0 lower left hand corner is origin.
			1 upper left hand corner is origin.

			Is generally 0. Acts as a vertical flip.

		Bit 7-6 data storage interleaving flag.

	*/
	uint8_t  image_descriptor_byte;

	/*
		Arbitrary character data, the image identification field
		gives the actual length.
	*/
	uint8_t image_identification_field[255];
} tga_version1_header;

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