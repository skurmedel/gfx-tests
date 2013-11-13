#ifndef SPHERETRACE_H
#define SPHERETRACE_H

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

/*
	VECTORS.
*/
typedef struct vect3_t
{
	float x, y, z;
} vec3;

vec3 mkvec3(float a, float b, float c);
vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_sub(vec3 a, vec3 b);
vec3 vec3_dot(vec3 a, vec3 b);
vec3 vec3_scale(vec3 a, float s);

typedef vec3 rgb;

/*
	TGA STUFF.
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

uint64_t tga_len(uint16_t w, uint16_t h, uint8_t bitdepth);
tga_data *tga_create(uint32_t w, uint32_t h, uint8_t bitdepth);
void tga_free(tga_data *tga);
void tga_write(tga_data *data, FILE *f);
#endif