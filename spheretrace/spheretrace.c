#include "spheretrace.h"

/*
	MATHS.
*/
quadratic_zeros find_quadratic_zeros(float p, float q)
{
	float phalf = p / 2.0f;
	/* Midpoint for equation. */
	float mid = -(phalf);
	float discriminant = (phalf * phalf) - q;

	quadratic_zeros s;
	if (discriminant < 0.0f)
		s.real_count = 0;
	else if (discriminant == 0.0f)
		s.real_count = 1;
	else
		s.real_count = 2;

	if (s.real_count < 1)
	{
		s.s1 = NAN;
		s.s2 = NAN;	
	}
	else
	{
		s.s1 = mid + sqrt(discriminant);
		if (s.real_count > 1)
			s.s2 = mid - sqrt(discriminant);
		else
			s.s2 = s.s1;	
	}	

	return s;
}


/*
	VECTOR METHODS.
*/
vec3 mkvec3(float a, float b, float c)
{
	vec3 v = {a, b, c};
	return v;
}

vec3 vec3_add(vec3 a, vec3 b)
{
	vec3 v;
	v.x = a.x + b.x;
	v.y = a.y + b.y;
	v.z = a.z + b.z;
	return v;
}

vec3 vec3_sub(vec3 a, vec3 b)
{
	vec3 v;
	v.x = a.x - b.x;
	v.y = a.y - b.y;
	v.z = a.z - b.z;
	return v;
}

vec3 vec3_dot(vec3 a, vec3 b)
{
	vec3 v;
	v.x = a.x * b.x;
	v.y = a.y * b.y;
	v.z = a.z * b.z;
	return v;
}

vec3 vec3_scale(vec3 a, float s)
{
	vec3 v;
	v.x = a.x * s;
	v.y = a.y * s;
	v.z = a.z * s;
	return v;
}

/*
	RAYS.
*/
ray mkray(vec3 origin, vec3 dir)
{
	ray r;
	r.origin = origin;
	r.dir = dir;
	return r;
}

/*
	Return a point p along the ray, where p=origin if t=0.
*/
vec3 ray_point(ray r, float t)
{
	vec3 d = vec3_scale(r.dir, t);
	return vec3_add(r.origin, d);
}

/*
	TGA METHODS.
*/
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

int main(int argc, char *argv[])
{
	printf("Hello my dear! You supplied %d arguments.\n", argc);

	quadratic_zeros qz = find_quadratic_zeros(4, -21);
	printf("Solution for x^2 + 4x - 21 = 0, %d solutions, %f and %f.\n", qz.real_count, qz.s1, qz.s2);

	return 0;
}