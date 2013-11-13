#ifndef SPHERETRACE_H
#define SPHERETRACE_H

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

/*
	MATHS.
*/
typedef struct quadratic_zeros_t
{
	float s1;
	float s2;
	/*
		Number of real solutions.
	*/
	int real_count;
} quadratic_zeros;
quadratic_zeros find_quadratic_zeros(float p, float q);

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
	RAYS.
*/
typedef struct ray_t
{
	vec3 origin;
	/*
		The direction of the ray.
	*/
	vec3 dir;
} ray;

ray mkray(vec3 origin, vec3 dir);
/*
	Return a point p along the ray, where p=origin if t=0.
*/
vec3 ray_point(ray r, float t);

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