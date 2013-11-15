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
	s.real_count = 0;
	if (discriminant == 0.0f)
		s.real_count = 1;
	else if (discriminant > 0.0f)
		s.real_count = 2;

	s.s1 = NAN;
	s.s2 = NAN;	
	if (s.real_count > 0)
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

float vec3_dot(vec3 a, vec3 b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_len2(vec3 a)
{
	return vec3_dot(a, a);
}

float vec3_len(vec3 a)
{
	return sqrt(vec3_len2(a));
}

vec3 vec3_scale(vec3 a, float s)
{
	vec3 v;
	v.x = a.x * s;
	v.y = a.y * s;
	v.z = a.z * s;
	return v;
}

vec3 vec3_norm(vec3 a)
{
	float l = vec3_len(a);
	float fraction = 1.0f / l;
	return vec3_scale(a, fraction);
}

/*
	RAYS.
*/
ray mkray(vec3 origin, vec3 dir)
{
	ray r;
	r.origin = origin;
	r.dir = vec3_norm(dir);
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

int ray_intersects_sphere(ray r, sphere_primitive s, ray_sphere_test *rst)
{
	vec3 l = vec3_norm(r.dir); /* l . l = ||l||^2 = 1*1 = 1 now */
	vec3 o_min_c = vec3_sub(r.origin, s.center);
	float o_min_c_sqr = vec3_dot(o_min_c, o_min_c);

	float p = 2.0f * vec3_dot(l, o_min_c);
	float q = o_min_c_sqr - (s.radius * s.radius);

	quadratic_zeros zeros = find_quadratic_zeros(p, q);

	rst->hits = zeros.real_count;
	if (rst->hits > 0)
	{
		rst->p1 = ray_point(r, zeros.s1);
		rst->n1 = vec3_norm(vec3_sub(rst->p1, s.center));
		if (rst->hits > 1)
		{
			rst->p2 = ray_point(r, zeros.s2);
			rst->n2 = vec3_norm(vec3_sub(rst->p2, s.center));
		}
	}

	return rst->hits;
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

	sphere_primitive s;
	s.radius = 0.50f;
	s.center = mkvec3(0, 0, 0);

	ray r = mkray(mkvec3(5.0f, 0, 0), mkvec3(-1, 0, 0));

	ray_sphere_test test;
	if (ray_intersects_sphere(r, s, &test) < 1)
	{
		printf("No intersections reported.\n");
	}
	else
	{
		printf("%d intersections, at %f %f %f and %f %f %f.\n", 
			test.hits, test.p1.x, test.p1.y, test.p1.z, test.p2.x, test.p2.y, test.p2.z);
		printf("hit normals: %f %f %f and %f %f %f.\n",
			test.n1.x, test.n1.y, test.n1.z, test.n2.x, test.n2.y, test.n2.z);
	}



	return 0;
}