#include "stdio.h"
#include "stdlib.h"
#include "tga.h"

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

int main(int argc, char *argv[])
{
#ifdef _WIN32
	/*
		On Windows we need a binary mode stdin.
	*/
	_setmode(_fileno(stdin), _O_BINARY);
	_setmode(_fileno(stdout), _O_BINARY);
	/* 
		Now we can use stdin as we please. 
	*/
#endif

	tga_read_result tga_res = tga_read(stdin, TGA_BYTES_MAX, TGA_PIXELS_MAX);
	if (!TGA_READ_IS_SUCCESS(tga_res))
	{
		puts(tga_res.msg);
		return tga_res.error;
	}

	tga_data *tga = tga_res.data;

	uint64_t stride = tga_calc_stride(tga);
	for (uint16_t y = 0; y < tga->height; ++y)
	{
		for (uint16_t x = 0; x < tga->width; ++x)
		{
			uint64_t offset = (stride * y) + (x * (tga->bitdepth / 8));
			uint8_t t = tga->data[offset];
			tga->data[offset] = tga->data[offset + 2];
			tga->data[offset + 2] = t;
		}
	}

	tga_write(tga, stdout);

	tga_free(tga);

	return 0;
}