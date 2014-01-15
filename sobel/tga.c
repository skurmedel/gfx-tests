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

#include "tga.h"
#include "stdlib.h"

extern uint64_t tga_len(uint16_t w, uint16_t h, uint8_t bitdepth)
{	
	uint64_t bytespp = bitdepth == TGA_24BPP? 3 : 4;
	uint64_t len = ((uint32_t) w) * ((uint32_t) h) * bytespp;
	return len;
}

// bithdepth TGA_24BPP or TGA_32BPP
extern tga_data *tga_create(uint32_t w, uint32_t h, uint8_t bitdepth)
{
	if (w < 1 || h < 1)
		return NULL;
	if (bitdepth != TGA_24BPP && bitdepth != TGA_32BPP)
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

extern void tga_free(tga_data *tga)
{
	if (tga != NULL)
		free(tga->data);
	free(tga);
}

extern void tga_write(tga_data *data, FILE *f)
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
	putc(data->bitdepth == TGA_32BPP? 0x08 : 0x00, f);

	/* Write the image data. */
	uint64_t len = tga_len(data->width, data->height, data->bitdepth);

	for (uint64_t i = 0; i < len; ++i)
	{
		putc(data->data[i], f);
	}
}

/* 
	tga_read helper macros. 
	Assumes the following is defined in the scope it is used in:
		FILE *f;
		uint64_t maxbytes;
		uint64_t maxpixels;

	Tries to read a byte into var and increment count.

	If fgetc fails, jumps to errorloc. Jumps to errorloc if count >= maxbytes.
*/
#define TRY_READ_BYTE(var, count, errorloc) \
		{\
			if ((count) == maxbytes) { goto errorloc; } \
			int byte = fgetc(f); \
			if (byte == EOF) { goto errorloc; } \
			(var) = byte; \
			count++; \
		}

#define RETURN_ERROR(msg, code) { return (tga_read_result) { 0, (msg), (code) }; }

uint16_t from_lo_hi(uint8_t lo, uint8_t hi)
{
	uint16_t final = hi << 8;
	final |= lo;
	return final;
}

extern tga_read_result tga_read(FILE *f, uint64_t maxbytes, uint64_t maxpixels)
{
	uint64_t nbytes = 0;
	uint8_t lastbyte = 0;

	uint8_t imageident_len = 0;
	/* Identification field. */
	TRY_READ_BYTE(imageident_len, nbytes, streamerror);

	/* Color map type. */
	/* Always zero because we don't support color mapped images. */
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
	if (lastbyte != 0) 
		RETURN_ERROR("Color mapped pictures not supported.", TGA_ERROR_READ_COLOR_MAPPED);

	/* Image type code. */
	/* Always 2 since we only support uncompressed RGB. */
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
	if (lastbyte != 2)
		RETURN_ERROR("Compression not supported.", TGA_ERROR_READ_COMPRESSED);

	/*  Color map specification, not used. */
	for (int i = 0; i < 5; i++)
		TRY_READ_BYTE(lastbyte, nbytes, streamerror);

	/* X-origin. lo-hi 2 byte integer. */
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
		
	/* Y-origin. lo-hi 2 byte integer. */
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
	
	uint16_t width = 0;
	uint16_t height = 0;
	/* Image width. lo-hi 2 byte integer. */
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
	TRY_READ_BYTE(width, nbytes, streamerror);
	width = from_lo_hi(lastbyte, width);

	/* Image height. lo-hi 2 byte integer. */
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
	TRY_READ_BYTE(height, nbytes, streamerror);
	height = from_lo_hi(lastbyte, height);

	/* Sanity check image & height. */
	if (width == 0)
		RETURN_ERROR("Width reported as zero.", TGA_ERROR_READ_INVALID_DIMENSIONS);
	if (height == 0)
		RETURN_ERROR("Height reported as zero.", TGA_ERROR_READ_INVALID_DIMENSIONS);

	uint8_t bitdepth;
	/* Image Pixel Size (amount of bits per pixel.) */
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
	bitdepth = lastbyte;

	if (bitdepth != TGA_24BPP && bitdepth != TGA_32BPP)
		RETURN_ERROR(
			"TGA header indicated unsupported bitdepth, only 24bpp and 32bpp are supported.", 
			TGA_ERROR_READ_BITDEPTH);

	/* Image Descriptor Byte */
	TRY_READ_BYTE(lastbyte, nbytes, streamerror);
	if (bitdepth == TGA_32BPP && (lastbyte & 0x08) != 8)
	{
		RETURN_ERROR(
			"Header indicated 32bpp, but Image Descriptor is contraditory.", 
			TGA_ERROR_READ_MALFORMED_HEADER);
	}
	else if (bitdepth == TGA_24BPP && (lastbyte & 0x08) != 0)
		RETURN_ERROR(
			"Header indicated 24bpp, but Image Descriptor is contraditory.",
			TGA_ERROR_READ_MALFORMED_HEADER);
	// Todo: More sanity checking for rest of Image Descriptor data here.

	/* Read and throw away image identification field. */
	for (int i = 0; i < imageident_len; ++i)
		TRY_READ_BYTE(lastbyte, nbytes, streamerror);

	/* 
		Here we could read the Color map data, but we don't support it
		and above we bail out if the header indicates one exists.

		So we just don't.
	*/

	/*
		Read the image data. Pretty straightforward.
	*/
	tga_data *tga = tga_create(width, height, bitdepth);
	if (!tga)
		RETURN_ERROR("Could not allocate memory for image data.", TGA_ERROR_READ_OOM);

	uint64_t expected_bytes = tga_len(width, height, bitdepth);
	uint64_t bytes_per_pixel = bitdepth == TGA_32BPP? 4 : 3;
	for (int i = 0; i < expected_bytes; ++i)
	{
		if ((i / bytes_per_pixel) == maxpixels)
		{
			RETURN_ERROR("Image data was larger than permitted", TGA_ERROR_READ_TOO_FAR);
		}

		TRY_READ_BYTE(lastbyte, nbytes, streamerror);
		tga->data[i] = lastbyte;
	}

	tga_read_result result;
	result.data = tga;
	result.msg = "Read was succesful.";
	result.error = TGA_READ_SUCCESS;

	return result;

streamerror:
	if (nbytes >= maxbytes)
	{
		RETURN_ERROR("Image data was larger than permitted.", TGA_ERROR_READ_TOO_FAR);
	}

	RETURN_ERROR("Stream error or unexpected EOF.", TGA_ERROR_READ_EOF);
}

#undef RETURN_ERROR
#undef TRY_READ_BYTE

extern uint64_t tga_calc_stride(tga_data *tga)
{
	uint64_t bytes_per_pixel = tga->bitdepth / 8;
	return tga->width * bytes_per_pixel;
}